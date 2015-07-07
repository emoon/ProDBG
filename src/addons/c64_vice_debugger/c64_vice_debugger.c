#include "pd_backend.h"
#include "pd_menu.h"
#include "pd_host.h"
#include "c64_vice_connection.h"
#include "c64_vice_custom_regs.h"
#include <stdlib.h>
#include <uv.h>

#ifndef _WIN32
#include <unistd.h>
#include <limits.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <jansson.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char s_recvBuffer[512 * 1024];
static char s_tempBuffer[512 * 1024];
static const int maxBreakpointCount = 8192;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    C64_VICE_MENU_ATTACH_TO_VICE,
    C64_VICE_MENU_START_WITH_CONFIG,
    C64_VICE_MENU_DETACH_FROM_VICE,
};

static PDMessageFuncs* messageFuncs;

#ifdef _WIN32
__declspec(dllimport) void OutputDebugStringA(const char*);
#endif

static bool doDebug = false;

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_debug_printf(const char* format, ...)
{
    va_list ap;
    char buffer[512 * 1024];
    
    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

#ifdef _WIN32
    OutputDebugStringA(buffer);
#else
    printf("%s", buffer);
#endif
}

#define log_debug(fmt, ...) \
do { \
	if (doDebug) \
		log_debug_printf("%s:(%d) " fmt , __FILENAME__ , __LINE__, __VA_ARGS__); \
} while (0)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Regs6510
{
    uint16_t pc;
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint16_t flags;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum BreakpoinType
{
    BreakpointType_Normal,
    BreakpointType_Data,
} BreakpoinType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    //char* condition;
    uint16_t address;
    BreakpoinType type;
    int32_t id;
    int32_t internalId;

} Breakpoint;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Breakpoints
{
    Breakpoint** data;
    int count;
} Breakpoints;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Config
{
    const char* viceExe;
    const char* prgFile;
    const char* kickAssSymbols;
    const char* breakpointFile;
} Config;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PluginData
{
    struct VICEConnection* conn;
    struct Regs6510 regs;
    bool hasUpdatedRegistes;
    bool hasUpdatedExceptionLocation;
    PDDebugState state;
    char tempFileFull[8192];
    Breakpoints breakpoints;
    Config config;
    uv_process_t process;

} PluginData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef bool (*ParseDataFunc)(PluginData* data, const char* res, int len, PDReader* reader, PDWriter* writer);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Add to services

static void sleepMs(int ms)
{
#ifdef _MSC_VER
    Sleep(ms);
#else
    usleep((unsigned int)(ms * 1000));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool getFullName(char* fullName, const char* name)
{
#ifdef _MSC_VER
    if (GetFullPathNameA(name, 8192, fullName, 0) == 0)
    {
        strcpy(fullName, name);
        return false;
    }
#else
    void* t = realpath(name, fullName);
    (void)t;

    return true;
#endif

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* loadToMemory(const char* filename, size_t* size)
{
    FILE* f = fopen(filename, "rb");
    void* data = 0;
    size_t s = 0;

    *size = 0;

    if (!f)
        return 0;

    // TODO: Use fstat here?

    fseek(f, 0, SEEK_END);
    long ts = ftell(f);

    if (ts < 0)
        goto end;

    s = (size_t)ts;

    data = malloc(s);

    if (!data)
        goto end;

    fseek(f, 0, SEEK_SET);

    size_t t = fread(data, s, 1, f);
    (void)t;

    *size = s;

    end:

    fclose(f);

    return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int parsePrg(const char* filename)
{
	size_t size = 0;

	const char* data = (const char*)loadToMemory(filename, &size);

	if (!data)
		return -1;

	if (size < 10)
	{
		free((void*)data);
		log_debug("Prg file %s it too small (less than 7 bytes)\n", filename);
		return -1;
	}

	// Seek to pos 7 in the file where the sys offset is located. The file looks like this
	//
	// load offset - 2 bytes
	// unknown     - 5
	// text string (null terminated) decimal start adress

	int runAddress = atoi(&data[7]);

	free((void*)data);

	return runAddress; 
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setupDefaultConfig(PluginData* data)
{
#ifdef PRODBG_MAC
    data->config.viceExe = strdup("/Applications/VICE/x64.app/Contents/MacOS/x64");
#elif PRODBG_WIN
    data->config.viceExe = strdup("x64.exe");
#else
    data->config.viceExe = strdup("x64");
#endif

    data->config.prgFile = strdup("examples/c64_vice/test.prg");
    data->config.kickAssSymbols = 0;
    data->config.breakpointFile = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void loadConfig(PluginData* data, const char* filename)
{
    const char* viceExe = 0;
    const char* prgFile = 0;
    const char* kickAssSymbols = 0;
    const char* breakpointFile = 0;
    json_error_t error;

    setupDefaultConfig(data);

    json_t* root = json_load_file(filename, 0, &error);

    if (!root || !json_is_object(root))
        return;

    log_debug("loaded config\n", "");

    json_unpack(root, "{s:s, s:s, s:s, s:s}",
                "vice_exe", &viceExe,
                "prg_file", &prgFile,
                "kickass_symbols", &kickAssSymbols,
                "breakpoints_file", &breakpointFile);

    if (viceExe && viceExe[0] != 0)
        data->config.viceExe = strdup(viceExe);

    if (prgFile && prgFile[0] != 0)
        data->config.prgFile = strdup(prgFile);

    if (breakpointFile && breakpointFile[0] != 0)
        data->config.breakpointFile = strdup(breakpointFile);

    if (kickAssSymbols && kickAssSymbols[0] != 0)
        data->config.kickAssSymbols = strdup(kickAssSymbols);

    json_decref(root);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sendCommand(PluginData* data, const char* format, ...)
{
    va_list ap;
    char buffer[2048];

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

    int len = (int)strlen(buffer);

    if (!data->conn)
        return;

    int ret = VICEConnection_send(data->conn, buffer, len, 0);
    (void)ret;

    log_debug("sent command %s (%d - %d)\n", buffer, len, ret);

    sleepMs(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Tries to get data from VICE. Has a maxTry count that can be used to actually add some retries in this code
// as data from VICE can take a while. For each loop we sleep for 1 ms in order to not hammer on the socket and
// allows VICE some time.

static bool getDataToBuffer(PluginData* data, char* resBuffer, int bufferSize, int* len, int maxTry)
{
    int res = 0;
    int lenCount = 0;

    if (!data->conn)
        return false;

    memset(resBuffer, 0, bufferSize);

    for (int i = 0; i < maxTry; ++i)
    {
        bool gotData = false;

        //log_debug("trying to get data %d\n", i);

        while (VICEConnection_pollRead(data->conn))
        {
            res = VICEConnection_recv(data->conn, resBuffer, bufferSize - lenCount, 0);

            if (res == 0)
                break;

            gotData = true;

            resBuffer += res;
            lenCount += res;
        }

        if (gotData)
        {
        	log_debug("got some data, len %d", lenCount);

            *len = lenCount;
            return true;
        }

        sleepMs(1);
    }

	log_debug("no data from VICE\nn", "");

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getData(PluginData* data, char** resBuffer, int* len)
{
	if (!getDataToBuffer(data, s_recvBuffer, sizeof(s_recvBuffer), len, 1))
		return false;

	*resBuffer = (char*)&s_recvBuffer;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int waitForData(PluginData* data, char** resBuffer, int* len)
{
	const int maxTry = 1000;

	for (int i = 0; i < maxTry; ++i)
	{
		getData(data, resBuffer, len);

		if (*len != 0)
			return 1;

		sleepMs(1);
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Breakpoint* createBreakpoint()
{
    Breakpoint* bp = (Breakpoint*)malloc(sizeof(Breakpoint));
    memset(bp, 0, sizeof(Breakpoint));
    return bp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addBreakpoint(PluginData* data, Breakpoint* bp)
{
    assert(data->breakpoints.count < maxBreakpointCount);
    data->breakpoints.data[data->breakpoints.count++] = bp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool findBreakpointById(PluginData* data, Breakpoint** breakpoint, int id)
{
    for (int i = 0, end = data->breakpoints.count; i < end; ++i)
    {
        Breakpoint* bp = data->breakpoints.data[i];

        if (bp->id == id)
        {
            *breakpoint = data->breakpoints.data[i];
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   static Breakpoint* getBreakpoint(PluginData* data, uint64_t* address, PDReader* reader, PDWriter* writer)
   {
    int32_t id;
    Breakpoint* bp;

    if (PDRead_findU32(reader, &id, "id", 0) == PDReadStatus_notFound)
    {
        PDWrite_eventBegin(writer, PDEventType_replyBreakpoint);
        PDWrite_string(writer, "error", "No ID being sent for breakpoint");
        PDWrite_eventEnd(writer);
        return 0;
    }

    if (PDRead_findU64(reader, address, "address", 0) == PDReadStatus_notFound)
    {
        PDWrite_eventBegin(writer, PDEventType_replyBreakpoint);
        PDWrite_string(writer, "error", "prodNo address is being sent for breakpoint");
        PDWrite_eventEnd(writer);
        return 0;
    }

    if (!findBreakpointById(data, &bp, id))
    {
        bp = createBreakpoint();
        bp->id = id;
        bp->internalId = -1;
    }

    return bp;
   }
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool sendCommandGetData(PluginData* data, const char* sendCmd, ParseDataFunc parseFunc, PDReader* reader, PDWriter* writer, int maxTry)
{
    char* res = s_recvBuffer;
    int len = 0;

    sendCommand(data, sendCmd); 

	for (int i = 0; i < maxTry; ++i)
	{
		int tempLen = 0; 

		// We give VICE 1 sek to does this thing. TODO: Lower this value or have it as a config?

		if (!getDataToBuffer(data, res, (int)(sizeof(s_recvBuffer)) - len, &tempLen, 1000))
		{
			log_debug("couldn't get any data\n", "");
			return false;
		}

		log_debug("got data %s\n", res);

		len += tempLen;

		if (parseFunc(data, res, len, reader, writer))
			return true;

		res += tempLen;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool checkForDefaultState(PluginData* data, const char* res, int len, PDReader* reader, PDWriter* writer)
{
	(void)data; (void)len; (void)reader; (void)writer;
	return strstr(res, "(C:$");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool delBreakpointById(PluginData* data, int32_t id)
{
    const int breakpointCount = data->breakpoints.count;

    for (int i = 0, end = data->breakpoints.count; i < end; ++i)
    {
        Breakpoint* bp = data->breakpoints.data[i];

        if (bp->id == id)
        {	
			char temp[1024];
			sprintf(temp, "del %d\n", id);

            sendCommandGetData(data, temp, checkForDefaultState, 0, 0, 20);

            // Swap with the last bp and decrese the count

            Breakpoint* lastBp = data->breakpoints.data[breakpointCount - 1];
            data->breakpoints.data[i] = lastBp;
            data->breakpoints.count--;

            // swap with the last and dec count

            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool delBreakpoint(PluginData* data, PDReader* reader, PDWriter* writer)
{
    int32_t id;

    if (PDRead_findS32(reader, &id, "id", 0) == PDReadStatus_notFound)
    {
        PDWrite_eventBegin(writer, PDEventType_replyBreakpoint);
        PDWrite_string(writer, "error", "No ID being sent for breakpoint");
        PDWrite_eventEnd(writer);
        return false;
    }

    log_debug("deleting breakpoint with id %d\n", id);

    return delBreakpointById(data, id);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void connectToLocalHost(PluginData* data)
{
    struct VICEConnection* conn = 0;

    // Kill the current connection if we have one

    if (data->conn)
    {
        VICEConnection_destroy(data->conn);
        data->conn = 0;
    }

    conn = VICEConnection_create(VICEConnectionType_Connect, 6510);

    if (!VICEConnection_connect(conn, "localhost", 6510))
    {
        VICEConnection_destroy(conn);

        data->conn = 0;
        data->state = PDDebugState_noTarget;

        return;
    }

    data->conn = conn;
    data->state = PDDebugState_running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc)
{
    (void)serviceFunc;

    PluginData* data = malloc(sizeof(PluginData));
    memset(data, 0, sizeof(PluginData));

    getFullName((char*)&data->tempFileFull, "temp/vice_mem_dump");

    data->state = PDDebugState_noTarget;

    loadConfig(data, "data/c64_vice.cfg");

    messageFuncs = serviceFunc(PDMESSAGEFUNCS_GLOBAL);

    //TODO: non fixed size?

    data->breakpoints.data = (Breakpoint**)malloc(sizeof(Breakpoint*) * maxBreakpointCount);
    data->breakpoints.count = 0;

    return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void parseMonFile(PluginData* data, const char* filename)
{
    char textLine[1024];

    FILE* f = fopen(filename, "rt");

    if (!f)
        return;

    for (;;)
    {
        if (!fgets(textLine, sizeof(textLine), f))
            break;

        sendCommand(data, "%s\n", textLine);
    }

    fclose(f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t* getMemoryInternal(PluginData* data, const char* tempfile, size_t* readSize, uint16_t address, uint16_t addressEnd)
{
    *readSize = 0;

#ifndef _WIN32
    if (unlink(tempfile) < 0)
    {
        log_debug("c64_vice: Unable to delete %s (error %d)\n", tempfile, errno);
    }
#else
    // TODO: Implement me.
    /*
       if (DeleteFile(tempfile) != 0)
       {
        printf("failed to delete %s\n", timepfile);
        return 0;
       }
     */
#endif

    sendCommand(data, "save \"%s\" 0 %04x %04x\n", tempfile, address, addressEnd);

    // TODO: Improve this? (wait for (C: as return data back from send command)

    // Wait 10 ms for operation to complete and if we can't open the file we try for a few times and if we still can't we bail

    sleepMs(10);

    for (int i = 0; i < 10; ++i)
    {
        uint8_t* mem = loadToMemory(data->tempFileFull, readSize);

        if (!mem)
        {
            sleepMs(1);
            continue;
        }

        log_debug("returing mem...\n", "");

        return mem;
    }

    log_debug("Unable to get memory...\n", "");

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool loadImage(PluginData* data, const char* filename)
{
    char* res = 0;
    int len = 0;

    sendCommand(data, "load \"%s\" 0\n", filename);

    sleepMs(200);

    if (!getData(data, &res, &len))
    {
        log_debug("failed to get any data back from load...\n", "");
        return false;
    }

    log_debug("res %s\n", res);

    // TODO: Parse that we actually manage to load the image

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t getBasicStart(PluginData* data)
{
    size_t readSize = 0;

    uint8_t* memory = getMemoryInternal(data, data->tempFileFull, &readSize, 0x800, 0x810);

    if (!memory)
    {
        log_debug("falied to get memory :(\n", "");
        return 0;
    }

    log_debug("size read %d\n", (int)readSize);

    const char* address = (char*)&memory[2 + 6];

    log_debug("memory\n", "");

    for (int i = 0; i < 18; ++i)
    {
        char c = (char)memory[i];
        char pc = (c >= 32 && c < 127) ? c : '.';
        log_debug("%c ", pc);
    }

    log_debug("\n", "");

    for (int i = 0; i < 18; ++i)
    {
        char c = (char)memory[i];
        log_debug("%02x ", c);
    }

    log_debug("\n", "");

    uint16_t startAddress = (uint16_t)strtol(address, 0, 10);

    log_debug("basic: address to start from (text) %s\n", address);
    log_debug("start address %d %x\n", startAddress, startAddress);

    free(memory);

    return startAddress;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void launchVICEWithConfig(PluginData* data)
{
    int r, cmdIndex = 1;
    uv_process_options_t options = { 0 };

    log_debug("spawning vice...\n", "");

    char* args[10];
    args[0] = (char*)data->config.viceExe;

    // TODO: Must generate the breakpoint file from the json one

    args[cmdIndex++] = "-remotemonitor";

    if (data->config.breakpointFile)
    {
        //args[cmdIndex++] = "-moncommands";
        //args[cmdIndex++] = "examples/c64_vice/test_mon.txt";
    }

    //args[cmdIndex++] = (char*)data->config.prgFile;
    args[cmdIndex++] = NULL;

    options.exit_cb = 0;
    options.file = data->config.viceExe;
    options.args = args;

    if ((r = uv_spawn(uv_default_loop(), &data->process, &options)))
    {
        messageFuncs->error("Unable to launch VICE", uv_strerror(r));
        return;
    }

    sleepMs(3000);

    connectToLocalHost(data);

    // if connected we load the image and make sure we get a reply back

    if (VICEConnection_isConnected(data->conn))
    {
        log_debug("connected to vice...\n", "");

        if (!loadImage(data, data->config.prgFile))
            return;

        log_debug("image loaded ...\n", "");

        // parse the

        parseMonFile(data, data->config.breakpointFile);

        log_debug("start from basic...\n", "");

        // start vice!

        log_debug("started from basic\n", "");

        uint16_t address = 0; //getBasicStart(data);

        parseMonFile(data, data->config.breakpointFile);

        if (address != 0)
        {
            log_debug("start from %x\n", address);
            sendCommand(data, "g %x\n", address);
        }

        return;
    }

    log_debug("unable to make connection with vice\n", "");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroyInstance(void* userData)
{
    PluginData* plugin = (PluginData*)userData;

    if (plugin->process.pid > 0)
        uv_kill(plugin->process.pid, 2);

    if (plugin->conn)
        VICEConnection_destroy(plugin->conn);

    free(plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onMenu(PluginData* data, PDReader* reader)
{
    uint32_t menuId;

    PDRead_findU32(reader, &menuId, "menu_id", 0);

    switch (menuId)
    {
        case C64_VICE_MENU_ATTACH_TO_VICE:
        {
            connectToLocalHost(data);
            break;
        }

        case C64_VICE_MENU_START_WITH_CONFIG:
        {
            launchVICEWithConfig(data);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeRegister(PDWriter* writer, const char* name, uint8_t size, uint16_t reg, uint8_t readOnly)
{
    PDWrite_arrayEntryBegin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "size", size);

    if (readOnly)
        PDWrite_u8(writer, "read_only", 1);

    if (size == 2)
        PDWrite_u16(writer, "register", reg);
    else
        PDWrite_u8(writer, "register", (uint8_t)reg);

    PDWrite_arrayEntryEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeStatusRegister(PDWriter* writer, const char* name, uint16_t reg)
{
    PDWrite_arrayEntryBegin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "read_only", 1);

    char statusString[128] = { 0 };

    sprintf(statusString, "0x$%2x N:%d V:%d -:%d B:%d D:%d I:%d Z:%d C:%d", reg,
            (reg >> 7) & 1, (reg >> 6) & 1, (reg >> 5) & 1, (reg >> 4) & 1,
            (reg >> 3) & 1, (reg >> 2) & 1, (reg >> 1) & 1, (reg >> 0) & 1);

    PDWrite_string(writer, "register_string", statusString);

    PDWrite_arrayEntryEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getMemory(PluginData* data, PDReader* reader, PDWriter* writer)
{
    uint64_t address;
    uint64_t size;
    size_t readSize = 0;

    PDRead_findU64(reader, &address, "address_start", 0);
    PDRead_findU64(reader, &size, "size", 0);

    // so this is a bit of a hack. If we request memory d000 we switch to io and then back
    // this isn't really correct but will do for now

    if (address == 0xdd00)
        sendCommand(data, "bank io\n");

    uint8_t* memory = getMemoryInternal(data, data->tempFileFull, &readSize, (uint16_t)(address), (uint16_t)(address + size));

    if (address == 0xdd00)
        sendCommand(data, "bank ram\n");

    if (memory)
    {
        // Lets do this!
        // + 2 is because VICE writes address at the start of the block and at the end
        //

        log_debug("c64_vice: sending memory\n", "");

        PDWrite_eventBegin(writer, PDEventType_setMemory);
        PDWrite_u64(writer, "address", address);
        PDWrite_data(writer, "data", memory + 2, (uint32_t)(readSize - 3));
        PDWrite_eventEnd(writer);

        // writer takes a copy

        free(memory);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool shouldSendCommand(PluginData* data)
{
    bool t0 = data->state != PDDebugState_running;
    bool t1 = VICEConnection_isConnected(data->conn);

    return t0 && t1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool parseSetExecutable(PluginData* plugin, const char* res, int len, PDReader* reader, PDWriter* writer)
{
	(void)plugin;
	(void)len;
	(void)reader;
	(void)writer;
	(void)res;

	return strstr(s_recvBuffer, "Loading") && strstr(s_recvBuffer, "(C:$");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sent to VICE: load "<filename>" 0 (device)
//
// Expected VICE reply:
// 
// Loading <filename> from xxxx to xxxx (xb bytes)
// (C:$xxxx) 
//
// Returns false if unable to do any of the required steps 
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool setExecutable(PluginData* data, PDReader* reader)
{
    const char* filename = 0;

    PDRead_findString(reader, &filename, "filename", 0);

    if (!filename)
    {
        log_debug("Unable to find filename %s\n", filename);
        return false;
    }

    log_debug("setExecutable %s\n", filename);

    int startAddress = parsePrg(filename);

    if (startAddress == -1)
    	return false;

    log_debug("loading %s and running from $%x\n", filename, startAddress);

	char temp[2048];
	sprintf(temp, "load \"%s\" 0\n", filename);

	if (!sendCommandGetData(data, temp, parseSetExecutable, reader, 0, 20))
		return false;

	sendCommand(data, "g $%x\n", startAddress);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool parseRegistersCall(PluginData* plugin, const char* data, int length, PDReader* reader, PDWriter* writer)
{
    const char* pch;
    struct Regs6510* regs = &plugin->regs;

    (void)reader;
    (void)writer;

    memcpy(s_tempBuffer, data, length);
    s_tempBuffer[length] = 0;

    log_debug("parsing registers %s\n", s_tempBuffer);

    // Format from VICE looks like this:
    // (C:$e5cf)   ADDR AC XR YR SP 00 01 NV-BDIZC LIN CYC  STOPWATCH
    //           .;e5cf 00 00 0a f3 2f 37 00100010 000 001    3400489

    char* str = strstr(s_tempBuffer, ".;");

    if (!str)
	{
		log_debug("Failed to find .;\n", "");
        return false;
	}

    pch = strtok(str, " \t\n");

    regs->pc = (uint16_t)strtol(&pch[2], 0, 16); pch = strtok(0, " \t");
    regs->a = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
    regs->x = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
    regs->y = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
    regs->sp = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");

    pch = strtok(0, " \t"); // skip 00
    pch = strtok(0, " \t"); // skip 01

    regs->flags = (uint8_t)strtol(pch, 0, 2);

    plugin->hasUpdatedRegistes = true;
    plugin->hasUpdatedExceptionLocation = true;

    log_debug("parse registers down\n", "");

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sent to VICE: registers
//
// Format from VICE looks like this:
// (C:$e5cf)   ADDR AC XR YR SP 00 01 NV-BDIZC LIN CYC  STOPWATCH
//           .;e5cf 00 00 0a f3 2f 37 00100010 000 001    3400489

static bool getRegisters(PluginData* data)
{
	return sendCommandGetData(data, "registers\n", parseRegistersCall, 0, 0, 20); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* parseDisassemblyLine(char* line)
{
    char* start = line;

    // Handle the case if we get a line that looks like this (we want to skip everything after -)
    // .C:0811  EE 20 D0    INC $D020      - A:00 X:17 Y:17 SP:f6 ..-.....   19262882

    for (;;)
    {
        char c = *line++;

        if (c == '\n' || c == 0)
            break;

        if (c == '-')
        {
            line[-1] = 0;
            break;
        }

    }

    return start;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool parseDisassemblyCall(PluginData* plugin, const char* res, int len, PDReader* reader, PDWriter* writer)
{
    memcpy(s_tempBuffer, res, len);
    s_tempBuffer[len] = 0;

    (void)plugin;
    (void)reader;

    // parse the buffer

    char* pch = strtok(s_tempBuffer, "\n");

    PDWrite_eventBegin(writer, PDEventType_setDisassembly);
    PDWrite_arrayBegin(writer, "disassembly");

    bool hasAllDisasembly = false;

    while (pch)
    {
        // expected format of each line:
        // xxx.. .C:080e  A9 22       LDA #$22

        char* endOfStream = strstr(pch, "(C:");
        char* line = strstr(pch, ".C");

        if (endOfStream)
		{
			hasAllDisasembly = true;
			break;
		}

        if (!line)
            break;

        uint16_t address = (uint16_t)strtol(&line[3], 0, 16);

        PDWrite_arrayEntryBegin(writer);
        PDWrite_u16(writer, "address", address);
        PDWrite_string(writer, "line", parseDisassemblyLine(&line[9]));

        PDWrite_arrayEntryEnd(writer);

        pch = strtok(0, "\n");
    }

    PDWrite_arrayEnd(writer);
    PDWrite_eventEnd(writer);

    return hasAllDisasembly;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sent to VICE: disass $start $end
//
// Format from VICE looks like this:
// .C:081F  xx          xxxx
// .C:0820  00          BRK
// .C:0821  00          BRK
// (C:$0822)

static bool getDisassembly(PluginData* data, PDReader* reader, PDWriter* writer)
{
	char temp[2048];

    uint64_t addressStart = 0;
    uint32_t instructionCount = 0;

    PDRead_findU64(reader, &addressStart, "address_start", 0);
    PDRead_findU32(reader, &instructionCount, "instruction_count", 0);

    // assume that one instruction is 3 bytes which is high but that gives us more data back than we need which is better than too little

	sprintf(temp, "disass $%04x $%04x\n", (uint16_t)addressStart, (uint16_t)(addressStart + instructionCount * 3));
	
	return sendCommandGetData(data, temp, parseDisassemblyCall, reader, writer, 20); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool parseBreakpointCall(PluginData* data, const char* res, int len, PDReader* reader, PDWriter* writer)
{
    Breakpoint* bp = 0;

    (void)len;
    (void)reader;

    const char* breakStrOffset = strstr(res, "BREAK:");

    if (!breakStrOffset)
        return false;

    int id = atoi(breakStrOffset + 7);

    const char* address = strstr(breakStrOffset, "C:$");

    if (!findBreakpointById(data, &bp, id))
    {
        bp = createBreakpoint();
        addBreakpoint(data, bp);
    }

    bp->id = id;

    if (address)
        bp->address = (uint16_t)strtol(address + 3, 0, 16);

    // add data or update existing

    PDWrite_eventBegin(writer, PDEventType_replyBreakpoint);
    PDWrite_u64(writer, "address", bp->address);
    PDWrite_u32(writer, "id", (uint32_t)id);
    PDWrite_eventEnd(writer);

    log_debug("sending reply back: breakpoint %x - %d\n", bp->address, id);

    // make sure we got all dat;

    return strstr(breakStrOffset, "(C:$");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sent to VICE: break $xxxx <condition>
//
// Sent from VICE:
// BREAK: 1  C:$8000  (Stop on exec)
// (C:$xxxx)

static bool setBreakpoint(PluginData* data, PDReader* reader, PDWriter* writer)
{
    uint64_t address = 0;
    int32_t id = -1;
    const char* condition = 0;

    PDRead_findS32(reader, &id, "id", 0);
    PDRead_findU64(reader, &address, "address", 0);
    PDRead_findString(reader, &condition, "condition", 0);

    log_debug("got breakpoint %d %llu %s\n", id, address, condition);

    if (id != -1)
    	delBreakpointById(data, id);

    char temp[1024];

    if (condition)
        sprintf(temp, "break $%04x if %s\n", (uint16_t)address, condition);
    else
        sprintf(temp, "break $%04x\n", (uint16_t)address);

	return sendCommandGetData(data, temp, parseBreakpointCall, reader, writer, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool findParentheses(const char* text, const char** start, const char** end)
{
	int i;

	// expected format of each line:
	// (xx) xxxx
	// Notice that first line can have this format:
	// (C:$e5cf) (xx) xxxx 
	
	int len = (int)strlen(text);

	for (i = len; i > -1; --i)
	{
		if (text[i] == ')')
			*end = &text[i];
			
		if (text[i] == '(')
		{
			*start = &text[i];
			break;
		}
	}

	// Handles the case if the line would look like (C:$xxxx)

	if (text[i + 1] == 'C')
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool parseForCallstack(PluginData* data, const char* res, int length, PDReader* reader, PDWriter* writer)
{
    uint16_t callStackEntries[256];
    int callStackCount = 0;

	(void)data;
    (void)reader;

    memcpy(s_tempBuffer, res, length);
    s_tempBuffer[length] = 0;

    char* pch = strtok(s_tempBuffer, "\n");

    while (pch)
    {
    	const char* startText;
    	const char* endText;

    	if (!findParentheses(pch, &startText, &endText))
    		break;

    	// startText points at (xx)
    	// endText points at ) xxxx

		uint16_t offset = (uint16_t)atoi(startText + 1);
		uint16_t address = (uint16_t)strtol(endText + 2, 0, 16);

		callStackEntries[callStackCount++] = address + offset;

        pch = strtok(0, "\n");
    }

    if (callStackCount == 0)
        return false;

    PDWrite_eventBegin(writer, PDEventType_setCallstack);
    PDWrite_arrayBegin(writer, "callstack");

    for (int i = 0; i < callStackCount; ++i)
    {
        PDWrite_arrayEntryBegin(writer);
        PDWrite_u16(writer, "address", callStackEntries[i]);
        PDWrite_arrayEntryEnd(writer);
    }

    PDWrite_arrayEnd(writer);
    PDWrite_eventEnd(writer);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sent to VICE: bt 
//
// Sent from VICE:
// (C:$e5cf) (2) e112  <- first line can include (C:)
// (2) xxxx
// (4) xxxx
// (6) xxxx
// (8) xxxx
// (10) xxxx
// (C:$xxxx)

static bool setCallstack(PluginData* data, PDReader* reader, PDWriter* writer)
{
	log_debug("calling setCallstack\n", "");

	return sendCommandGetData(data, "bt\n", parseForCallstack, reader, writer, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void processEvents(PluginData* data, PDReader* reader, PDWriter* writer)
{
    uint32_t event;

    while ((event = PDRead_getEvent(reader)))
    {
        switch (event)
        {
            //case PDEventType_getExceptionLocation : setExceptionLocation(plugin, writer); break;
            //case PDEventType_getCallstack : setCallstack(plugin, writer); break;

            case PDEventType_getRegisters:
            {
            	getRegisters(data);
                break;
            }

            case PDEventType_getCallstack:
            {
                if (shouldSendCommand(data))
					setCallstack(data, reader, writer); 

                break;
            }

            case PDEventType_getDisassembly:
            {
                if (shouldSendCommand(data))
                    getDisassembly(data, reader, writer);

                break;
            }

            case PDEventType_getMemory:
            {
                if (shouldSendCommand(data))
                    getMemory(data, reader, writer);
                break;
            }

            case PDEventType_menuEvent:
            {
                onMenu(data, reader);
                break;
            }

            case PDEventType_setBreakpoint:
            {
                setBreakpoint(data, reader, writer);

                // if we add a breakpoint to VICE it will always stop but if we are already running when
                // adding the breakpoint we just force VICE to run again

                if (data->state == PDDebugState_running)
                    sendCommand(data, "ret\n");

                break;
            }

            case PDEventType_deleteBreakpoint:
            {
                delBreakpoint(data, reader, writer);
                break;
            }

			case PDEventType_setExecutable:
			{
                //if (shouldSendCommand(data))
              	setExecutable(data, reader);

				break;
			}
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint16_t findRegisterInString(const char* str, const char* needle)
{
    const char* offset = strstr(str, needle);

    size_t needleLength = strlen(needle);

    if (!offset)
    {
        log_debug("findRegisterInString: Unable to find %s in %s\n", needle, str);
        return 0;
    }

    offset += needleLength;

    return (uint16_t)strtol(offset, 0, 16);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint16_t findStatusInString(const char* str)
{
    const char* offset = strstr(str, "SP:");

    if (!offset)
        return 0;

    // Expected string to look like this;
    // SP:XX ..-...Z. and here we jump so we are at this pos: ..-...Z.

    offset += 6;

    uint8_t flags = 0;

    for (int i = 7; i > 0; i--)
    {
        const char c = *offset++;

        if (c != '.')
            flags |= 1 << i;
    }

    return flags;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void stopOnExec(PluginData* plugin, const char* data)
{
    const char* stopOnExec = "Stop on  exec";
    char* found = 0;

    if (!(found = strstr(data, stopOnExec)))
        return;

    int execLen = (int)strlen(stopOnExec);
    found += execLen;

    plugin->regs.pc = (uint16_t)strtol(found, 0, 16);
    plugin->regs.a = (uint8_t)findRegisterInString(found, "A:");
    plugin->regs.x = (uint8_t)findRegisterInString(found, "X:");
    plugin->regs.y = (uint8_t)findRegisterInString(found, "Y:");
    plugin->regs.sp = (uint8_t)findRegisterInString(found, "SP:");
    plugin->regs.flags = (uint8_t)findStatusInString(found);

    plugin->hasUpdatedRegistes = true;
    plugin->hasUpdatedExceptionLocation = true;
    plugin->state = PDDebugState_stopBreakpoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* findRegStart(const char* res)
{
    char c = *res++;

    while (c != 0 && c != '\n')
    {
        if (c == '-')
            return res;

        c = *res++;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void updateEvents(PluginData* plugin)
{
    char* res = 0;
    int len = 0;

    if (!plugin->conn)
        return;

    // Fetch the data that has been sent from VICE

    if (!getData(plugin, &res, &len))
        return;

    plugin->state = PDDebugState_stopException;

    // do data parsing here

    stopOnExec(plugin, res);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool parseOnStepCall(PluginData* plugin, const char* res, int len, PDReader* reader, PDWriter* writer)
{
    const char* regStart;
    const char* step = strstr(res, ".C");

    (void)reader;
    (void)writer;
    (void)len;

    if (!step)
        return false;

    if (!(regStart = findRegStart(res)))
        return false;

    // return data from VICE is of the follwing format:
    // .C:0811  EE 20 D0    INC $D020      - A:00 X:17 Y:17 SP:f6 ..-.....   19262882

    plugin->regs.pc = (uint16_t)strtol(&step[3], 0, 16);
    plugin->regs.a = (uint8_t)findRegisterInString(regStart, "A:");
    plugin->regs.x = (uint8_t)findRegisterInString(regStart, "X:");
    plugin->regs.y = (uint8_t)findRegisterInString(regStart, "Y:");
    plugin->regs.sp = (uint8_t)findRegisterInString(regStart, "SP:");
    plugin->regs.flags = (uint8_t)findStatusInString(res);

    plugin->hasUpdatedRegistes = true;
    plugin->hasUpdatedExceptionLocation = true;
    plugin->state = PDDebugState_trace;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Sent to VICE: z
//
// Expected result
//
// .C:xxxx  xx xx xx    xx xxC6        - A:xx X:xx Y:xx SP:xx ........    xxxxxxx
//
// Example
//
// .C:e5cd  A5 C6       LDA $C6        - A:00 X:00 Y:0A SP:f3 ..-...Z.    5719913
// (C:$e5cd) 

bool onStep(PluginData* data, PDReader* reader, PDWriter* writer)
{
	return sendCommandGetData(data, "z\n", parseOnStepCall, reader, writer, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onAction(PluginData* plugin, PDAction action)
{
    switch (action)
    {
        case PDAction_none:
            break;

        case PDAction_stop:
        {
            sendCommand(plugin, "n\n");
            break;
        }

        case PDAction_break:
        {
            sendCommand(plugin, "n\n");
            break;
        }

        case PDAction_run:
        {
            //if (plugin->state != PDDebugState_running)
            {
                sendCommand(plugin, "ret\n");
                plugin->state = PDDebugState_running;
            }

            break;
        }

        case PDAction_step:
        {
            onStep(plugin, 0, 0);
            break;
        }

        case PDAction_stepOver:
        {
            sendCommand(plugin, "n\n");
            break;
        }

        case PDAction_stepOut:
        case PDAction_custom:
        {
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
    PluginData* plugin = (PluginData*)userData;

    plugin->hasUpdatedRegistes = false;
    plugin->hasUpdatedExceptionLocation = false;

    onAction(plugin, action);

    processEvents(plugin, reader, writer);

    updateEvents(plugin);

    if (plugin->hasUpdatedRegistes)
    {
		log_debug("sending registens\n", "");

        PDWrite_eventBegin(writer, PDEventType_setRegisters);
        PDWrite_arrayBegin(writer, "registers");

        writeStatusRegister(writer, "flags", plugin->regs.flags);
        writeRegister(writer, "pc", 2, plugin->regs.pc, 1);
        writeRegister(writer, "sp", 1, plugin->regs.sp, 0);
        writeRegister(writer, "a", 1, plugin->regs.a, 0);
        writeRegister(writer, "x", 1, plugin->regs.x, 0);
        writeRegister(writer, "y", 1, plugin->regs.y, 0);

        PDWrite_arrayEnd(writer);
        PDWrite_eventEnd(writer);
    }

    if (plugin->hasUpdatedExceptionLocation)
    {
        PDWrite_eventBegin(writer, PDEventType_setExceptionLocation);
        PDWrite_u64(writer, "address", plugin->regs.pc);
        PDWrite_u8(writer, "address_size", 2);
        PDWrite_eventEnd(writer);
    }

    return plugin->state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenuItem s_menu0[] =
{
    { "Attach To VICE", C64_VICE_MENU_ATTACH_TO_VICE, 0, 0, 0 },
    { "Start With Config", C64_VICE_MENU_START_WITH_CONFIG, 256 + 3, 0, 0 }, // key hack
    { "Detach From VICE", C64_VICE_MENU_DETACH_FROM_VICE, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenu s_menus[] =
{
    { "C64 VICE", (PDMenuItem*)&s_menu0 },
    { 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenu* createMenu()
{
    return (PDMenu*)&s_menus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin =
{
    "C64 VICE Debugger",
    createInstance,
    destroyInstance,
    createMenu,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
{
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, privateData);
    registerPlugin(PD_VIEW_API_VERSION, &g_c64CustomViewPlugin, privateData);
}


