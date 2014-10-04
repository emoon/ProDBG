#include "session.h"
#include "core/alloc.h"
#include "core/log.h"
#include "api/plugin_instance.h"
#include "api/src/remote/pd_readwrite_private.h"
#include "api/src/remote/remote_connection.h"
#include <pd_view.h>
#include <pd_backend.h>
#include <stdlib.h>
#include <stb.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    ReadWriteBufferSize = 2 * 1024 * 1024,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum SessionType
{
    Session_Local,
    Session_Remote,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Session
{
    enum SessionType type;
    PDReader reader;
    PDWriter backendWriter;
    PDWriter viewPluginsWriter;
    struct PDBackendInstance* backend;
    struct RemoteConnection* connection;
    struct ViewPluginInstance** viewPlugins;
} Session;

static void updateLocal(Session* s, PDAction action);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void commonInit(Session* s)
{
    PDBinaryWriter_init(&s->backendWriter);
    PDBinaryWriter_init(&s->viewPluginsWriter);
    PDBinaryReader_init(&s->reader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_createRemote(const char* target, int port)
{
    Session* s = alloc_zero(sizeof(Session));

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
    Session* s = alloc_zero(sizeof(Session));

    // setup temporary writer

    PDWriter* writer = alloc_zero(sizeof(PDWriter));
    PDBinaryWriter_init(writer);

    commonInit(s);

    // Create the backend

    s->backend = alloc_zero(sizeof(struct PDBackendInstance));
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
        backend->plugin->update(backend->userData, action, &s->reader, &s->backendWriter);
        PDBinaryWriter_finalize(&s->backendWriter);

        //StatusBar_setText(1, "Status: %s", getStateName(state));
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
        p->plugin->update(p->userData, &p->ui, &s->reader, &s->viewPluginsWriter);
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

void Session_update(Session* s)
{
    if (s->type == Session_Local)
        updateLocal(s, PDAction_none);
    else
        updateRemote(s, PDAction_none);
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

void Session_addViewPlugin(Session* session, struct ViewPluginInstance* plugin)
{
    stb_arr_push(session->viewPlugins, plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_removeViewPlugin(Session* session, struct ViewPluginInstance* plugin)
{
    (void)session;
    (void)plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

