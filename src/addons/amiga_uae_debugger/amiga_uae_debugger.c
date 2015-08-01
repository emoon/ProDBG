#include "pd_backend.h"
#include "pd_menu.h"
#include "pd_host.h"
#include "remote_connection.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#endif

extern PDBackendPlugin g_backendPlugin;

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

unsigned int m68k_read_disassembler_8(unsigned int address)
{
	(void)address;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int m68k_read_disassembler_16(unsigned int address)
{
	(void)address;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned int m68k_read_disassembler_32(unsigned int address)
{
	(void)address;
	return 0;
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

	writeRegister(writer, "sr", 4, get_u32(&tdata), 0);
	writeRegister(writer, "pc", 4, get_u32(&tdata), 0);

	PDWrite_arrayEnd(writer);
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
