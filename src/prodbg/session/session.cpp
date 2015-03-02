#include "session.h"
#include "session_private.h"
#include "api/plugin_instance.h"
#include "api/include/pd_script.h"
#include "api/src/remote/pd_readwrite_private.h"
#include "api/src/remote/remote_connection.h"
#include "core/alloc.h"
#include "core/log.h"
#include "core/math.h"
#include "core/plugin_handler.h"
#include "ui/plugin.h"
#include "ui/ui_statusbar.h"
#include "ui/ui_dock_private.h" // TODO: Fix me

#include <stdlib.h>
#include <stb.h>
#include <assert.h>

#include <pd_view.h>
#include <pd_backend.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    ReadWriteBufferSize = 2 * 1024 * 1024,
};

static void updateLocal(Session* s, PDAction action);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void commonInit(Session* s)
{
    s->writer0 = (PDWriter*)alloc_zero(sizeof(PDWriter));
    s->writer1 = (PDWriter*)alloc_zero(sizeof(PDWriter));
    s->tempWriter0 = (PDWriter*)alloc_zero(sizeof(PDWriter));
    s->tempWriter1 = (PDWriter*)alloc_zero(sizeof(PDWriter));
    s->reader = (PDReader*)alloc_zero(sizeof(PDReader));

    PDBinaryWriter_init(s->writer0);
    PDBinaryWriter_init(s->writer1);
    PDBinaryWriter_init(s->tempWriter0);
    PDBinaryWriter_init(s->tempWriter1);
    PDBinaryReader_init(s->reader);

    s->currentWriter = s->writer0;
    s->prevWriter = s->writer1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session* Session_create()
{
    Session* s = new Session;

    UIStatusBar_setText("Not running");

    commonInit(s);

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PRODBG_USING_DOCKING

void Session_createDockingGrid(Session* session, int width, int height)
{
    FloatRect rect = {{{ 0.0f, 0.0f, (float)width, (float)height }}};

    session->uiDockingGrid = UIDock_createGrid(&rect);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Session_loadLayout(Session* session, const char* filename, int width, int height)
{
    UIDockingGrid* grid = UIDock_loadLayout(filename, (float)width, (float)height);

    if (!grid)
        return false;

    session->uiDockingGrid = grid;

    // TODO: Fix me

    for (UIDock* dock : grid->docks)
        Session_addViewPlugin(session, dock->view);

    return true;
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_startRemote(Session* s, const char* target, int port)
{
    s->type = Session_Remote;

    struct RemoteConnection* conn = RemoteConnection_create(RemoteConnectionType_Connect, port);

    if (!RemoteConnection_connect(conn, target, port))
    {
        log_info("Unable to connect to %s:%d", target, port);
        RemoteConnection_destroy(conn);
        return s;
    }
    else
    {
        s->connection = conn;
    }

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Session_isConnected(Session* session)
{
    if (!session->connection)
        return 0;

    return RemoteConnection_isConnected(session->connection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_createRemote(const char* target, int port)
{
    Session* s = new Session;

    commonInit(s);

    Session_startRemote(s, target, port);

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_startLocal(Session* s, PDBackendPlugin* backend, const char* filename)
{
    // Create the backend

    s->type = Session_Local;
    s->backend = (PDBackendInstance*)alloc_zero(sizeof(struct PDBackendInstance));
    s->backend->plugin = backend;
    s->backend->userData = backend->createInstance(0);

    // Set the executable if we have any

    if (filename)
    {
        PDWrite_eventBegin(s->currentWriter, PDEventType_setExecutable);
        PDWrite_string(s->currentWriter, "filename", filename);
        PDWrite_eventEnd(s->currentWriter);

        // Add existing breakpoints

        for (auto i = s->breakpoints.begin(), end = s->breakpoints.end(); i != end; ++i)
        {
            PDWrite_eventBegin(s->currentWriter, PDEventType_setBreakpoint);
            PDWrite_string(s->currentWriter, "filename", (*i)->filename);
            PDWrite_u32(s->currentWriter, "line", (unsigned int)(*i)->line);
            PDWrite_eventEnd(s->currentWriter);
        }
    }

    // TODO: Not run directly but allow user to select if run, otherwise (ProDG style stop-at-main?)

    updateLocal(s, PDAction_run);

    //log_info("second update\n");

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_createLocal(PDBackendPlugin* backend, const char* filename)
{
    Session* s = new Session;

    commonInit(s);

    return Session_startLocal(s, backend, filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_destroy(Session* session)
{
    if (session->backend)
        session->backend->plugin->destroyInstance(session->backend->userData);

    session->backend = 0;

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

static void doToggleBreakpoint(Session* s, PDReader* reader)
{
    const char* filename;
    uint32_t line;

    PDRead_findString(reader, &filename, "filename", 0);
    PDRead_findU32(reader, &line, "line", 0);

    for (auto i = s->breakpoints.begin(), end = s->breakpoints.end(); i != end; ++i)
    {
        if ((*i)->line == (int)line && !strcmp((*i)->filename, filename))
        {
            free((void*)(*i)->filename);
            s->breakpoints.erase(i);
            return;
        }
    }

    Breakpoint* breakpoint = (Breakpoint*)malloc(sizeof(Breakpoint));
    breakpoint->filename = strdup(filename);
    breakpoint->line = (int)line;

    s->breakpoints.push_back(breakpoint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void toggleBreakpoint(Session* s, PDReader* reader)
{
    uint32_t event;

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_setBreakpoint:
            {
                doToggleBreakpoint(s, reader);
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void executeCommand(Session* s, PDReader* reader)
{
    (void)s;

    const char* command;
    PDRead_findString(reader, &command, "command", 0);
    PDScriptState* scriptState;
    PDScript_createState(&scriptState);
    if (PDScript_loadString(scriptState, command))
        goto cleanup;

    /*PDScriptCallState callState;
       callState.inputCount = 0;
       callState.outputCount = 0;

       if (PDScript_primeCall(scriptState, &callState))
        goto cleanup;

       int result = PDScript_executeCall(scriptState, &callState);
       (void)result;*/

    cleanup:
    //free(callState.funcName);
    PDScript_destroyState(&scriptState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateScript(Session* s, PDReader* reader)
{
    uint32_t event;

    while ((event = PDRead_getEvent(reader)) != 0)
    {
        switch (event)
        {
            case PDEventType_executeConsole:
            {
                executeCommand(s, reader);
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateLocal(Session* s, PDAction action)
{
    PDBinaryWriter_finalize(s->currentWriter);

    // Swap the write buffers

    PDWriter* temp = s->currentWriter;
    s->currentWriter = s->prevWriter;
    s->prevWriter = temp;

    unsigned int reqDataSize = PDBinaryWriter_getSize(s->prevWriter);
    PDBackendInstance* backend = s->backend;

    PDBinaryReader_reset(s->reader);

    // TODO: Temporary hack, send no request data to backend if we are running.

    //if (s->state == PDDebugState_running)
    //   reqDataSize = 0;

    PDBinaryReader_initStream(s->reader, PDBinaryWriter_getData(s->prevWriter), reqDataSize);
    PDBinaryWriter_reset(s->currentWriter);

    if (backend)
        s->state = backend->plugin->update(backend->userData, action, s->reader, s->currentWriter);

    int len = stb_arr_len(s->viewPlugins);

    PDBinaryReader_initStream(s->reader, PDBinaryWriter_getData(s->prevWriter), PDBinaryWriter_getSize(s->prevWriter));
    PDBinaryReader_reset(s->reader);

    for (int i = 0; i < len; ++i)
    {
        struct ViewPluginInstance* p = s->viewPlugins[i];
        PluginUIState state = PluginUI_updateInstance(p, s->reader, s->currentWriter);

        if (state == PluginUIState_CloseView)
        {
        #if PRODBG_USING_DOCKING
            UIDock_deleteView(s->uiDockingGrid, p);
        #endif
            p->markDeleted = true;
        }

        PDBinaryReader_reset(s->reader);
    }

    updateScript(s, s->reader);

    toggleBreakpoint(s, s->reader);
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
    if (!s->connection)
        return;

    (void)action;

    PDBinaryReader_reset(s->reader);

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

            PDBinaryReader_initStream(s->reader, outputBuffer, (unsigned int)totalSize);
        }
    }

    int len = stb_arr_len(s->viewPlugins);

    for (int i = 0; i < len; ++i)
    {
        struct ViewPluginInstance* p = s->viewPlugins[i];
        PluginUIState state = PluginUI_updateInstance(p, s->reader, s->currentWriter);

        if (state == PluginUIState_CloseView)
        {
        #if PRODBG_USING_DOCKING
            UIDock_deleteView(s->uiDockingGrid, p);
        #endif
            p->markDeleted = true;
        }

        PDBinaryReader_reset(s->reader);
    }

    PDBinaryWriter_finalize(s->currentWriter);

    // Swap the write buffers

    PDWriter* temp = s->currentWriter;
    s->currentWriter = s->prevWriter;
    s->prevWriter = temp;

    if (PDBinaryWriter_getSize(s->prevWriter) > 4)
    {
        if (PDBinaryWriter_getSize(s->prevWriter) > 4 && RemoteConnection_isConnected(s->connection))
        {
            RemoteConnection_sendStream(s->connection, PDBinaryWriter_getData(s->prevWriter));
        }
    }

    PDBinaryWriter_reset(s->currentWriter);
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
// TODO: Somewhat temporay functions

void Session_loadSourceFile(Session* s, const char* filename)
{
    PDWrite_eventBegin(s->currentWriter, PDEventType_setExceptionLocation);
    PDWrite_string(s->currentWriter, "filename", filename);
    PDWrite_u32(s->currentWriter, "line", 0);
    PDWrite_eventEnd(s->currentWriter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_toggleBreakpointCurrentLine(Session* s)
{
    PDWrite_eventBegin(s->currentWriter, PDEventType_toggleBreakpointCurrentLine);
    PDWrite_u8(s->currentWriter, "dummy", 0);
    PDWrite_eventEnd(s->currentWriter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_stepIn(Session* s)
{
    PDBackendInstance* backend = s->backend;

    if (backend)
        s->state = backend->plugin->update(backend->userData, PDAction_step, s->reader, s->currentWriter);
    else if (s->type == Session_Remote)
        Session_action(s, PDAction_step);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_stepOver(Session* s)
{
    PDBackendInstance* backend = s->backend;

    if (backend)
        s->state = backend->plugin->update(backend->userData, PDAction_stepOver, s->reader, s->currentWriter);
    else if (s->type == Session_Remote)
        Session_action(s, PDAction_stepOver);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SessionStatus Session_onMenu(Session* session, int eventId)
{
    int count = 0;

    if (session->backend)
    {
        PluginData* pluginData = session->backend->pluginData;

        if (pluginData->menuStart >= eventId && eventId < pluginData->menuEnd)
        {
            return SessionStatus_ok;
        }
    }

    PluginData** plugins = PluginHandler_getPlugins(&count);

    for (int i = 0; i < count; ++i)
    {
        PluginData* pluginData = plugins[i];

        if (!strstr(pluginData->type, PD_BACKEND_API_VERSION))
            continue;

        PDBackendPlugin* plugin = (PDBackendPlugin*)pluginData->plugin;

        if (!plugin)
            continue;

        if (!plugin->registerMenu)
            continue;

        // See if this event is own by this plugin

        if (pluginData->menuStart >= eventId && eventId < pluginData->menuEnd)
        {
            PDBackendInstance* backend = session->backend;

            // Create a backend of this type if we don't have a backend already for the session

            if (!backend)
            {
                backend = (PDBackendInstance*)alloc_zero(sizeof(struct PDBackendInstance));
                backend->plugin = plugin;
                backend->pluginData = pluginData;
                backend->userData = backend->plugin->createInstance(0);
            }

            session->backend = backend;

            UIStatusBar_setText("%s Backend active", plugin->name);

            // Write down
        }
    }

    return SessionStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if PRODBG_USING_DOCKING

struct UIDockingGrid* Session_getDockingGrid(struct Session* session)
{
    return session->uiDockingGrid;
}

#endif




