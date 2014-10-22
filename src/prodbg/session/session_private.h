#pragma once

#include <list>

struct PDBackendInstance;
struct RemoteConnection;
struct ViewPluginInstance;

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

	std::list<Breakpoint*> breakpoints;

} Session;


