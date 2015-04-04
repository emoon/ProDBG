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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_debug(const char* format, ...)
{
    va_list ap;
	char buffer[2048];

    va_start(ap, format);
	vsprintf(buffer, format, ap);
    va_end(ap);

#ifdef _WIN32
	OutputDebugStringA(buffer);
#else
	printf(buffer);
#endif
}

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
	char* condition;
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
    realpath(name, fullName);

    return true;
#endif

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* loadToMemory(const char* filename, size_t* size)
{
    FILE* f = fopen(filename, "rb");
    void* data;
    size_t s;

    *size = 0;

    if (!f)
        return 0;

    // TODO: Use fstat here?

    fseek(f, 0, SEEK_END);
    s = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    data = malloc(s);

    if (!data)
        return 0;

    fread(data, s, 1, f);

    *size = s;

    fclose(f);

    return data;
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

    log_debug("loaded config\n");

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

    //printf("sent command %s (%d - %d)\n", buffer, len, ret);

    sleepMs(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getData(PluginData* data, char** resBuffer, int* len)
{
    const int maxTry = 1;
    int res = 0;

    if (!data->conn)
        return 0;

    char* resData = (char*)&s_recvBuffer;
    int lenCount = 0;

    memset(resData, 0, 1024);

    for (int i = 0; i < maxTry; ++i)
    {
        bool gotData = false;

        while (VICEConnection_pollRead(data->conn))
        {
            res = VICEConnection_recv(data->conn, resData, ((int)sizeof(s_recvBuffer)) - lenCount, 0);

            if (res == 0)
                break;

            gotData = true;

            resData += res;
            lenCount += res;
        }

        if (gotData)
        {
            *len = lenCount;
            *resBuffer = (char*)&s_recvBuffer;

            return 1;
        }

        // Got some data so read it back
    }

    // got no data

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

static bool delBreakpointById(PluginData* data, int32_t id)
{
	const int breakpointCount = data->breakpoints.count;

	for (int i = 0, end = data->breakpoints.count; i < end; ++i)
	{
		Breakpoint* bp = data->breakpoints.data[i];

		if (bp->id == id)
		{
			sendCommand(data, "del %d\n", id);

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

	printf("deleting breakpoint with id %d\n", id);

	return delBreakpointById(data, id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setBreakpoint(PluginData* data, PDReader* reader, PDWriter* writer)
{
	uint64_t address = 0;
	int32_t id = -1;
	const char* condition = 0;

	(void)writer;

    PDRead_findS32(reader, &id, "id", 0);
	PDRead_findU64(reader, &address, "address", 0);
	PDRead_findString(reader, &condition, "condition", 0);

	if (id != -1)
		delBreakpointById(data, id);

    if (condition)
		sendCommand(data, "break $%04x if %s\n", (uint16_t)address, condition);
	else
		sendCommand(data, "break $%04x\n", (uint16_t)address);
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
    
    data->breakpoints.data = (Breakpoint**)malloc(sizeof(Breakpoint**) * maxBreakpointCount);
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

    sendCommand(data, "save \"%s\" 0 %04x %04x\n", tempfile, address, addressEnd);

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

        return mem;
    }

    printf("Unable to get memory...\n");

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
		printf("failed to get any data back from load...\n");
    	return false;
	}

	printf("res %s\n", res);

	// TODO: Parse that we actually manage to load the image

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint16_t getBasicStart(PluginData* data)
{
	size_t readSize = 0;

	uint8_t* memory = getMemoryInternal(data, data->tempFileFull, &readSize, 0x800, 0x810); 

	if (!memory)
	{
		printf("falied to get memory :(\n");
		return 0;
	}

	printf("size read %d\n", (int)readSize);

	const char* address = (char*)&memory[2 + 6];

	printf("memory\n");

	for (int i = 0; i < 18; ++i)
	{
		char c = (char)memory[i];
		char pc = (c >= 32 && c < 127) ? c : '.';
		printf("%c ", pc);
	}

	printf("\n");

	for (int i = 0; i < 18; ++i)
	{
		char c = (char)memory[i];
		printf("%02x ", c);
	}

	printf("\n");

	uint16_t startAddress = (uint16_t)strtol(address, 0, 10);

	printf("basic: address to start from (text) %s\n", address);
	printf("start address %d %x\n", startAddress, startAddress);

	free(memory);

	return startAddress;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void launchVICEWithConfig(PluginData* data)
{
	int r, cmdIndex = 1;
	uv_process_options_t options = { 0 };

	log_debug("spawning vice...\n");

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
		printf("connected to vice...\n");

		if (!loadImage(data, data->config.prgFile))
			return;

		printf("image loaded ...\n");

		// parse the 

		parseMonFile(data, data->config.breakpointFile);

		printf("start from basic...\n");

		// start vice!
		
		printf("started from basic\n");

		uint16_t address = getBasicStart(data);

		parseMonFile(data, data->config.breakpointFile);

		if (address != 0)
		{
			printf("start from %x\n", address);
			sendCommand(data, "g %x\n", address);
		}

		return;
	}

	printf("unable to make connection with vice\n");
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

static void parseRegisters(PluginData* plugin, char* data, int length)
{
	const char* pch;
	struct Regs6510* regs = &plugin->regs;

	memcpy(s_tempBuffer, data, length);
	s_tempBuffer[length] = 0;

    // Format from VICE looks like this:
    // (C:$e5cf)   ADDR AC XR YR SP 00 01 NV-BDIZC LIN CYC  STOPWATCH
    //           .;e5cf 00 00 0a f3 2f 37 00100010 000 001    3400489
    //
    //

    char* str = strstr(s_tempBuffer, ".;");

    if (!str)
        return;

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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void parseBreakpoint(PluginData* data, const char* res, PDWriter* writer)
{
	Breakpoint* bp = 0;

	// TODO: loop, look for more breakpoints

    const char* breakStrOffset = strstr(res, "BREAK:");

	if (!breakStrOffset) 
		return;

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

	printf("sending reply back: breakpoint %x - %d\n", bp->address, id);

	// TODO: Condition

	//if (bp->condition)
	//	free(bp->condition);

	//if (condition)
	//	bp->condition = strdup(condition);
	// else
	// bp->condition = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getDisassembly(PluginData* data, PDReader* reader)
{
    uint64_t addressStart = 0;
    uint32_t instructionCount = 0;

    PDRead_findU64(reader, &addressStart, "address_start", 0);
    PDRead_findU32(reader, &instructionCount, "instruction_count", 0);

    // assume that one instruction is 3 bytes which is high but that gives us more data back than we need which is
    // better than too little

    sendCommand(data, "disass $%04x $%04x\n", (uint16_t)addressStart, (uint16_t)(addressStart + instructionCount * 3));
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

    	printf("c64_vice: sending memory\n");

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

	printf("debugstate %d\n", data->state);
	printf("should send command %d %d\n", t0, t1);

	return t0 && t1;
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
               	sendCommand(data, "registers\n");
                break;
            }

            case PDEventType_getCallstack:
            {
            	if (shouldSendCommand(data))
                	sendCommand(data, "bt\n");

                break;
            }

            case PDEventType_getDisassembly:
            {
            	if (shouldSendCommand(data))
                    getDisassembly(data, reader);

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

void onStep(PluginData* plugin)
{
    sendCommand(plugin, "n\n");
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
    		sendCommand(plugin, "z\n");
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

static void parseDisassembly(PDWriter* writer, const char* data, int length)
{
	memcpy(s_tempBuffer, data, length);
	s_tempBuffer[length] = 0;

    // parse the buffer

    char* pch = strtok(s_tempBuffer, "\n");

    PDWrite_eventBegin(writer, PDEventType_setDisassembly);
    PDWrite_arrayBegin(writer, "disassembly");

    while (pch)
    {
        // expected format of each line:
        // xxx.. .C:080e  A9 22       LDA #$22

		char* line = strstr(pch, ".C");

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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void parseForCallstack(PDWriter* writer, const char* data, int length)
{
	uint16_t callStackEntries[256 * 2];
	int callStackCount = 0;

	memcpy(s_tempBuffer, data, length);
	s_tempBuffer[length] = 0;

    char* pch = strtok(s_tempBuffer, "\n");

    while (pch)
    {
        // expected format of each line:
        // xxx.. .C:080e  A9 22       LDA #$22

        if (pch[0] == '(' &&  pch[1] != 'C')
		{
			uint32_t offset = (uint32_t)atoi(&pch[2]);

			char* endOffset = strstr(&pch[2], ") ");

			if (endOffset)
			{
				endOffset += 2;

				uint16_t address = (uint16_t)strtol(endOffset, 0, 16);

				callStackEntries[(callStackCount * 2) + 0] = address;
				callStackEntries[(callStackCount * 2) + 1] = (uint16_t)offset;

				callStackCount++;
			}
		}

		pch = strtok(0, "\n");
    }
	
	if (callStackCount == 0)
		return;

    PDWrite_eventBegin(writer, PDEventType_setCallstack);
    PDWrite_arrayBegin(writer, "callstack");

    for (int i = 0; i < callStackCount; ++i)
	{
        PDWrite_arrayEntryBegin(writer);
        PDWrite_u16(writer, "address", callStackEntries[(i * 2) + 0] + callStackEntries[(i * 2) + 1]);
        PDWrite_arrayEntryEnd(writer);
	}

    PDWrite_arrayEnd(writer);
    PDWrite_eventEnd(writer);
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

static void parseStep(PluginData* plugin, const char* res)
{
	const char* regStart;
	const char* step = strstr(res, ".C");

	if (!step)
		return;

	if (!(regStart = findRegStart(res)))
		return;

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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateEvents(PluginData* plugin, PDWriter* writer)
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

	parseRegisters(plugin, res, len);

	parseStep(plugin, res);

	parseBreakpoint(plugin, res, writer);

	parseDisassembly(writer, res, len);

	parseForCallstack(writer, res, len);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
    PluginData* plugin = (PluginData*)userData;

    plugin->hasUpdatedRegistes = false;
    plugin->hasUpdatedExceptionLocation = false;

    onAction(plugin, action);

    processEvents(plugin, reader, writer);

	updateEvents(plugin, writer);

    if (plugin->hasUpdatedRegistes)
    {
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


