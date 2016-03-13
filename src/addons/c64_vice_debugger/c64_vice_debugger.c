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

static char RECV_BUFFER[512 * 1024];
static char TEMP_BUFFER[512 * 1024];
static const int MAX_BREAKPOINT_COUNT = 8192;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    C64_VICE_MENU_ATTACH_TO_VICE,
    C64_VICE_MENU_START_WITH_CONFIG,
    C64_VICE_MENU_DETACH_FROM_VICE,
};

static PDMessageFuncs* MESSAGE_FUNCS;

#ifdef _WIN32
__declspec(dllimport) void OutputDebugStringA(const char*);
#endif

static bool doDebug = false;

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void log_debug_printf(const char* format, ...) {
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
            log_debug_printf("%s:(%d) " fmt, __FILENAME__, __LINE__, __VA_ARGS__); \
    } while (0)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Regs6510 {
    uint16_t pc;
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint16_t flags;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum BreakpoinType {
    BreakpointType_Normal,
    BreakpointType_Data,
} BreakpoinType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct {
    //char* condition;
    uint16_t address;
    BreakpoinType type;
    int32_t id;
    int32_t internalId;

} Breakpoint;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Breakpoints {
    Breakpoint** data;
    int count;
} Breakpoints;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Config {
    const char* vice_exe;
    const char* prg_file;
    const char* kick_ass_symbols;
    const char* breakpoint_file;
} Config;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PluginData {
    struct VICEConnection* conn;
    struct Regs6510 regs;
    bool has_updated_registers;
    bool has_updated_exception_location;
    PDDebugState state;
    char temp_file_full[8192];
    Breakpoints breakpoints;
    Config config;
    uv_process_t process;

} PluginData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
typedef bool (*ParseDataFunc)(PluginData* data, const char* res, int len, PDReader* reader, PDWriter* writer);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Add to services

static void sleepMs(int ms) {
#ifdef _MSC_VER
    Sleep(ms);
#else
    usleep((unsigned int)(ms * 1000));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool get_full_name(char* fullName, const char* name) {
#ifdef _MSC_VER
    if (GetFullPathNameA(name, 8192, fullName, 0) == 0) {
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

void* load_to_memory(const char* filename, size_t* size) {
    FILE* f = fopen(filename, "rb");
    void* data = 0;
    size_t s = 0;

    *size = 0;

    if (!f)
        return 0;

    // TODO: Use fstat here?

    fseek(f, 0, SEEK_END);
    long ts = ftell(f);

    if (ts < 0) {
        goto end;
	}

    s = (size_t)ts;

    data = malloc(s);

    if (!data) {
        goto end;
	}

    fseek(f, 0, SEEK_SET);

    size_t t = fread(data, s, 1, f);
    (void)t;

    *size = s;

    end:

    fclose(f);

    return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int parse_prg(const char* filename) {
    size_t size = 0;

    const char* data = (const char*)load_to_memory(filename, &size);

    if (!data) {
        return -1;
	}

    if (size < 10) {
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

static void setupDefaultConfig(PluginData* data) {
#ifdef PRODBG_MAC
    data->config.vice_exe = strdup("/Applications/VICE/x64.app/Contents/MacOS/x64");
#elif PRODBG_WIN
    data->config.vice_exe = strdup("x64.exe");
#else
    data->config.vice_exe = strdup("x64");
#endif

    data->config.prg_file = strdup("examples/c64_vice/test.prg");
    data->config.kick_ass_symbols = 0;
    data->config.breakpoint_file = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void load_config(PluginData* data, const char* filename) {
    const char* vice_exe = 0;
    const char* prg_file = 0;
    const char* kick_ass_symbols = 0;
    const char* breakpoint_file = 0;
    json_error_t error;

    setupDefaultConfig(data);

    json_t* root = json_load_file(filename, 0, &error);

    if (!root || !json_is_object(root)) {
        return;
	}

    log_debug("loaded config\n", "");

    json_unpack(root, "{s:s, s:s, s:s, s:s}",
                "vice_exe", &vice_exe,
                "prg_file", &prg_file,
                "kickass_symbols", &kick_ass_symbols,
                "breakpoints_file", &breakpoint_file);

    if (vice_exe && vice_exe[0] != 0) {
        data->config.vice_exe = strdup(vice_exe);
	}

    if (prg_file && prg_file[0] != 0) {
        data->config.prg_file = strdup(prg_file);
	}

    if (breakpoint_file && breakpoint_file[0] != 0) {
        data->config.breakpoint_file = strdup(breakpoint_file);
	}

    if (kick_ass_symbols && kick_ass_symbols[0] != 0) {
        data->config.kick_ass_symbols = strdup(kick_ass_symbols);
	}

    json_decref(root);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void send_command(PluginData* data, const char* format, ...) {
    va_list ap;
    char buffer[2048];

    va_start(ap, format);
    vsprintf(buffer, format, ap);
    va_end(ap);

    int len = (int)strlen(buffer);

    if (!data->conn) {
        return;
	}

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

static bool get_dataToBuffer(PluginData* data, char* resBuffer, int bufferSize, int* len, int maxTry) {
    int res = 0;
    int lenCount = 0;

    if (!data->conn) {
        return false;
	}

    memset(resBuffer, 0, bufferSize);

    for (int i = 0; i < maxTry; ++i) {
        bool gotData = false;

        //log_debug("trying to get data %d\n", i);

        while (VICEConnection_pollRead(data->conn)) {
            res = VICEConnection_recv(data->conn, resBuffer, bufferSize - lenCount, 0);

            if (res == 0)
                break;

            gotData = true;

            resBuffer += res;
            lenCount += res;
        }

        if (gotData) {
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

static int get_data(PluginData* data, char** resBuffer, int* len) {
    if (!get_dataToBuffer(data, RECV_BUFFER, sizeof(RECV_BUFFER), len, 1))
        return false;

    *resBuffer = (char*)&RECV_BUFFER;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int wait_for_data(PluginData* data, char** resBuffer, int* len) {
    const int maxTry = 1000;

    for (int i = 0; i < maxTry; ++i) {
        get_data(data, resBuffer, len);

        if (*len != 0)
            return 1;

        sleepMs(1);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Breakpoint* create_breakpoint() {
    Breakpoint* bp = (Breakpoint*)malloc(sizeof(Breakpoint));
    memset(bp, 0, sizeof(Breakpoint));
    return bp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void add_breakpoint(PluginData* data, Breakpoint* bp) {
    assert(data->breakpoints.count < MAX_BREAKPOINT_COUNT);
    data->breakpoints.data[data->breakpoints.count++] = bp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool find_breakpoint_by_id(PluginData* data, Breakpoint** breakpoint, int id) {
    for (int i = 0, end = data->breakpoints.count; i < end; ++i) {
        Breakpoint* bp = data->breakpoints.data[i];

        if (bp->id == id) {
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

    if (PDRead_find_u32(reader, &id, "id", 0) == PDReadStatus_NotFound)
    {
        PDWrite_event_begin(writer, PDEventType_replyBreakpoint);
        PDWrite_string(writer, "error", "No ID being sent for breakpoint");
        PDWrite_event_end(writer);
        return 0;
    }

    if (PDRead_find_u64(reader, address, "address", 0) == PDReadStatus_NotFound)
    {
        PDWrite_event_begin(writer, PDEventType_replyBreakpoint);
        PDWrite_string(writer, "error", "prodNo address is being sent for breakpoint");
        PDWrite_event_end(writer);
        return 0;
    }

    if (!find_breakpoint_by_id(data, &bp, id))
    {
        bp = create_breakpoint();
        bp->id = id;
        bp->internalId = -1;
    }

    return bp;
   }
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool send_command_get_data(PluginData* data, const char* sendCmd, ParseDataFunc parseFunc, PDReader* reader, PDWriter* writer, int maxTry) {
    char* res = RECV_BUFFER;
    int len = 0;

    send_command(data, sendCmd);

    for (int i = 0; i < maxTry; ++i) {
        int tempLen = 0;

        // We give VICE 1 sek to does this thing. TODO: Lower this value or have it as a config?

        if (!get_dataToBuffer(data, res, (int)(sizeof(RECV_BUFFER)) - len, &tempLen, 1000)) {
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

static bool check_for_default_state(PluginData* data, const char* res, int len, PDReader* reader, PDWriter* writer) {
    (void)data; (void)len; (void)reader; (void)writer;
    return strstr(res, "(C:$");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool del_breakpoint_by_id(PluginData* data, int32_t id) {
    const int breakpointCount = data->breakpoints.count;

    for (int i = 0, end = data->breakpoints.count; i < end; ++i) {
        Breakpoint* bp = data->breakpoints.data[i];

        if (bp->id == id) {
            char temp[1024];
            sprintf(temp, "del %d\n", id);

            send_command_get_data(data, temp, check_for_default_state, 0, 0, 20);

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

static bool del_breakpoint(PluginData* data, PDReader* reader, PDWriter* writer) {
    int32_t id;

    if (PDRead_find_s32(reader, &id, "id", 0) == PDReadStatus_NotFound) {
        PDWrite_event_begin(writer, PDEventType_ReplyBreakpoint);
        PDWrite_string(writer, "error", "No ID being sent for breakpoint");
        PDWrite_event_end(writer);
        return false;
    }

    log_debug("deleting breakpoint with id %d\n", id);

    return del_breakpoint_by_id(data, id);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void connect_to_local_host(PluginData* data) {
    struct VICEConnection* conn = 0;

    // Kill the current connection if we have one

    if (data->conn) {
        VICEConnection_destroy(data->conn);
        data->conn = 0;
    }

    conn = VICEConnection_create(VICEConnectionType_Connect, 6510);

    if (!VICEConnection_connect(conn, "localhost", 6510)) {
        VICEConnection_destroy(conn);

        data->conn = 0;
        data->state = PDDebugState_NoTarget;

        return;
    }

    data->conn = conn;
    data->state = PDDebugState_Running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* create_instance(ServiceFunc* serviceFunc) {
    (void)serviceFunc;

    PluginData* data = malloc(sizeof(PluginData));
    memset(data, 0, sizeof(PluginData));

    get_full_name((char*)&data->temp_file_full, "temp/vice_mem_dump");

    data->state = PDDebugState_NoTarget;

    load_config(data, "data/c64_vice.cfg");

    MESSAGE_FUNCS = serviceFunc(PDMESSAGEFUNCS_GLOBAL);

    //TODO: non fixed size?

    data->breakpoints.data = (Breakpoint**)malloc(sizeof(Breakpoint*) * MAX_BREAKPOINT_COUNT);
    data->breakpoints.count = 0;

    return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void parse_mon_file(PluginData* data, const char* filename) {
    char textLine[1024];

    FILE* f = fopen(filename, "rt");

    if (!f)
        return;

    for (;;) {
        if (!fgets(textLine, sizeof(textLine), f))
            break;

        send_command(data, "%s\n", textLine);
    }

    fclose(f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t* get_memory_internal(PluginData* data, const char* tempfile, size_t* read_size, uint16_t address, uint16_t addressEnd) {
    *read_size = 0;

#ifndef _WIN32
    if (unlink(tempfile) < 0) {
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

    send_command(data, "save \"%s\" 0 %04x %04x\n", tempfile, address, addressEnd);

    // TODO: Improve this? (wait for (C: as return data back from send command)

    // Wait 10 ms for operation to complete and if we can't open the file we try for a few times and if we still can't we bail

    sleepMs(10);

    for (int i = 0; i < 10; ++i) {
        uint8_t* mem = load_to_memory(data->temp_file_full, read_size);

        if (!mem) {
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

static bool load_image(PluginData* data, const char* filename) {
    char* res = 0;
    int len = 0;

    send_command(data, "load \"%s\" 0\n", filename);

    sleepMs(200);

    if (!get_data(data, &res, &len)) {
        log_debug("failed to get any data back from load...\n", "");
        return false;
    }

    log_debug("res %s\n", res);

    // TODO: Parse that we actually manage to load the image

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t get_basic_start(PluginData* data) {
    size_t read_size = 0;

    uint8_t* memory = get_memory_internal(data, data->temp_file_full, &read_size, 0x800, 0x810);

    if (!memory) {
        log_debug("falied to get memory :(\n", "");
        return 0;
    }

    log_debug("size read %d\n", (int)read_size);

    const char* address = (char*)&memory[2 + 6];

    log_debug("memory\n", "");

    for (int i = 0; i < 18; ++i) {
        char c = (char)memory[i];
        char pc = (c >= 32 && c < 127) ? c : '.';
        log_debug("%c ", pc);
    }

    log_debug("\n", "");

    for (int i = 0; i < 18; ++i) {
        char c = (char)memory[i];
        log_debug("%02x ", c);
    }

    log_debug("\n", "");

    uint16_t start_address = (uint16_t)strtol(address, 0, 10);

    log_debug("basic: address to start from (text) %s\n", address);
    log_debug("start address %d %x\n", start_address, start_address);

    free(memory);

    return start_address;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void launch_vice_with_config(PluginData* data) {
    int r, cmdIndex = 1;
    uv_process_options_t options = { 0 };

    log_debug("spawning vice...\n", "");

    char* args[10];
    args[0] = (char*)data->config.vice_exe;

    // TODO: Must generate the breakpoint file from the json one

    args[cmdIndex++] = "-remotemonitor";

    if (data->config.breakpoint_file) {
        //args[cmdIndex++] = "-moncommands";
        //args[cmdIndex++] = "examples/c64_vice/test_mon.txt";
    }

    //args[cmdIndex++] = (char*)data->config.prg_file;
    args[cmdIndex++] = NULL;

    options.exit_cb = 0;
    options.file = data->config.vice_exe;
    options.args = args;

    if ((r = uv_spawn(uv_default_loop(), &data->process, &options))) {
        MESSAGE_FUNCS->error("Unable to launch VICE", uv_strerror(r));
        return;
    }

    sleepMs(3000);

    connect_to_local_host(data);

    // if connected we load the image and make sure we get a reply back

    if (VICEConnection_isConnected(data->conn)) {
        log_debug("connected to vice...\n", "");

        if (!load_image(data, data->config.prg_file)) {
            return;
		}

        log_debug("image loaded ...\n", "");

        // parse the

        parse_mon_file(data, data->config.breakpoint_file);

        log_debug("start from basic...\n", "");

        // start vice!

        log_debug("started from basic\n", "");

        uint16_t address = 0; //get_basic_start(data);

        parse_mon_file(data, data->config.breakpoint_file);

        if (address != 0) {
            log_debug("start from %x\n", address);
            send_command(data, "g %x\n", address);
        }

        return;
    }

    log_debug("unable to make connection with vice\n", "");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroy_instance(void* user_data) {
    PluginData* plugin = (PluginData*)user_data;

    if (plugin->process.pid > 0) {
        uv_kill(plugin->process.pid, 2);
	}

    if (plugin->conn) {
        VICEConnection_destroy(plugin->conn);
	}

    free(plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void on_menu(PluginData* data, PDReader* reader) {
    uint32_t menuId;

    PDRead_find_u32(reader, &menuId, "menu_id", 0);

    switch (menuId) {
        case C64_VICE_MENU_ATTACH_TO_VICE:
        {
            connect_to_local_host(data);
            break;
        }

        case C64_VICE_MENU_START_WITH_CONFIG:
        {
            launch_vice_with_config(data);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void write_register(PDWriter* writer, const char* name, uint8_t size, uint16_t reg, uint8_t read_only) {
    PDWrite_array_entry_begin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "size", size);

    if (read_only) {
        PDWrite_u8(writer, "read_only", 1);
	}

    if (size == 2) {
        PDWrite_u16(writer, "register", reg);
	} else {
        PDWrite_u8(writer, "register", (uint8_t)reg);
	}

    PDWrite_entry_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void write_status_registers(PDWriter* writer, const char* name, uint16_t reg) {
    PDWrite_array_entry_begin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "read_only", 1);

    char statusString[128] = { 0 };

    sprintf(statusString, "0x$%2x N:%d V:%d -:%d B:%d D:%d I:%d Z:%d C:%d", reg,
            (reg >> 7) & 1, (reg >> 6) & 1, (reg >> 5) & 1, (reg >> 4) & 1,
            (reg >> 3) & 1, (reg >> 2) & 1, (reg >> 1) & 1, (reg >> 0) & 1);

    PDWrite_string(writer, "register_string", statusString);

    PDWrite_entry_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void get_memory(PluginData* data, PDReader* reader, PDWriter* writer) {
    uint64_t address;
    uint64_t size;
    size_t read_size = 0;

    PDRead_find_u64(reader, &address, "address_start", 0);
    PDRead_find_u64(reader, &size, "size", 0);

    // so this is a bit of a hack. If we request memory d000 we switch to io and then back
    // this isn't really correct but will do for now

    if (address == 0xdd00) {
        send_command(data, "bank io\n");
	}

    uint8_t* memory = get_memory_internal(data, data->temp_file_full, &read_size, (uint16_t)(address), (uint16_t)(address + size));

    if (address == 0xdd00) {
        send_command(data, "bank ram\n");
	}

    if (memory) {
        // Lets do this!
        // + 2 is because VICE writes address at the start of the block and at the end
        //

        log_debug("c64_vice: sending memory\n", "");

        PDWrite_event_begin(writer, PDEventType_SetMemory);
        PDWrite_u64(writer, "address", address);
        PDWrite_data(writer, "data", memory + 2, (uint32_t)(read_size - 3));
        PDWrite_event_end(writer);

        // writer takes a copy

        free(memory);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool should_send_command(PluginData* data) {
    bool t0 = data->state != PDDebugState_Running;
    bool t1 = VICEConnection_isConnected(data->conn);
    return t0 && t1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool parse_set_executable(PluginData* plugin, const char* res, int len, PDReader* reader, PDWriter* writer) {
    (void)plugin;
    (void)len;
    (void)reader;
    (void)writer;
    (void)res;
    return strstr(RECV_BUFFER, "Loading") && strstr(RECV_BUFFER, "(C:$");
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

static bool set_executable(PluginData* data, PDReader* reader) {
    const char* filename = 0;

    PDRead_find_string(reader, &filename, "filename", 0);

    if (!filename) {
        log_debug("Unable to find filename %s\n", filename);
        return false;
    }

    log_debug("set_executable %s\n", filename);

    int start_address = parse_prg(filename);

    if (start_address == -1) {
        return false;
	}

    log_debug("loading %s and running from $%x\n", filename, start_address);

    char temp[2048];
    sprintf(temp, "load \"%s\" 0\n", filename);

    if (!send_command_get_data(data, temp, parse_set_executable, reader, 0, 20)) {
        return false;
	}

    send_command(data, "g $%x\n", start_address);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool parse_registers_call(PluginData* plugin, const char* data, int length, PDReader* reader, PDWriter* writer) {
    const char* pch;
    struct Regs6510* regs = &plugin->regs;

    (void)reader;
    (void)writer;

    memcpy(TEMP_BUFFER, data, length);
    TEMP_BUFFER[length] = 0;

    log_debug("parsing registers %s\n", TEMP_BUFFER);

    // Format from VICE looks like this:
    // (C:$e5cf)   ADDR AC XR YR SP 00 01 NV-BDIZC LIN CYC  STOPWATCH
    //           .;e5cf 00 00 0a f3 2f 37 00100010 000 001    3400489

    char* str = strstr(TEMP_BUFFER, ".;");

    if (!str) {
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

    plugin->has_updated_registers = true;
    plugin->has_updated_exception_location = true;

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

static bool get_registers(PluginData* data) {
    return send_command_get_data(data, "registers\n", parse_registers_call, 0, 0, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char* parse_disassembly_line(char* line) {
    char* start = line;

    // Handle the case if we get a line that looks like this (we want to skip everything after -)
    // .C:0811  EE 20 D0    INC $D020      - A:00 X:17 Y:17 SP:f6 ..-.....   19262882

    for (;;) {
        char c = *line++;

        if (c == '\n' || c == 0)
            break;

        if (c == '-') {
            line[-1] = 0;
            break;
        }

    }

    return start;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool parse_disassassembly_call(PluginData* plugin, const char* res, int len, PDReader* reader, PDWriter* writer) {
    memcpy(TEMP_BUFFER, res, len);
    TEMP_BUFFER[len] = 0;

    (void)plugin;
    (void)reader;

    // parse the buffer

    char* pch = strtok(TEMP_BUFFER, "\n");

    PDWrite_event_begin(writer, PDEventType_SetDisassembly);
    PDWrite_array_begin(writer, "disassembly");

    bool hasAllDisasembly = false;

    while (pch) {
        // expected format of each line:
        // xxx.. .C:080e  A9 22       LDA #$22

        char* endOfStream = strstr(pch, "(C:");
        char* line = strstr(pch, ".C");

        if (endOfStream) {
            hasAllDisasembly = true;
            break;
        }

        if (!line)
            break;

        uint16_t address = (uint16_t)strtol(&line[3], 0, 16);

        PDWrite_array_entry_begin(writer);
        PDWrite_u16(writer, "address", address);
        PDWrite_string(writer, "line", parse_disassembly_line(&line[9]));

        PDWrite_entry_end(writer);

        pch = strtok(0, "\n");
    }

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);

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

static bool get_disassembly(PluginData* data, PDReader* reader, PDWriter* writer) {
    char temp[2048];

    uint64_t address_start = 0;
    uint32_t instruction_count = 0;

    PDRead_find_u64(reader, &address_start, "address_start", 0);
    PDRead_find_u32(reader, &instruction_count, "instruction_count", 0);

    // assume that one instruction is 3 bytes which is high but that gives us more data back than we need which is better than too little

    sprintf(temp, "disass $%04x $%04x\n", (uint16_t)address_start, (uint16_t)(address_start + instruction_count * 3));

    return send_command_get_data(data, temp, parse_disassassembly_call, reader, writer, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool parse_breakpoint_call(PluginData* data, const char* res, int len, PDReader* reader, PDWriter* writer) {
    Breakpoint* bp = 0;

    (void)len;
    (void)reader;

    const char* breakStrOffset = strstr(res, "BREAK:");

    if (!breakStrOffset)
        return false;

    int id = atoi(breakStrOffset + 7);

    const char* address = strstr(breakStrOffset, "C:$");

    if (!find_breakpoint_by_id(data, &bp, id)) {
        bp = create_breakpoint();
        add_breakpoint(data, bp);
    }

    bp->id = id;

    if (address)
        bp->address = (uint16_t)strtol(address + 3, 0, 16);

    // add data or update existing

    PDWrite_event_begin(writer, PDEventType_ReplyBreakpoint);
    PDWrite_u64(writer, "address", bp->address);
    PDWrite_u32(writer, "id", (uint32_t)id);
    PDWrite_event_end(writer);

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

static bool set_breakpoint(PluginData* data, PDReader* reader, PDWriter* writer) {
    uint64_t address = 0;
    int32_t id = -1;
    const char* condition = 0;

    PDRead_find_s32(reader, &id, "id", 0);
    PDRead_find_u64(reader, &address, "address", 0);
    PDRead_find_string(reader, &condition, "condition", 0);

    log_debug("got breakpoint %d %llu %s\n", id, address, condition);

    if (id != -1)
        del_breakpoint_by_id(data, id);

    char temp[1024];

    if (condition) {
        sprintf(temp, "break $%04x if %s\n", (uint16_t)address, condition);
	} else {
        sprintf(temp, "break $%04x\n", (uint16_t)address);
	}

    return send_command_get_data(data, temp, parse_breakpoint_call, reader, writer, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool find_parentheses(const char* text, const char** start, const char** end) {
    int i;

    // expected format of each line:
    // (xx) xxxx
    // Notice that first line can have this format:
    // (C:$e5cf) (xx) xxxx

    int len = (int)strlen(text);

    for (i = len; i > -1; --i) {
        if (text[i] == ')')
            *end = &text[i];

        if (text[i] == '(') {
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

static bool parse_for_callstack(PluginData* data, const char* res, int length, PDReader* reader, PDWriter* writer) {
    uint16_t callstack_entries[256];
    int callstack_count = 0;

    (void)data;
    (void)reader;

    memcpy(TEMP_BUFFER, res, length);
    TEMP_BUFFER[length] = 0;

    char* pch = strtok(TEMP_BUFFER, "\n");

    while (pch) {
        const char* startText;
        const char* endText;

        if (!find_parentheses(pch, &startText, &endText))
            break;

        // startText points at (xx)
        // endText points at ) xxxx

        uint16_t offset = (uint16_t)atoi(startText + 1);
        uint16_t address = (uint16_t)strtol(endText + 2, 0, 16);

        callstack_entries[callstack_count++] = address + offset;

        pch = strtok(0, "\n");
    }

    if (callstack_count == 0)
        return false;

    PDWrite_event_begin(writer, PDEventType_SetCallstack);
    PDWrite_array_begin(writer, "callstack");

    for (int i = 0; i < callstack_count; ++i) {
        PDWrite_array_entry_begin(writer);
        PDWrite_u16(writer, "address", callstack_entries[i]);
        PDWrite_entry_end(writer);
    }

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);

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

static bool set_callstack(PluginData* data, PDReader* reader, PDWriter* writer) {
    log_debug("calling set_callstack\n", "");

    return send_command_get_data(data, "bt\n", parse_for_callstack, reader, writer, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void process_events(PluginData* data, PDReader* reader, PDWriter* writer) {
    uint32_t event;

    while ((event = PDRead_get_event(reader))) {
        switch (event) {
            //case PDEventType_getExceptionLocation : setExceptionLocation(plugin, writer); break;
            //case PDEventType_getCallstack : set_callstack(plugin, writer); break;

            case PDEventType_GetRegisters:
            {
                get_registers(data);
                break;
            }

            case PDEventType_GetCallstack:
            {
                if (should_send_command(data))
                    set_callstack(data, reader, writer);

                break;
            }

            case PDEventType_GetDisassembly:
            {
                if (should_send_command(data))
                    get_disassembly(data, reader, writer);

                break;
            }

            case PDEventType_GetMemory:
            {
                if (should_send_command(data))
                    get_memory(data, reader, writer);
                break;
            }

            case PDEventType_MenuEvent:
            {
                on_menu(data, reader);
                break;
            }

            case PDEventType_SetBreakpoint:
            {
                set_breakpoint(data, reader, writer);

                // if we add a breakpoint to VICE it will always stop but if we are already running when
                // adding the breakpoint we just force VICE to run again

                if (data->state == PDDebugState_Running)
                    send_command(data, "ret\n");

                break;
            }

            case PDEventType_DeleteBreakpoint:
            {
                del_breakpoint(data, reader, writer);
                break;
            }

            case PDEventType_SetExecutable:
            {
                //if (should_send_command(data))
                set_executable(data, reader);

                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint16_t find_register_in_string(const char* str, const char* needle) {
    const char* offset = strstr(str, needle);

    size_t neddle_length = strlen(needle);

    if (!offset) {
        log_debug("find_register_in_string: Unable to find %s in %s\n", needle, str);
        return 0;
    }

    offset += neddle_length;

    return (uint16_t)strtol(offset, 0, 16);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint16_t find_status_in_string(const char* str) {
    const char* offset = strstr(str, "SP:");

    if (!offset) {
        return 0;
	}

    // Expected string to look like this;
    // SP:XX ..-...Z. and here we jump so we are at this pos: ..-...Z.

    offset += 6;

    uint8_t flags = 0;

    for (int i = 7; i > 0; i--) {
        const char c = *offset++;

        if (c != '.')
            flags |= 1 << i;
    }

    return flags;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void stop_on_exec(PluginData* plugin, const char* data) {
    const char* stop_on_exec = "Stop on  exec";
    char* found = 0;

    if (!(found = strstr(data, stop_on_exec))) {
        return;
	}

    int execLen = (int)strlen(stop_on_exec);
    found += execLen;

    plugin->regs.pc = (uint16_t)strtol(found, 0, 16);
    plugin->regs.a = (uint8_t)find_register_in_string(found, "A:");
    plugin->regs.x = (uint8_t)find_register_in_string(found, "X:");
    plugin->regs.y = (uint8_t)find_register_in_string(found, "Y:");
    plugin->regs.sp = (uint8_t)find_register_in_string(found, "SP:");
    plugin->regs.flags = (uint8_t)find_status_in_string(found);

    plugin->has_updated_registers = true;
    plugin->has_updated_exception_location = true;
    plugin->state = PDDebugState_StopBreakpoint;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* find_reg_start(const char* res) {
    char c = *res++;

    while (c != 0 && c != '\n') {
        if (c == '-')
            return res;

        c = *res++;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void update_events(PluginData* plugin) {
    char* res = 0;
    int len = 0;

    if (!plugin->conn)
        return;

    // Fetch the data that has been sent from VICE

    if (!get_data(plugin, &res, &len))
        return;

    plugin->state = PDDebugState_StopException;

    // do data parsing here

    stop_on_exec(plugin, res);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool parse_on_step_call(PluginData* plugin, const char* res, int len, PDReader* reader, PDWriter* writer) {
    const char* reg_start;
    const char* step = strstr(res, ".C");

    (void)reader;
    (void)writer;
    (void)len;

    if (!step)
        return false;

    if (!(reg_start = find_reg_start(res)))
        return false;

    // return data from VICE is of the follwing format:
    // .C:0811  EE 20 D0    INC $D020      - A:00 X:17 Y:17 SP:f6 ..-.....   19262882

    plugin->regs.pc = (uint16_t)strtol(&step[3], 0, 16);
    plugin->regs.a = (uint8_t)find_register_in_string(reg_start, "A:");
    plugin->regs.x = (uint8_t)find_register_in_string(reg_start, "X:");
    plugin->regs.y = (uint8_t)find_register_in_string(reg_start, "Y:");
    plugin->regs.sp = (uint8_t)find_register_in_string(reg_start, "SP:");
    plugin->regs.flags = (uint8_t)find_status_in_string(res);

    plugin->has_updated_registers = true;
    plugin->has_updated_exception_location = true;
    plugin->state = PDDebugState_Trace;

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

bool on_step(PluginData* data, PDReader* reader, PDWriter* writer) {
    return send_command_get_data(data, "z\n", parse_on_step_call, reader, writer, 20);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void on_action(PluginData* plugin, PDAction action) {
    switch (action) {
        case PDAction_None:
            break;

        case PDAction_Stop:
        {
            send_command(plugin, "n\n");
            break;
        }

        case PDAction_Break:
        {
            send_command(plugin, "n\n");
            break;
        }

        case PDAction_Run:
        {
            //if (plugin->state != PDDebugState_running)
            {
                send_command(plugin, "ret\n");
                plugin->state = PDDebugState_Running;
            }

            break;
        }

        case PDAction_Step:
        {
            on_step(plugin, 0, 0);
            break;
        }

        case PDAction_StepOver:
        {
            send_command(plugin, "n\n");
            break;
        }

        case PDAction_StepOut:
        case PDAction_Custom:
        {
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data, PDAction action, PDReader* reader, PDWriter* writer) {
    PluginData* plugin = (PluginData*)user_data;

    plugin->has_updated_registers = false;
    plugin->has_updated_exception_location = false;

    on_action(plugin, action);

    process_events(plugin, reader, writer);

    update_events(plugin);

    if (plugin->has_updated_registers) {
        log_debug("sending registens\n", "");

        PDWrite_event_begin(writer, PDEventType_SetRegisters);
        PDWrite_array_begin(writer, "registers");

        write_status_registers(writer, "flags", plugin->regs.flags);
        write_register(writer, "pc", 2, plugin->regs.pc, 1);
        write_register(writer, "sp", 1, plugin->regs.sp, 0);
        write_register(writer, "a", 1, plugin->regs.a, 0);
        write_register(writer, "x", 1, plugin->regs.x, 0);
        write_register(writer, "y", 1, plugin->regs.y, 0);

        PDWrite_array_end(writer);
        PDWrite_event_end(writer);
    }

    if (plugin->has_updated_exception_location) {
        PDWrite_event_begin(writer, PDEventType_SetExceptionLocation);
        PDWrite_u64(writer, "address", plugin->regs.pc);
        PDWrite_u8(writer, "address_size", 2);
        PDWrite_event_end(writer);
    }

    return plugin->state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenuItem MENU_0[] = {
    { "Attach To VICE", C64_VICE_MENU_ATTACH_TO_VICE, 0, 0, 0 },
    { "Start With Config", C64_VICE_MENU_START_WITH_CONFIG, 256 + 3, 0, 0 }, // key hack
    { "Detach From VICE", C64_VICE_MENU_DETACH_FROM_VICE, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenu MENUS[] = {
    { "C64 VICE", (PDMenuItem*)&MENU_0 },
    { 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenu* createMenu() {
    return (PDMenu*)&MENUS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin = {
    "C64 VICE Debugger",
    create_instance,
    destroy_instance,
    createMenu,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, private_data);
    registerPlugin(PD_VIEW_API_VERSION, &g_c64CustomViewPlugin, private_data);
}

