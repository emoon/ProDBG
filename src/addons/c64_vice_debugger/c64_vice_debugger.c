#include "pd_backend.h"
#include "pd_menu.h"
#include "c64_vice_connection.h"
#include <stdlib.h>

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
static const int maxBreakpointCount = 8192;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    C64_VICE_MENU_ATTACH_TO_VICE,
    C64_VICE_MENU_DETACH_FROM_VICE,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Regs6510
{
    uint16_t pc;
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
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
	uint32_t id;
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
	const char* progFile;
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

	data->config.progFile = strdup("examples/c64_vice/test.prg");
	data->config.kickAssSymbols = strdup("examples/c64_vice/test.sym");
	data->config.breakpointFile = strdup("examples/c64_vice/breakpoints.txt");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void loadConfig(PluginData* data, const char* filename)
{
	const char* viceExe = 0;
	const char* progFile = 0;
	const char* kickAssSymbols = 0;
	const char* breakpointFile = 0;
    json_error_t error;

	setupDefaultConfig(data);

    json_t* root = json_load_file(filename, 0, &error);

    if (!root || !json_is_object(root))
        return;

	json_unpack(root, "{s:s, s:s, s:s, s:s}",
		"vice_exe", &viceExe,
		"prg_file", &progFile,
		"kickass_symbols", &kickAssSymbols,
		"breakpoints_file", &breakpointFile);

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

    printf("send command %s", buffer);

    VICEConnection_send(data->conn, buffer, len, 0);

	sleepMs(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getData(PluginData* data, char** resBuffer, int* len)
{
    const int maxTry = 100;
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

        sleepMs(1);
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

static bool findBreakpointById(PluginData* data, Breakpoint** breakpoint, uint32_t id) 
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

static Breakpoint* getBreakpoint(PluginData* data, uint64_t* address, PDReader* reader, PDWriter* writer)
{
	uint32_t id;
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
		PDWrite_string(writer, "error", "No address is being sent for breakpoint");
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setBreakpoint(PluginData* data, PDReader* reader, PDWriter* writer)
{
	uint64_t address;
    char* res = 0;
    int len = 0;
    int internalId = 0;
    const char* condition = 0;

	Breakpoint* bp = getBreakpoint(data, &address, reader, writer);

    PDRead_findString(reader, &condition, "condition", 0);

    // if the bp already exists we delete it first

	if (bp->internalId != -1)
	{
		sendCommand(data, "del %d\n", bp->internalId);
		getData(data, &res, &len);	// TODO: Handle the data?
	}

	getData(data, &res, &len);

    if (condition)
		sendCommand(data, "break $%04x if %s\n", (uint16_t)address, condition);
	else
		sendCommand(data, "break $%04x\n", (uint16_t)address);

	getData(data, &res, &len);

	printf("setBreakpoint %s\n", res);
	
	if (strncmp(res, "BREAK: ", 6) == 0)
	{
		internalId = atoi(res + 7);
		printf("Interanl bp id %d\n", internalId);
	}
	else
	{
    	PDWrite_eventBegin(writer, PDEventType_replyBreakpoint);
		PDWrite_string(writer, "error", res); 
		PDWrite_eventEnd(writer);
		return;
	}

	// add data or update existing

	bp->address = (uint16_t)address;

	if (bp->condition)
		free(bp->condition);

	if (condition)
		bp->condition = strdup(condition);
	else
		bp->condition = 0;

	if (bp->internalId == -1)
		addBreakpoint(data, bp);

	bp->internalId = internalId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool delBreakpoint(PluginData* data, PDReader* reader, PDWriter* writer)
{
    char* res = 0;
    int len = 0;
	uint32_t id;

    if (PDRead_findU32(reader, &id, "id", 0) == PDReadStatus_notFound)
	{
    	PDWrite_eventBegin(writer, PDEventType_replyBreakpoint);
		PDWrite_string(writer, "error", "No ID being sent for breakpoint");
		PDWrite_eventEnd(writer);
		return false;
	}

	const int breakpointCount = data->breakpoints.count;

	printf("breakpointCount %d\n", breakpointCount);

	for (int i = 0, end = data->breakpoints.count; i < end; ++i)
	{
		Breakpoint* bp = data->breakpoints.data[i];

		if (bp->id == id)
		{
			sendCommand(data, "del %d\n", bp->internalId);
			getData(data, &res, &len);	// TODO: Handle the data?

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

    //TODO: non fixed size?
    
    data->breakpoints.data = (Breakpoint**)malloc(sizeof(Breakpoint**) * maxBreakpointCount);
    data->breakpoints.count = 0;

    connectToLocalHost(data);

    return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroyInstance(void* userData)
{
    PluginData* plugin = (PluginData*)userData;

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

static void parseRegisters(struct Regs6510* regs, char* str)
{
	const char* pch;

    // Format from VICE looks like this:
    // (C:$e5cf)   ADDR AC XR YR SP 00 01 NV-BDIZC LIN CYC  STOPWATCH
    //           .;e5cf 00 00 0a f3 2f 37 00100010 000 001    3400489
    //

    str = strstr(str, ".;");

    if (!str)
        return;

    pch = strtok(str, " \t\n");

    regs->pc = (uint16_t)strtol(&pch[2], 0, 16); pch = strtok(0, " \t");
    regs->a = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
    regs->x = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
    regs->y = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
    regs->sp = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getRegisters(PluginData* data)
{
    char* res = 0;
    int len = 0;

    sendCommand(data, "registers\n");

    if (!getData(data, &res, &len))
        return;

    data->state = PDDebugState_stopException;

    parseRegisters(&data->regs, res);

    data->hasUpdatedRegistes = true;
    data->hasUpdatedExceptionLocation = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getDisassembly(PluginData* data, PDReader* reader, PDWriter* writer)
{
    char* res = 0;
    int len = 0;

    uint64_t addressStart = 0;
    uint32_t instructionCount = 0;

    PDRead_findU64(reader, &addressStart, "address_start", 0);
    PDRead_findU32(reader, &instructionCount, "instruction_count", 0);

    // assume that one instruction is 3 bytes which is high but that gives us more data back than we need which is
    // better than too little

    sendCommand(data, "disass $%04x $%04x\n", (uint16_t)addressStart, (uint16_t)(addressStart + instructionCount * 3));

    if (!getData(data, &res, &len))
        return;

    // parse the buffer

    char* pch = strtok(res, "\n");

    PDWrite_eventBegin(writer, PDEventType_setDisassembly);
    PDWrite_arrayBegin(writer, "disassembly");

    while (pch)
    {
        // expected format of each line:
        // .C:080e  A9 22       LDA #$22

        if (pch[0] != '.')
            break;

        uint16_t address = (uint16_t)strtol(&pch[3], 0, 16);

        PDWrite_arrayEntryBegin(writer);
        PDWrite_u16(writer, "address", address);
        PDWrite_string(writer, "line", &pch[9]);

        PDWrite_arrayEntryEnd(writer);

        pch = strtok(0, "\n");
    }

    PDWrite_arrayEnd(writer);
    PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getMemory(PluginData* data, PDReader* reader, PDWriter* writer)
{
    uint64_t address;
    uint64_t size;

    PDRead_findU64(reader, &address, "address_start", 0);
    PDRead_findU64(reader, &size, "size", 0);

    sendCommand(data, "save \"%s\" 0 %04x %04x\n", data->tempFileFull, (uint16_t)(address), (uint16_t)(address + size));

    // Wait 10 ms for operation to complete and if we can't open the file we try for a few times and if we still can't we bail

    sleepMs(10);

    for (int i = 0; i < 10; ++i)
    {
        size_t readSize = 0;

        uint8_t* mem = loadToMemory(data->tempFileFull, &readSize);

        if (!mem)
        {
            sleepMs(1);
            continue;
        }

        // Lets do this!
        // + 2 is because VICE writes address at the start of the block and at the end

        PDWrite_eventBegin(writer, PDEventType_setMemory);
        PDWrite_u64(writer, "address", address);
        PDWrite_data(writer, "data", mem + 2, (uint32_t)(readSize - 3));
        PDWrite_eventEnd(writer);

        // writer takes a copy

        free(mem);

        return;
    }
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
                if (!data->hasUpdatedRegistes)
                    getRegisters(data);

                break;
            }

            case PDEventType_getDisassembly:
            {
                getDisassembly(data, reader, writer);
                break;
            }

            case PDEventType_getMemory:
            {
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
        printf("findRegisterInString: Unable to find %s in %s\n", needle, str);
        return 0;
    }

    offset += needleLength;

    return (uint16_t)strtol(offset, 0, 16);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void onStep(PluginData* plugin)
{
    char* res = 0;
    int len = 0;

    sendCommand(plugin, "n\n");

    if (!getData(plugin, &res, &len))
        return;

    printf("getRegistes %s\n.................................\n", res);

    // return data from VICE is of the follwing format:
    // .C:0811  EE 20 D0    INC $D020      - A:00 X:17 Y:17 SP:f6 ..-.....   19262882
    
    plugin->regs.pc = (uint16_t)strtol(&res[3], 0, 16);
    plugin->regs.a = (uint8_t)findRegisterInString(res, "A:");
    plugin->regs.x = (uint8_t)findRegisterInString(res, "X:");
    plugin->regs.y = (uint8_t)findRegisterInString(res, "Y:");
    plugin->regs.sp = (uint8_t)findRegisterInString(res, "SP:");

    plugin->hasUpdatedRegistes = true;
    plugin->hasUpdatedExceptionLocation = true;
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
            if (plugin->state != PDDebugState_running)
                sendCommand(plugin, "ret\n");

            break;
        }

        case PDAction_step:
        {
            onStep(plugin);
            break;
        }

        case PDAction_stepOver:
        case PDAction_stepOut:
        case PDAction_custom:
        {
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateEvents(PluginData* data)
{
	const char* stopOnExec = "Stop on  exec";
	char* found = 0;
    char* res = 0;
    int len = 0;

	if (!data->conn || !VICEConnection_pollRead(data->conn))
		return;

	// Fetch the data that has been sent from VICE

    if (!getData(data, &res, &len))
        return;

    if ((found = strstr(res, stopOnExec)))
	{
		printf("stop on exec %s\n", res);

    	size_t execLen = strlen(stopOnExec);
    	found += execLen;

		data->regs.pc = (uint16_t)strtol(found, 0, 16);
		data->regs.a = (uint8_t)findRegisterInString(found, "A:");
		data->regs.x = (uint8_t)findRegisterInString(found, "X:");
		data->regs.y = (uint8_t)findRegisterInString(found, "Y:");
		data->regs.sp = (uint8_t)findRegisterInString(found, "SP:");

		data->hasUpdatedRegistes = true;
		data->hasUpdatedExceptionLocation = true;
		data->state = PDDebugState_stopBreakpoint;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
    PluginData* plugin = (PluginData*)userData;

    plugin->hasUpdatedRegistes = false;
    plugin->hasUpdatedExceptionLocation = false;

	updateEvents(plugin);

    onAction(plugin, action);

    processEvents(plugin, reader, writer);

    if (plugin->hasUpdatedRegistes)
    {
        PDWrite_eventBegin(writer, PDEventType_setRegisters);
        PDWrite_arrayBegin(writer, "registers");

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
    { "Attach to VICE", C64_VICE_MENU_ATTACH_TO_VICE, 0, 0, 0 },
    { "Detach from VICE", C64_VICE_MENU_DETACH_FROM_VICE, 0, 0, 0 },
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
}


