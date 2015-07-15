#pragma once

#include <list>

struct PDBackendInstance;
struct RemoteConnection;
struct ViewPluginInstance;
struct UIDockingGrid;
struct Con;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum SessionType
{
    Session_Null,
    Session_Local,
    Session_Remote,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Breakpoint
{
    const char* filename;
    int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Session
{
    inline Session()
    {
        type = Session_Null;
        reader = 0;
        writer0 = 0;
        writer1 = 0;
        tempWriter0 = 0;
        tempWriter1 = 0;

        currentWriter = 0;
        prevWriter = 0;

        state = PDDebugState_noTarget;
        backend = 0;
        connection = 0;
        viewPlugins = 0;
        //uiDockingGrid = 0;
        i3_dock_grid = 0;
    }

    enum SessionType type;
    PDReader* reader;
    PDWriter* writer0;
    PDWriter* writer1;
    PDWriter* tempWriter0;
    PDWriter* tempWriter1;

    PDWriter* currentWriter;
    PDWriter* prevWriter;

    PDDebugState state;
    PDBackendInstance* backend;
    RemoteConnection* connection;
    ViewPluginInstance** viewPlugins;
    //UIDockingGrid* uiDockingGrid;
    Con* i3_dock_grid;

    std::list<Breakpoint*> breakpoints;

} Session;


