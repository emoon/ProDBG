#ifdef PRODBG_MAC
#include <foundation/apple.h>
#endif
#include "session.h"
#include "session_private.h"
#include "api/plugin_instance.h"
#include "api/src/remote/pd_readwrite_private.h"
#include "api/src/remote/remote_connection.h"
#include "core/alloc.h"
#include "core/log.h"
#include "core/math.h"
#include "core/file_monitor.h"
#include "core/script.h"
#include "core/plugin_handler.h"
#include "core/service.h"
#include "ui/plugin.h"
#include "ui/bgfx/ui_host.h"
#include "ui/bgfx/ui_dock_private.h" // TODO: Fix me
#include "ui/plugin.h"
#include "i3wm_docking.h"


#include <stdlib.h>
//#include <stb.h>
#include <assert.h>

#include "pd_view.h"
#include "pd_backend.h"

#include <foundation/array.h>
#include <foundation/path.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    ReadWriteBufferSize = 2 * 1024 * 1024,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if FOUNDATION_PLATFORM_APPLE
#define LIB_EXT ".dylib"
#elif FOUNDATION_PLATFORM_WINDOWS
#define LIB_EXT ".dll"
#else
#define LIB_EXT ".so"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Session** s_sessions = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fileUpdateCallback(void* userData, const char* file, int type) {
    (void)userData;

    // TODO: Only supporting modified files for now

    if (type != FOUNDATIONEVENT_FILE_MODIFIED)
        return;

    const char* filename = path_base_file_name(file);

    PluginData* pluginData = PluginHandler_findPluginByFilename(filename);

    if (!pluginData)
        return;

    void* pluginPtr = pluginData->plugin;

    PluginData* reloadPluginData = PluginHandler_reloadPlugin(pluginData);

    PDViewPlugin* newPluginPtr = (PDViewPlugin*)reloadPluginData->plugin;

    int sessionCount = array_size(s_sessions);

    for (int i = 0; i < sessionCount; ++i) {
        Session* session = s_sessions[i];

        int viewPluginCount = array_size(session->viewPlugins);

        for (int p = 0; p < viewPluginCount; ++p) {
            ViewPluginInstance* instance = session->viewPlugins[p];
            PDViewPlugin* instancePlugin = instance->plugin;

            if (instancePlugin == pluginPtr)
                instance->plugin = newPluginPtr;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_globalInit(bool reloadPlugins) {
    if (!reloadPlugins)
        return;

    //init_logging();

    FileMonitor_addPath(OBJECT_DIR, LIB_EXT, fileUpdateCallback, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_globalDestroy() {
    int count = array_size(s_sessions);

    for (int i = 0; i < count; ++i)
        delete s_sessions[i];

    array_clear(s_sessions);

    FileMonitor_removePath(OBJECT_DIR);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_destroy(Session* session) {
    int count = array_size(s_sessions);

    for (int i = 0; i < count; ++i) {
        if (s_sessions[i] != session)
            continue;

        if (session->backend)
            session->backend->plugin->destroy_instance(session->backend->userData);

        delete session;

        array_erase(s_sessions, i);

        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session** Session_getSessions() {
    return s_sessions;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateLocal(Session* s, PDAction action);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void commonInit(Session* s) {
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

struct Session* Session_create() {
    Session* s = new Session;

    g_pluginUI->setStatusText("Not running");

    commonInit(s);

    array_push(s_sessions, s);

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_createDockingGrid(Session* session, int width, int height) {
    (void)session;
    (void)width;
    (void)height;

    docksys_create(0, 0, width, height);

    session->i3_dock_grid = 0; //docksys_create_workspace("test_ws");

    //tree_open_con(NULL, NULL);
    //tree_split(focused, HORIZ);

    printf("cerated dock grid %p\n", session->i3_dock_grid);

#if 0
    IntRect rect = {{{ 0, 0, width, height }}};

    session->uiDockingGrid = 0; UIDock_createGrid(&rect);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Session_loadLayout(Session* session, const char* filename, int width, int height) {
    (void)session;
    (void)filename;
    (void)width;
    (void)height;
    /*
       UIDockingGrid* grid = UIDock_loadLayout(filename, width, height);

       if (!grid)
        return false;

       session->uiDockingGrid = grid;

       // TODO: Fix me

       for (UIDock* dock : grid->docks)
        Session_addViewPlugin(session, dock->view);
     */

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_startRemote(Session* s, const char* target, int port) {
    s->type = Session_Remote;

    struct RemoteConnection* conn = RemoteConnection_create(RemoteConnectionType_Connect, port);

    if (!RemoteConnection_connect(conn, target, port)) {
        pd_info("Unable to connect to %s:%d", target, port);
        RemoteConnection_destroy(conn);
        return s;
    }else {
        s->connection = conn;
    }

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Session_isConnected(Session* session) {
    if (!session->connection)
        return 0;

    return RemoteConnection_isConnected(session->connection);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_createRemote(const char* target, int port) {
    Session* s = new Session;

    commonInit(s);

    Session_startRemote(s, target, port);

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_startLocal(Session* s, PDBackendPlugin* backend, const char* filename) {
    // Create the backend

    s->type = Session_Local;
    s->backend = (PDBackendInstance*)alloc_zero(sizeof(struct PDBackendInstance));
    s->backend->plugin = backend;
    s->backend->userData = backend->create_instance(Service_getService);

    // Set the executable if we have any

    if (filename) {
        PDWrite_event_begin(s->currentWriter, PDEventType_SetExecutable);
        PDWrite_string(s->currentWriter, "filename", filename);
        PDWrite_event_end(s->currentWriter);

        // Add existing breakpoints

        for (auto i = s->breakpoints.begin(), end = s->breakpoints.end(); i != end; ++i) {
            PDWrite_event_begin(s->currentWriter, PDEventType_SetBreakpoint);
            PDWrite_string(s->currentWriter, "filename", (*i)->filename);
            PDWrite_u32(s->currentWriter, "line", (unsigned int)(*i)->line);
            PDWrite_event_end(s->currentWriter);
        }
    }

    // TODO: Not run directly but allow user to select if run, otherwise (ProDG style stop-at-main?)

    // updateLocal(s, PDAction_run);

    //pd_info("second update\n");

    return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_createLocal(PDBackendPlugin* backend, const char* filename) {
    Session* s = new Session;

    commonInit(s);

    return Session_startLocal(s, backend, filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* getStateName(int state) {
    if (state < PDDebugState_Count && state >= 0) {
        switch (state) {
            case PDDebugState_NoTarget:
                return "No target";
            case PDDebugState_Running:
                return "Running";
            case PDDebugState_StopBreakpoint:
                return "Stop (breakpoint)";
            case PDDebugState_StopException:
                return "Stop (exception)";
            case PDDebugState_Trace:
                return "Trace (stepping)";
        }
    }

    return "Unknown";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void doToggleBreakpoint(Session* s, PDReader* reader) {
    const char* filename;
    uint32_t line;

    PDRead_find_string(reader, &filename, "filename", 0);
    PDRead_find_u32(reader, &line, "line", 0);

    for (auto i = s->breakpoints.begin(), end = s->breakpoints.end(); i != end; ++i) {
        if ((*i)->line == (int)line && !strcmp((*i)->filename, filename)) {
            free((void*)(*i)->filename);
            s->breakpoints.erase(i);
            return;
        }
    }

    Breakpoint* breakpoint = (Breakpoint*)malloc(sizeof(Breakpoint));
    breakpoint->filename = strdup(filename);
    breakpoint->line = (int)line;

    printf("adding new breakpoint to session %s:%d\n", breakpoint->filename, breakpoint->line);

    s->breakpoints.push_back(breakpoint);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void toggleBreakpoint(Session* s, PDReader* reader) {
    uint32_t event;

    while ((event = PDRead_get_event(reader)) != 0) {
        switch (event) {
            case PDEventType_SetBreakpoint:
            {
                printf("session breakpoint\n");
                doToggleBreakpoint(s, reader);
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void executeCommand(Session* s, PDReader* reader) {
    (void)s;

    const char* command;
    PDRead_find_string(reader, &command, "command", 0);
    ScriptState* scriptState;
    Script_createState(&scriptState);
    if (Script_loadString(scriptState, command))
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
    Script_destroyState(&scriptState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateScript(Session* s, PDReader* reader) {
    uint32_t event;

    while ((event = PDRead_get_event(reader)) != 0) {
        switch (event) {
            case PDEventType_ExecuteConsole:
            {
                executeCommand(s, reader);
                break;
            }
        }
    }
}

// TOOD: Fix me

Con* getCoveredCon(int x, int y);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateLocal(Session* s, PDAction action) {
    PDBinaryWriter_finalize(s->currentWriter);

    // Swap the write buffers

    PDWriter* temp = s->currentWriter;
    s->currentWriter = s->prevWriter;
    s->prevWriter = temp;

    unsigned int reqDataSize = PDBinaryWriter_getSize(s->prevWriter);
    PDBackendInstance* backend = s->backend;

    PDBinaryReader_reset(s->reader);

    toggleBreakpoint(s, s->reader);

    // TODO: Temporary hack, send no request data to backend if we are running.

    //if (s->state == PDDebugState_running)
    //   reqDataSize = 0;

    PDBinaryReader_initStream(s->reader, PDBinaryWriter_getData(s->prevWriter), reqDataSize);
    PDBinaryWriter_reset(s->currentWriter);

    if (backend) {
        s->state = backend->plugin->update(backend->userData, action, s->reader, s->currentWriter);
        if (g_pluginUI)
            g_pluginUI->setStatusText("%s Backend: %s", backend->plugin->name, getStateName(s->state));
    }

    int len = array_size(s->viewPlugins);

    PDBinaryReader_initStream(s->reader, PDBinaryWriter_getData(s->prevWriter), PDBinaryWriter_getSize(s->prevWriter));
    PDBinaryReader_reset(s->reader);

    for (int i = 0; i < len; ++i) {
        struct ViewPluginInstance* p = s->viewPlugins[i];
        PluginUI::State state = g_pluginUI->updateInstance(p, s->reader, s->currentWriter);

        if (state == PluginUI::CloseView) {
            docksys_close_con(p);
            p->markDeleted = true;
        }

        PDBinaryReader_reset(s->reader);
    }

    updateScript(s, s->reader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* getBackendState(PDReader* reader) {
    uint32_t event;
    uint32_t state;
    const char* retState = "Unknown";

    while ((event = PDRead_get_event(reader)) != 0) {
        switch (event) {
            case PDEventType_SetStatus:
            {
                PDRead_find_u32(reader, &state, "state", 0);
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

static void updateRemote(Session* s, PDAction action) {
    if (!s->connection)
        return;

    (void)action;

    PDBinaryReader_reset(s->reader);

    if (RemoteConnection_pollRead(s->connection)) {
        int totalSize = 0;
        uint8_t cmd[4];
        uint8_t* outputBuffer;

        // TODO: Make this a bit less hardcoded (cmd decode)

        if (RemoteConnection_recv(s->connection, (char*)&cmd, 4, 0)) {
            totalSize = (((int)(cmd[0])) << 24) | (((int)(cmd[1])) << 16) | (((int)(cmd[2])) << 8) | ((int)(cmd[3]));

            outputBuffer = RemoteConnection_recvStream(s->connection, 0, totalSize);

            PDBinaryReader_initStream(s->reader, outputBuffer, (unsigned int)totalSize);
        }
    }

    int len = array_size(s->viewPlugins);

    for (int i = 0; i < len; ++i) {
        struct ViewPluginInstance* p = s->viewPlugins[i];
        PluginUI::State state = g_pluginUI->updateInstance(p, s->reader, s->currentWriter);

        if (state == PluginUI::CloseView) {
            docksys_close_con(p);
            p->markDeleted = true;
        }

        PDBinaryReader_reset(s->reader);
    }

    PDBinaryWriter_finalize(s->currentWriter);

    // Swap the write buffers

    PDWriter* temp = s->currentWriter;
    s->currentWriter = s->prevWriter;
    s->prevWriter = temp;

    if (PDBinaryWriter_getSize(s->prevWriter) > 4) {
        if (PDBinaryWriter_getSize(s->prevWriter) > 4 && RemoteConnection_isConnected(s->connection)) {
            RemoteConnection_sendStream(s->connection, PDBinaryWriter_getData(s->prevWriter));
        }
    }

    PDBinaryWriter_reset(s->currentWriter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Remove vewies that has been marked deleted

static void updateMarkedDelete(Session* s) {
    int count = array_size(s->viewPlugins);

    for (int i = 0; i < count; ++i) {
        struct ViewPluginInstance* p = s->viewPlugins[i];

        if (p->markDeleted) {
            Session_removeViewPlugin(s, p);
            count = array_size(s->viewPlugins);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_update(Session* s) {
    switch (s->type) {
        case Session_Null:
        case Session_Local:
        {
            updateLocal(s, PDAction_None);
            break;
        }

        case Session_Remote:
        {
            updateRemote(s, PDAction_None);
            break;
        }
    }

    updateMarkedDelete(s);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_action(Session* s, PDAction action) {
    if (!s)
        return;

    if (s->type == Session_Local) {
        updateLocal(s, action);
    }else {
        if (RemoteConnection_isConnected(s->connection)) {
            uint8_t command[4];
            command[0] = 1 << 7; // action tag
            command[1] = 0;
            command[2] = (action >> 8) & 0xff;
            command[3] = (action >> 0) & 0xff;
            int t = RemoteConnection_send(s->connection, &command, sizeof(uint32_t), 0);
            (void)t;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_addViewPlugin(struct Session* session, struct ViewPluginInstance* instance) {
    array_push(session->viewPlugins, instance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Session_removeViewPlugin(Session* session, struct ViewPluginInstance* plugin) {
    int count = array_size(session->viewPlugins);

    if (count == 0)
        return true;

    if (count == 1) {
        array_pop(session->viewPlugins);
        return true;
    }

    for (int i = 0; i < count; ++i) {
        if (session->viewPlugins[i] == plugin) {
            array_erase(session->viewPlugins, i);
            return true;
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ViewPluginInstance** Session_getViewPlugins(struct Session* session, int* count) {
    *count = array_size(session->viewPlugins);
    return session->viewPlugins;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Somewhat temporay functions

void Session_loadSourceFile(Session* s, const char* filename) {
    PDWrite_event_begin(s->currentWriter, PDEventType_SetExceptionLocation);
    PDWrite_string(s->currentWriter, "filename", filename);
    PDWrite_u32(s->currentWriter, "line", 0);
    PDWrite_event_end(s->currentWriter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_toggleBreakpointCurrentLine(Session* s) {
    PDWrite_event_begin(s->currentWriter, PDEventType_ToggleBreakpointCurrentLine);
    PDWrite_u8(s->currentWriter, "dummy", 0);
    PDWrite_event_end(s->currentWriter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_stepIn(Session* s) {
    PDBackendInstance* backend = s->backend;

    if (backend)
        s->state = backend->plugin->update(backend->userData, PDAction_Step, s->reader, s->currentWriter);
    else if (s->type == Session_Remote)
        Session_action(s, PDAction_Step);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_stepOver(Session* s) {
    PDBackendInstance* backend = s->backend;

    if (backend)
        s->state = backend->plugin->update(backend->userData, PDAction_StepOver, s->reader, s->currentWriter);
    else if (s->type == Session_Remote)
        Session_action(s, PDAction_StepOver);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SessionStatus Session_onMenu(Session* session, int eventId) {
    int count = 0;

    if (session->backend) {
        PluginData* pluginData = session->backend->pluginData;

        if (pluginData->menuStart >= eventId && eventId < pluginData->menuEnd) {
            return SessionStatus_ok;
        }
    }

    PluginData** plugins = PluginHandler_getBackendPlugins(&count);

    for (int i = 0; i < count; ++i) {
        PluginData* pluginData = plugins[i];

        PDBackendPlugin* plugin = (PDBackendPlugin*)pluginData->plugin;

        if (!plugin)
            continue;

        if (!plugin->register_menu)
            continue;

        // See if this event is own by this plugin

        if (pluginData->menuStart <= eventId && eventId < pluginData->menuEnd) {
            PDBackendInstance* backend = session->backend;

            // Create a backend of this type if we don't have a backend already for the session

            if (!backend) {
                backend = (PDBackendInstance*)alloc_zero(sizeof(struct PDBackendInstance));
                backend->plugin = plugin;
                backend->pluginData = pluginData;
                backend->userData = backend->plugin->create_instance(Service_getService);
            }

            session->backend = backend;
            session->type = Session_Local;

            g_pluginUI->setStatusText("%s Backend active", plugin->name);

            // Write down

            PDWrite_event_begin(session->currentWriter, PDEventType_MenuEvent);
            PDWrite_u32(session->currentWriter, "menu_id", (uint32_t)(eventId - pluginData->menuStart));
            PDWrite_event_end(session->currentWriter);
        }
    }

    return SessionStatus_ok;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Con* Session_getDockingGrid(struct Session* session) {
    //return session->uiDockingGrid;
    return session->i3_dock_grid;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ViewPluginInstance* Session_getViewAt(struct Session* session, int x, int y, int border) {
    int count = 0;

    ViewPluginInstance** instances = Session_getViewPlugins(session, &count);

    for (int i = 0; i < count; ++i) {
        IntRect rect = instances[i]->rect;

        const int x0 = rect.x;
        const int y0 = rect.y;
        const int x1 = (rect.width + x0) - border;
        const int y1 = (rect.height + y0) - border;

        if ((x >= x0 && x < x1) && (y >= y0 && y < y1))
            return instances[i];
    }

    return 0;
}


