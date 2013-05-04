#include <ProDBGAPI.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <SBTarget.h>
#include <SBListener.h>
#include <SBProcess.h>
#include <SBDebugger.h>
#include <SBHostOS.h>
#include <SBEvent.h>
#include <SBBreakpoint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct LLDBPlugin
{
	lldb::SBDebugger debugger;
	lldb::SBTarget target;
	lldb::SBListener listener;

} LLDBPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc)
{
	printf("Create instance\n");
	LLDBPlugin* plugin = new LLDBPlugin; 
	return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroyInstance(void* userData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onBreak(LLDBPlugin* data, void* actionData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStep(LLDBPlugin* data, void* actionData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStepOver(LLDBPlugin* data, void* actionData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onSetCodeBreakpoint(LLDBPlugin* data, void* actionData)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void actionCallback(void* userData, PDDebugAction action, void* actionData)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	switch (action)
	{
		case PD_DEBUG_ACTION_BREAK : onBreak(plugin, actionData); break;
		case PD_DEBUG_ACTION_STEP : onStep(plugin, actionData); break;
		case PD_DEBUG_ACTION_STEP_OVER : onStepOver(plugin, actionData); break;
		case PD_DEBUG_ACTION_SET_CODE_BREAKPOINT : onSetCodeBreakpoint(plugin, actionData); break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void startCallback(void* userData, PDLaunchAction action, void* launchData)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	printf("Trying to start debug session\n");

	lldb::SBDebugger::Initialize();
	lldb::SBHostOS::ThreadCreated ("<lldb-tester.app.main>");
 	plugin->debugger = lldb::SBDebugger::Create(false);
 	plugin->listener = plugin->debugger.GetListener(); 

	const char* cat[] =
	{
		"all",
		0
	};

	// very hacky right now but is only used for testing anyway
	plugin->target = plugin->debugger.CreateTarget((const char*)launchData);
	lldb::SBBreakpoint breakpoint = plugin->target.BreakpointCreateByName("main");

	if (breakpoint.IsValid())
	{
		printf("Added valid breakpoint\n");
	}

	if (!plugin->debugger.EnableLog("lldb", cat))
	{
		printf("Unable to enable log!\n");
	}
	
	if (!plugin->target.IsValid())
		return;

	printf("Target is valid, launching\n");

	lldb::SBLaunchInfo launchInfo(0);
	lldb::SBError error;

	lldb::SBProcess process = plugin->target.Launch(launchInfo, error);

	if (!error.Success())
	{
		printf("error false\n");
		return;
	}

	if (!process.IsValid())
	{
		printf("process not valid\n");
		return;
	}

	process.GetBroadcaster().AddListener(
			plugin->listener, 
			lldb::SBProcess::eBroadcastBitStateChanged | lldb::SBProcess::eBroadcastBitInterrupt);

    lldb::SBEvent evt;

    while (1)
	{
		plugin->listener.WaitForEvent (UINT32_MAX, evt);
		lldb::StateType state =  lldb::SBProcess::GetStateFromEvent (evt);
		printf("event = %s\n", lldb::SBDebugger::StateAsCString(state));

		if (state == lldb::eStateStopped)
		{
			process.Kill();
			return;
		}

		usleep(10000);
	}

	//process.Destroy();
	//lldb::SBDebugger::Destroy(plugin->debugger);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugPlugin plugin =
{
	createInstance,
	destroyInstance,
	startCallback,
	actionCallback,
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

void InitPlugin(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin)
{
	printf("Starting to register Plugin!\n");
	registerPlugin(PD_PTYPE_DEBUGGER, &plugin);
}

}

