#include "pd_backend.h"
#include "pd_menu.h"
#include "pd_host.h"
#include "remote_connection.h"
#include "m68k.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#endif

extern PDBackendPlugin g_backendPlugin;
static const char s_hexchars [] = "0123456789abcdef";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static void sleepMs(int ms)
{
#ifdef _MSC_VER
    Sleep(ms);
#else
    usleep((unsigned int)(ms * 1000));
#endif
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    AMIGA_UAE_MENU_ATTACH_TO_UAE,
    AMIGA_UAE_MENU_START_WITH_CONFIG,
    AMIGA_UAE_MENU_DETACH_FROM_UAE,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PluginData
{
    struct RemoteConnection* conn;
	int dummy;
	PDDebugState state;
	uint32_t exceptionLocation;
} PluginData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc)
{
	(void)serviceFunc;

	PluginData* t = (PluginData*)malloc(sizeof(PluginData));
	memset(t, 0, sizeof(PluginData));

	return t;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool queryRemoteTarget(PluginData* data)
{
	uint8_t reply[1024];

	int ret = RemoteConnection_sendFormatRecv(reply, sizeof(reply), data->conn, 100, "$QStartNoAckMode#b0");

	if (ret == 0)
	{
		printf("Failed to get response from target\n");
		return false;
	}

	if (!strcmp((char*)reply, "$OK#9a"))
	{
		printf("Failed to get proper response from target, expectd: $OK#9a got: %s\n", reply);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void connectToLocalHost(PluginData* data)
{
    struct RemoteConnection* conn = 0;

    // Kill the current connection if we have one

    if (data->conn)
    {
        RemoteConnection_destroy(data->conn);
        data->conn = 0;
    }

    conn = RemoteConnection_create(RemoteConnectionType_Connect, 6860);

    if (!RemoteConnection_connect(conn, "localhost", 6860))
    {
        RemoteConnection_destroy(conn);

        data->conn = 0;
        data->state = PDDebugState_noTarget;

        return;
    }

    data->conn = conn;

	if (!queryRemoteTarget(data))
	{
        RemoteConnection_destroy(data->conn);
        data->conn = 0;
        return;
	}

	printf("Proper reply from target\n");

    data->state = PDDebugState_running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t s_disassemblyBuffer[1024];
static uint32_t s_baseAddress = 0;
static int s_disBufferLength = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int m68k_read_disassembler_8(unsigned int address)
{
	address -= s_baseAddress;

	if ((int)address > s_disBufferLength)
	{
		printf("trying to read outside disbuffer\n");
		return 0;
	}

	return s_disassemblyBuffer[address];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int m68k_read_disassembler_16(unsigned int address)
{
	address -= s_baseAddress;

	if (((int)address + 1) > s_disBufferLength)
	{
		printf("trying to read outside disbuffer\n");
		return 0;
	}

	uint16_t v0 = s_disassemblyBuffer[address + 0];
	uint16_t v1 = s_disassemblyBuffer[address + 1];

	return (v0 << 8) | v1; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int m68k_read_disassembler_32(unsigned int address)
{
	address -= s_baseAddress;

	if (((int)address + 3) > s_disBufferLength)
	{
		printf("trying to read outside disbuffer\n");
		return 0;
	}

	uint16_t v0 = s_disassemblyBuffer[address + 0];
	uint16_t v1 = s_disassemblyBuffer[address + 1];
	uint16_t v2 = s_disassemblyBuffer[address + 2];
	uint16_t v3 = s_disassemblyBuffer[address + 3];

	return (v0 << 24) | (v1 << 16) | (v2 << 8) | v3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroyInstance(void* userData)
{
	free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onMenu(PluginData* data, PDReader* reader)
{
    uint32_t menuId;

    (void)data;

    PDRead_findU32(reader, &menuId, "menu_id", 0);

    switch (menuId)
    {
        case AMIGA_UAE_MENU_ATTACH_TO_UAE:
        {
        	printf("want to connect\n");
            connectToLocalHost(data);
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool isValidPacket(const uint8_t* buffer, int length)
{

	if (buffer[0] != '$')
		return false;

	for (int i = 0; i < length; ++i)
	{
		if (buffer[i] == '#')
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int hex(char ch)
{
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;

	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';

	if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t get_u32 (const uint8_t** data)
{
	const uint8_t* temp = *data;

	uint32_t t[4];

	for (int i = 0; i < 4; ++i) {
		t[i] = (uint32_t)hex((char)temp[0]) << 4 | (uint32_t)hex((char)temp[1]);
		temp += 2;
	}

	*data = temp;

	return (t[0] << 24) | (t[1] << 16) | (t[2] << 8) | t[3];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void writeRegister(PDWriter* writer, const char* name, uint8_t size, uint32_t reg, uint8_t readOnly)
{
    PDWrite_arrayEntryBegin(writer);
    PDWrite_string(writer, "name", name);
    PDWrite_u8(writer, "size", size);

    if (readOnly)
        PDWrite_u8(writer, "read_only", 1);

	PDWrite_u32(writer, "register", reg);

    PDWrite_arrayEntryEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void getRegisters(PluginData* data, PDWriter* writer)
{
	char regName[4] = { 0 };
	uint8_t reply[1024];
	const uint8_t* tdata = reply;

	printf("get registes\n");

	int length = RemoteConnection_sendFormatRecv(reply, sizeof(reply), data->conn, 100, "$g#67");

	if (!isValidPacket(reply, length))
	{
		printf("Packet %s isn't valid\n", (char*)reply);
		return;
	}

	tdata++; // skip '$'

	PDWrite_eventBegin(writer, PDEventType_setRegisters);
	PDWrite_arrayBegin(writer, "registers");

	// d registers

	regName[0] = 'd';

	for (int i = 0; i < 8; ++i)
	{
		regName[1] = '0' + (char)i;
		writeRegister(writer, regName, 4, get_u32(&tdata), 0);
	}

	// a registers
	
	regName[0] = 'a';

	for (int i = 0; i < 8; ++i)
	{
		regName[1] = '0' + (char)i;
		writeRegister(writer, regName, 4, get_u32(&tdata), 0);
	}

	uint32_t sr = get_u32(&tdata);
	uint32_t pc = get_u32(&tdata);

	data->exceptionLocation = pc;

	writeRegister(writer, "sr", 4, sr, 0);
	writeRegister(writer, "pc", 4, pc, 0);

	PDWrite_arrayEnd(writer);
	PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char checksumString(char* string, bool append)
{
	uint8_t cs = 0;
	size_t len = strlen(string);

	for (size_t i = 0; i < len; ++i)
		cs += (uint8_t)string[i];

	if (!append)
		return cs;

	string[len + 0] = '#';
	string[len + 1] = s_hexchars[cs >> 4]; 
	string[len + 2] = s_hexchars[cs & 0xf]; 
	string[len + 3] = 0; 

	return cs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void getDisassembly(PluginData* data, PDReader* reader, PDWriter* writer)
{
	uint8_t reply[1024];
	char cmdBuffer[512];
	int disLength = 0;

    uint64_t addressStart = 0;
    uint32_t instructionCount = 0;

    (void)data;
    (void)writer;

    PDRead_findU64(reader, &addressStart, "address_start", 0);
    PDRead_findU32(reader, &instructionCount, "instruction_count", 0);

	s_baseAddress = (uint32_t)addressStart;

	// TODO: We need to clean this up

	cmdBuffer[0] = '$';
	cmdBuffer[1] = 'm';
	sprintf(&cmdBuffer[2], "%x,%x", (uint32_t)addressStart, (uint32_t)instructionCount * 4);
	checksumString(&cmdBuffer[1], true);

	int length = RemoteConnection_sendFormatRecv(reply, sizeof(reply), data->conn, 100, cmdBuffer);

	if (length == 0)
		return;

	for (int i = 1; i < length; i += 2, ++disLength)
	{
		if (reply[i] == '#')
			break;

		s_disassemblyBuffer[disLength] = (uint8_t)(hex((char)reply[i + 0]) << 4) | 
										 (uint8_t)hex((char)reply[i + 1]);
	}

	s_disBufferLength = disLength;

	disLength = 0;

    PDWrite_eventBegin(writer, PDEventType_setDisassembly);
    PDWrite_arrayBegin(writer, "disassembly");

	while (disLength < s_disBufferLength - 3)
	{
		char tempBuffer[1024];
		int t = m68k_disassemble(tempBuffer, (uint32_t)addressStart + disLength, M68K_CPU_TYPE_68000);

        PDWrite_arrayEntryBegin(writer);
        PDWrite_u32(writer, "address", (uint32_t)addressStart + disLength);
        PDWrite_string(writer, "line", tempBuffer); 
        PDWrite_arrayEntryEnd(writer);

        disLength += t;
	}

    PDWrite_arrayEnd(writer);
    PDWrite_eventEnd(writer);

	printf("end dis........\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExceptionLocation(PluginData* data, PDWriter* writer)
{
	PDWrite_eventBegin(writer, PDEventType_setExceptionLocation);
	PDWrite_u64(writer, "address", data->exceptionLocation);
	PDWrite_u8(writer, "address_size", 4);
	PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStep(PluginData* data, PDWriter* writer)
{
	uint8_t reply[1024];

	int length = RemoteConnection_sendFormatRecv(reply, sizeof(reply), data->conn, 100, "$s#73");

	if (length == 0)
		return;

	// send the registers once we stepd

	getRegisters(data, writer);

	setExceptionLocation(data, writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onAction(PluginData* plugin, PDAction action, PDWriter* writer)
{
	switch (action)
	{
		case PDAction_step:
		{
			onStep(plugin, writer);
			break;
		}

		case PDAction_none:
		case PDAction_stop:
		case PDAction_break:
		case PDAction_run:
		case PDAction_stepOut:
		case PDAction_stepOver:
		case PDAction_custom:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
	(void)action;
	(void)writer;

	PluginData* data = (PluginData*)userData;

    onAction(data, action, writer);

    uint32_t event;

    while ((event = PDRead_getEvent(reader)))
    {
        switch (event)
        {
			case PDEventType_getRegisters:
			{
				getRegisters(data, writer);
				break;
			}

			case PDEventType_getDisassembly:
			{
				getDisassembly(data, reader, writer);
				break;
			}

            case PDEventType_menuEvent:
            {
                onMenu(data, reader);
                break;
            }
		}
	}

	return data->state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenuItem s_menu0[] =
{
    { "Attach To UAE", AMIGA_UAE_MENU_ATTACH_TO_UAE, 0, 0, 0 },
    { "Start With Config", AMIGA_UAE_MENU_START_WITH_CONFIG, 256 + 3, 0, 0 }, // key hack
    { "Detach From UAE", AMIGA_UAE_MENU_DETACH_FROM_UAE, 0, 0, 0 },
    { 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenu s_menus[] =
{
    { "Amiga UAE", (PDMenuItem*)&s_menu0 },
    { 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDMenu* createMenu()
{
    return (PDMenu*)&s_menus;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin g_backendPlugin =
{
    "Amiga UAE Debugger",
    createInstance,
    destroyInstance,
    createMenu,
    update,
};
