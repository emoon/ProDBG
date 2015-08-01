#include "pd_backend.h"
#include "pd_menu.h"
#include "pd_host.h"
#include "remote_connection.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern PDBackendPlugin g_backendPlugin;

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

#if 0
static void queryRemoteTarget(PluginData* data)
{

}
#endif

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

    conn = RemoteConnection_create(RemoteConnectionType_Connect, 6510);

    if (!RemoteConnection_connect(conn, "localhost", 6510))
    {
        RemoteConnection_destroy(conn);

        data->conn = 0;
        data->state = PDDebugState_noTarget;

        return;
    }

    data->conn = conn;
    data->state = PDDebugState_running;
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

PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
	(void)action;
	(void)writer;

	PluginData* data = (PluginData*)userData;

    uint32_t event;

    while ((event = PDRead_getEvent(reader)))
    {
        switch (event)
        {
            case PDEventType_menuEvent:
            {
                onMenu(data, reader);
                break;
            }
		}
	}

	return PDDebugState_noTarget;
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
