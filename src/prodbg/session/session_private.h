#pragma once

#include <pd_view.h>
#include <pd_backend.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum SessionType
{
    Session_Null,
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
    PDDebugState state;
    struct PDBackendInstance* backend;
    struct RemoteConnection* connection;
    struct ViewPluginInstance** viewPlugins;
} Session;


