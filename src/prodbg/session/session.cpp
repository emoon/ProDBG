#include "session.h"
#include "session_private.h"
#include "core/alloc.h"
#include "api/plugin_instance.h"
#include "api/src/remote/pd_readwrite_private.h"
#include "api/src/remote/remote_connection.h"
#include "core/log.h"
#include "core/plugin_handler.h"
#include "ui/plugin.h"
#include "ui/ui_layout.h"
#include "core/math.h"
#include <stdlib.h>
#include <stb.h>
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    ReadWriteBufferSize = 2 * 1024 * 1024,
};

static void updateLocal(Session* s, PDAction action);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void commonInit(Session* s)
{
    PDBinaryWriter_init(&s->backendWriter);
    PDBinaryWriter_init(&s->viewPluginsWriter);
    PDBinaryReader_init(&s->reader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* Session_create()
{
    Session* s = (Session*)alloc_zero(sizeof(Session));

    commonInit(s);

    return s;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_createRemote(const char* target, int port)
{
    Session* s = (Session*)alloc_zero(sizeof(Session));

    commonInit(s);

    s->type = Session_Remote;

    struct RemoteConnection* conn = RemoteConnection_create(RemoteConnectionType_Connect, port);

    if (!RemoteConnection_connect(conn, target, port))
    {
        //StatusBar_setText(0, "Unable to connect to %s:%d", target, port);
        RemoteConnection_destroy(conn);
        return s;
    }
    else
    {
        s->connection = conn;
    }

    //StatusBar_setText(0, "Connect to %s:%d", target, port);

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_createLocal(PDBackendPlugin* backend, const char* filename)
{
    Session* s = (Session*)alloc_zero(sizeof(Session));

    // setup temporary writer

    PDWriter* writer = (PDWriter*)alloc_zero(sizeof(PDWriter));
    PDBinaryWriter_init(writer);

    commonInit(s);

    // Create the backend

    s->backend = (PDBackendInstance*)alloc_zero(sizeof(struct PDBackendInstance));
    s->backend->plugin = backend;
    s->backend->userData = backend->createInstance(0);

    // TODO: So having a filename for a local session isn't really what is needed but this will get us
    // going with the local sessions which is the target here


    // TODO: This is a good point to add existing breakpoints
    //
    //

    PDBinaryWriter_reset(writer);
    PDWrite_eventBegin(writer, PDEventType_setExecutable);
    PDWrite_string(writer, "filename", filename);
    PDWrite_eventEnd(writer);
    PDBinaryWriter_finalize(writer);

    // Init the stream for reading and write back the data
    //
    log_info("init\n");

    PDBinaryReader_initStream(&s->reader, PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer));
    s->backend->plugin->update(s->backend->userData, PDAction_none, &s->reader, &s->backendWriter);

    //log_info("first update\n");

    // TODO: How to deal if plugin writes back data here?
    // TODO: Not run directly but allow user to select if run, otherwise (ProDG style stop-at-main?)

    updateLocal(s, PDAction_run);

    //log_info("second update\n");

    return s;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_destroy(Session* session)
{
    free(session);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* getStateName(int state)
{
    if (state < PDDebugState_count && state >= 0)
    {
        switch (state)
        {
            case PDDebugState_noTarget:
                return "No target";
            case PDDebugState_running:
                return "Running";
            case PDDebugState_stopBreakpoint:
                return "Stop (breakpoint)";
            case PDDebugState_stopException:
                return "Stop (exception)";
            case PDDebugState_trace:
                return "Trace (stepping)";
        }
    }

    return "Unknown";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateLocal(Session* s, PDAction action)
{
    struct PDBackendInstance* backend = s->backend;

    PDBinaryReader_reset(&s->reader);

    PDBinaryReader_initStream(
        &s->reader,
        PDBinaryWriter_getData(&s->viewPluginsWriter),
        PDBinaryWriter_getSize(&s->viewPluginsWriter));

    if (backend)
    {
        PDBinaryWriter_reset(&s->backendWriter);
        s->state = backend->plugin->update(backend->userData, action, &s->reader, &s->backendWriter);
        PDBinaryWriter_finalize(&s->backendWriter);
    }

    PDBinaryReader_initStream(
        &s->reader,
        PDBinaryWriter_getData(&s->backendWriter),
        PDBinaryWriter_getSize(&s->backendWriter));

    PDBinaryWriter_reset(&s->viewPluginsWriter);

    int len = stb_arr_len(s->viewPlugins);

    for (int i = 0; i < len; ++i)
    {
        struct ViewPluginInstance* p = s->viewPlugins[i];
        PluginUIState state = PluginUI_updateInstance(p, &s->reader, &s->viewPluginsWriter);

        if (state == PluginUIState_CloseView)
            p->markDeleted = true;

        PDBinaryReader_reset(&s->reader);
    }

    PDBinaryWriter_finalize(&s->viewPluginsWriter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* getBackendState(PDReader* reader)
{
    uint32_t event;
    uint32_t state;
    const char* retState = "Unknown";

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_setStatus:
            {
                PDRead_findU32(reader, &state, "state", 0);
                retState = getStateName((int)state);
                goto end;
            }
        }
    }

    end:;

    PDBinaryReader_reset(reader);

    return retState;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateRemote(Session* s, PDAction action)
{
    //PDBackendInstance* backend = s->backend;

    (void)action;

    if (!s->connection)
        return;

    PDBinaryReader_reset(&s->reader);

    if (RemoteConnection_pollRead(s->connection))
    {
        int totalSize = 0;
        uint8_t cmd[4];
        uint8_t* outputBuffer;

        // TODO: Make this a bit less hardcoded (cmd decode)

        if (RemoteConnection_recv(s->connection, (char*)&cmd, 4, 0))
        {
            totalSize = (cmd[0] << 24) | (cmd[1] << 16) | (cmd[2] << 8) | cmd[3];

            outputBuffer = RemoteConnection_recvStream(s->connection, 0, totalSize);

            PDBinaryReader_initStream(&s->reader, outputBuffer, (unsigned int)totalSize);
        }
    }

    // Get the current state of the plugin

    //const char* backendState = getBackendState(&s->reader);
    //(void)backendState;
    //StatusBar_setText(1, "Status: %s", backendState);

    PDBinaryWriter_reset(&s->viewPluginsWriter);

    int len = stb_arr_len(s->viewPlugins);

    for (int i = 0; i < len; ++i)
    {
        struct ViewPluginInstance* p = s->viewPlugins[i];
        p->plugin->update(p->userData, &p->ui, &s->reader, &s->viewPluginsWriter);
        PDBinaryReader_reset(&s->reader);
    }

    PDBinaryWriter_finalize(&s->viewPluginsWriter);

    if (PDBinaryWriter_getSize(&s->viewPluginsWriter) > 4)
    {
        if (PDBinaryWriter_getSize(&s->viewPluginsWriter) > 4 &&
            RemoteConnection_isConnected(s->connection))
        {
            RemoteConnection_sendStream(s->connection, PDBinaryWriter_getData(&s->viewPluginsWriter));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Remove vewies that has been marked deleted

static void updateMarkedDelete(Session* s)
{
    int count = stb_arr_len(s->viewPlugins);

    for (int i = 0; i < count; ++i)
    {
        struct ViewPluginInstance* p = s->viewPlugins[i];

        if (p->markDeleted)
        {
            Session_removeViewPlugin(s, p);
            count = stb_arr_len(s->viewPlugins);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_update(Session* s)
{
    switch (s->type)
    {
        case Session_Null:
        case Session_Local:
        {
            updateLocal(s, PDAction_none);
            break;
        }

        case Session_Remote:
        {
            updateRemote(s, PDAction_none);
            break;
        }
    }

    updateMarkedDelete(s);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_action(Session* s, PDAction action)
{
    if (!s)
        return;

    if (s->type == Session_Local)
    {
        updateLocal(s, action);
    }
    else
    {
        if (RemoteConnection_isConnected(s->connection))
        {
            uint8_t command[4];
            command[0] = 1 << 7; // action tag
            command[1] = 0;
            command[2] = (action >> 8) & 0xff;
            command[3] = (action >> 0) & 0xff;
            RemoteConnection_send(s->connection, &command, sizeof(uint32_t), 0);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_addViewPlugin(struct Session* session, struct ViewPluginInstance* instance)
{
    stb_arr_push(session->viewPlugins, instance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Session_removeViewPlugin(Session* session, struct ViewPluginInstance* plugin)
{
    int count = stb_arr_len(session->viewPlugins);

    if (count == 0)
        return true;

    if (count == 1)
    {
        stb_arr_pop(session->viewPlugins);
        return true;
    }

    for (int i = 0; i < count; ++i)
    {
        if (session->viewPlugins[i] == plugin)
        {
            stb_arr_fastdelete(session->viewPlugins, i);
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ViewPluginInstance** Session_getViewPlugins(struct Session* session, int* count)
{
    *count = stb_arr_len(session->viewPlugins);
    return session->viewPlugins;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_getLayout(Session* session, UILayout* layout, float width, float height)
{
	assert(session);
	assert(layout);

    int count = stb_arr_len(session->viewPlugins);

	memset(layout, 0, sizeof(UILayout));

	// No base paths (will use the default which depends on the build configuration when
	// trying to load the plugins

	layout->basePathCount = 0;
	layout->layoutItemCount = count;
	
	layout->layoutItems = (LayoutItem*)alloc_zero((int)sizeof(LayoutItem) * (int)count);

	for (int i = 0; i < count; ++i)
	{
		ViewPluginInstance* instance = session->viewPlugins[i];
		PDViewPlugin* plugin = (PDViewPlugin*)instance->plugin;
		LayoutItem* item = &layout->layoutItems[i];
		PluginData* pluginData = PluginHandler_getPluginData(instance->plugin);

		if (!pluginData)
			continue;

		item->pluginFile = strdup(pluginData->filename);
		item->pluginName = strdup(plugin->name);

		PluginUI_getWindowRect(instance, &item->rect);

		item->rect.x /= width;
		item->rect.y /= height;
		item->rect.width /= width;
		item->rect.height /= height;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_setLayout(Session* session, UILayout* layout, float width, float height)
{
	int count = layout->layoutItemCount;

	// TODO: Close all existing windows when loading layout?
	// TODO: Support base paths for plugins

	for (int i = 0; i < count; ++i)
	{
		LayoutItem* item = &layout->layoutItems[i];

		PluginData* pluginData = PluginHandler_findPlugin(0, item->pluginFile, item->pluginName, true);

		if (!pluginData)
		{
			log_error("Unable to find plugin %s %s\n", item->pluginFile, item->pluginName);
			continue;
		}

		FloatRect rect = item->rect;

		rect.x *= width;
		rect.y *= height;
		rect.width *= width;
		rect.height *= height;

		ViewPluginInstance* instance = PluginInstance_createViewPlugin(pluginData);

		if (!instance)
		{
			log_error("Unable to create instance for plugin %s %s\n", item->pluginFile, item->pluginName);
			continue;
		}

		PluginUI_setWindowRect(instance, &rect);

		Session_addViewPlugin(session, instance);
	}
}


