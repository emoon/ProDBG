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

static void* createInstance(ServiceFunc* serviceFunc)
{
	(void)serviceFunc;
	
	PluginData* data = malloc(sizeof(PluginData));

	data->state = PDDebugState_noTarget;

	return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroyInstance(void* userData)
{
	free(userData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void connectToLocalHost(PluginData* data)
{
	// Kill the current connection if we have one

	if (data->conn)
	{
		VICEConnection_destroy(data->conn);
		data->conn = 0;
	}

	struct VICEConnection* conn = VICEConnection_create(VICEConnectionType_Connect, 6510);

	if (!VICEConnection_connect(conn, "localhost", 6510))
	{
		VICEConnection_destroy(conn);

		data->conn = 0;
		data->state = PDDebugState_noTarget; 

		return;
	}

	// VICE state when connecting seems to be break so we set it as such

	data->conn = conn;
	data->state = PDDebugState_stopBreakpoint;
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

static void setRegisters(PluginData* data, PDWriter* writer)
{
	char* res = 0; 
	int len = 0;

	(void)writer;

	sendCommand(data, "registers");

	// Try to get some data back

	if (!getData(data, &res, &len))
		return;

	// TODO: Parse here

	printf("setRegisters: data back %s\n", res);
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
            case PDEventType_setRegisters : setRegisters(data, writer); break;
			case PDEventType_menuEvent : onMenu(data, reader);
        }
    }

    //setTty(plugin, writer);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
	(void)action;

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


