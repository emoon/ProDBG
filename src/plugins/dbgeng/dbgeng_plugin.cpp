#ifdef _WIN32

#include <pd_backend.h>
#include <assert.h>
#include <stdio.h>
#include <Dbgeng.h>
#include <string>

struct DbgEngPlugin
{
	PDDebugState state = PDDebugState_noTarget;
	bool hasValidTarget = false;

	std::string targetName;

	IDebugClient* debugClient = nullptr;
	IDebugControl* debugControl = nullptr;
};

void onRun(DbgEngPlugin* plugin)
{
	
}

void onStop(DbgEngPlugin* plugin)
{

}

static void onBreak(DbgEngPlugin* plugin)
{

}

static void onStep(DbgEngPlugin* plugin)
{

}

static void onStepOver(DbgEngPlugin* plugin)
{

}

static void onStepOut(DbgEngPlugin* plugin)
{

}

static void doAction(DbgEngPlugin* plugin, PDAction action)
{
	switch (action)
	{
	case PDAction_stop: onStop(plugin); break;
	case PDAction_break: onBreak(plugin); break;
	case PDAction_run: onRun(plugin); break;
	case PDAction_step: onStep(plugin); break;
	case PDAction_stepOut: onStepOut(plugin); break;
	case PDAction_stepOver: onStepOver(plugin); break;
	}
}

static void setExceptionLocation(DbgEngPlugin* plugin, PDWriter* writer)
{

}

static void setCallstack(DbgEngPlugin* plugin, PDWriter* writer)
{

}

static void setExecutable(DbgEngPlugin* plugin, PDReader* reader)
{
	const char* filename = 0;

	PDRead_findString(reader, &filename, "filename", 0);

	if (!filename)
	{
		printf("Unable to find filename which is required when starting a LLDB debug session\n");
		return;
	}

	printf("found filename \"%s\"\n", filename);

	plugin->targetName = filename;

	HRESULT hr = plugin->debugClient->CreateProcess(0, PSTR(filename), DEBUG_PROCESS);
	assert(SUCCEEDED(hr));
	
	printf("Valid target %s\n", filename);
}

static void setLocals(DbgEngPlugin* plugin, PDWriter* writer)
{

}

static void setBreakpoint(DbgEngPlugin* plugin, PDReader* reader, PDWriter* writer)
{

}

static void eventAction(DbgEngPlugin* plugin, PDReader* reader)
{
	uint32_t action = 0;

	printf("DbgEngPlugin; %d\n", (PDRead_findU32(reader, &action, "action", 0) & 0xff) >> 8);
	printf("DbgEngPlugin: got action (from event) %d\n", action);

	doAction(plugin, (PDAction)action);
}

static void processEvents(DbgEngPlugin* plugin, PDReader* reader, PDWriter* writer)
{
	PDEventType event;

	while ((event = (PDEventType)PDRead_getEvent(reader)))
	{
		switch (event)
		{
		case PDEventType_getExceptionLocation: setExceptionLocation(plugin, writer); break;
		case PDEventType_getCallstack: setCallstack(plugin, writer); break;
		case PDEventType_setExecutable: setExecutable(plugin, reader); break;
		case PDEventType_getLocals: setLocals(plugin, writer); break;
		case PDEventType_setBreakpoint: setBreakpoint(plugin, reader, writer); break;
		case PDEventType_action: eventAction(plugin, reader); break;
		}
	}
}


void* createInstance(ServiceFunc* serviceFunc)
{
	DbgEngPlugin* plugin = new DbgEngPlugin;

	HRESULT hr = DebugCreate(__uuidof(IDebugClient), (void**)&plugin->debugClient);
	assert(SUCCEEDED(hr));

	hr = plugin->debugClient->QueryInterface(__uuidof(IDebugControl), (void**)&plugin->debugControl);
	assert(SUCCEEDED(hr));

	return plugin;
}

void destroyInstance(void* userData)
{
	DbgEngPlugin* plugin = reinterpret_cast<DbgEngPlugin*>(userData);

	if (plugin->debugControl)
	{
		plugin->debugControl->Release();
		plugin->debugControl = nullptr;
	}

	if (plugin->debugClient)
	{
		plugin->debugClient->Release();
		plugin->debugClient = nullptr;
	}
		
	delete plugin;
}

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
	DbgEngPlugin* plugin = reinterpret_cast<DbgEngPlugin*>(userData);

	processEvents(plugin, reader, writer);

	doAction(plugin, action);

	/*
	if (plugin->state == PDDebugState_running)
		updateDbgEngEvent(plugin, writer);
	*/

	return plugin->state;
}

static PDBackendPlugin plugin =
{
	"Microsoft Debugger Engine",
	createInstance,
	destroyInstance,
	update,
};

extern "C" PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
{
	registerPlugin(PD_BACKEND_API_VERSION, &plugin, privateData);
}

#endif
