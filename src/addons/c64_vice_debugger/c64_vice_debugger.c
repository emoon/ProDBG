#include "pd_backend.h"
#include "pd_menu.h"
#include "c64_vice_connection.h"
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char s_recvBuffer[512 * 1024]; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum 
{
	C64_VICE_MENU_ATTACH_TO_VICE,
	C64_VICE_MENU_DETACH_FROM_VICE,
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sleepMs(int ms)
{
#ifdef _MSC_VER
    Sleep(ms);
#else
    usleep((unsigned int)(ms * 1000));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PluginData
{
	struct VICEConnection* conn;
	PDDebugState state;
} PluginData;

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

	data->state = PDDebugState_noTarget;

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

static void sendCommand(PluginData* data, const char* command)
{
	int len = (int)strlen(command);
	printf("sending command: %s\n", command);
	VICEConnection_send(data->conn, command, len, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int getData(PluginData* data, char** resBuffer, int* len)
{
	const int maxTry = 100;
	int res = 0;

	for (int i = 0; i < maxTry; ++i)
	{
		// TODO: Do we need to look for more data here (after the first pool)

		if (VICEConnection_pollRead(data->conn))
		{
			res = VICEConnection_recv(data->conn, s_recvBuffer, sizeof(s_recvBuffer), 0);

			if (res == 0)
				return 0;

			*len = res;
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

struct Regs6510
{
	uint16_t pc; 
	uint16_t a; 
	uint16_t x; 
	uint16_t y; 
	uint16_t sp; 
};

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
	// Format from VICE looks like this:
	// (C:$e5cf)   ADDR AC XR YR SP 00 01 NV-BDIZC LIN CYC  STOPWATCH
	//           .;e5cf 00 00 0a f3 2f 37 00100010 000 001    3400489
	//
	
  	const char* pch = strtok(str," \t\n"); 

  	// Skip the layout (hard-coded and ugly but as registers are fixed this should be fine
  	// but needs to be fixed if VICE changes the layout

  	for (int i = 0; i < 12; ++i)
		pch = strtok(0, " \t\n");

	regs->pc = (uint16_t)strtol(&pch[2], 0, 16); pch = strtok(0, " \t");
	regs->a = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
	regs->x = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
	regs->y = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
	regs->sp = (uint8_t)strtol(pch, 0, 16); pch = strtok(0, " \t");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setRegisters(PluginData* data, PDWriter* writer)
{
	struct Regs6510 regs;
	char* res = 0; 
	int len = 0;

	(void)writer;

	sendCommand(data, "registers\n");

	printf("waiting to get data back\n");

	// Try to get some data back

	if (!getData(data, &res, &len))
	{
		printf("No data back!\n");
		return;
	}

	// Always stop on command

	data->state = PDDebugState_stopException;

	parseRegisters(&regs, res);

    PDWrite_eventBegin(writer, PDEventType_setRegisters);
    PDWrite_arrayBegin(writer, "registers");

    writeRegister(writer, "pc", 2, regs.pc, 1);
    writeRegister(writer, "sp", 1, regs.sp, 0);
    writeRegister(writer, "a", 1, regs.a, 0);
    writeRegister(writer, "x", 1, regs.x, 0);
    writeRegister(writer, "y", 1, regs.y, 0);
    //writeRegister(writer, "status", 1, status, 1);

    PDWrite_arrayEnd(writer);
    PDWrite_eventEnd(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void processEvents(PluginData* data, PDReader* reader, PDWriter* writer)
{
    uint32_t event;

    while ((event = PDRead_getEvent(reader)))
    {
        printf("C64VICE: event %d\n", event);

        switch (event)
        {
            //case PDEventType_getExceptionLocation : setExceptionLocation(plugin, writer); break;
            //case PDEventType_getCallstack : setCallstack(plugin, writer); break;
            case PDEventType_getRegisters : setRegisters(data, writer); break;
			case PDEventType_menuEvent : onMenu(data, reader);
        }
    }

    //setTty(plugin, writer);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
	(void)action;

	printf("c64_vice_update\n");

    PluginData* plugin = (PluginData*)userData;

	processEvents(plugin, reader, writer);

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


