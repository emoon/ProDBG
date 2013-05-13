#include <ProDBGAPI.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <SBTarget.h>
#include <SBThread.h>
#include <SBListener.h>
#include <SBProcess.h>
#include <SBDebugger.h>
#include <SBHostOS.h>
#include <SBEvent.h>
#include <SBBreakpoint.h>
#include <SBStream.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum DebugState
{
	DebugState_default,
	DebugState_updateEvent,
	DebugState_stopException,
	DebugState_stopBreakpoint,
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct LLDBPlugin
{
	lldb::SBDebugger debugger;
	lldb::SBTarget target;
	lldb::SBListener listener;
	lldb::SBProcess process;
	DebugState debugState;

} LLDBPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc)
{
	printf("Create instance\n");
	LLDBPlugin* plugin = new LLDBPlugin; 
	plugin->debugState = DebugState_default;
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

static void onContinue(LLDBPlugin* data, void* actionData)
{
	data->process.Continue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum State
{
	State_Default,
	State_InvalidProcess,
	State_Exit,
	State_Restart
};

const bool m_verbose = true;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateLLDBEvent(LLDBPlugin* plugin)
{
	if (!plugin->process.IsValid())
		return;

	lldb::SBEvent evt;

	plugin->listener.WaitForEvent(1, evt);
	lldb::StateType state = lldb::SBProcess::GetStateFromEvent(evt);

	printf("event = %s\n", lldb::SBDebugger::StateAsCString(state));

	if (lldb::SBProcess::GetRestartedFromEvent(evt))
		return;

	switch (state)
	{
		case lldb::eStateInvalid:
		case lldb::eStateDetached:
		case lldb::eStateCrashed:
		case lldb::eStateUnloaded:
			return;

		case lldb::eStateExited:
			return;

		case lldb::eStateConnected:
		case lldb::eStateAttaching:
		case lldb::eStateLaunching:
		case lldb::eStateRunning:
		case lldb::eStateStepping:
			return;

		case lldb::eStateStopped:
		case lldb::eStateSuspended:
		{
			//call_test_step = true;
			bool fatal = false;
			bool selected_thread = false;
			for (uint32_t thread_index = 0; thread_index < plugin->process.GetNumThreads(); thread_index++)
			{
				lldb::SBThread thread(plugin->process.GetThreadAtIndex((size_t)thread_index));
				lldb::SBFrame frame(thread.GetFrameAtIndex(0));
				bool select_thread = false;
				lldb::StopReason stop_reason = thread.GetStopReason();

				if (m_verbose) 
					printf("tid = 0x%llx pc = 0x%llx ",thread.GetThreadID(),frame.GetPC());

				switch (stop_reason)
				{
					case lldb::eStopReasonNone:
						if (m_verbose)
							printf("none\n");
						break;
						
					case lldb::eStopReasonTrace:
						select_thread = true;
						if (m_verbose)
							printf("trace\n");
						break;
						
					case lldb::eStopReasonPlanComplete:
						select_thread = true;
						if (m_verbose)
							printf("plan complete\n");
						break;
					case lldb::eStopReasonThreadExiting:
						if (m_verbose)
							printf("thread exiting\n");
						break;
					case lldb::eStopReasonExec:
						if (m_verbose)
							printf("exec\n");
						break;
					case lldb::eStopReasonInvalid:
						if (m_verbose)
							printf("invalid\n");
						break;
					case lldb::eStopReasonException:
						select_thread = true;
						plugin->debugState = DebugState_stopException;
						if (m_verbose)
							printf("exception\n");
						fatal = true;
						break;
					case lldb::eStopReasonBreakpoint:
						select_thread = true;
						plugin->debugState = DebugState_stopBreakpoint;
						if (m_verbose)
							printf("breakpoint id = %lld.%lld\n",thread.GetStopReasonDataAtIndex(0),thread.GetStopReasonDataAtIndex(1));
						break;
					case lldb::eStopReasonWatchpoint:
						select_thread = true;
						if (m_verbose)
							printf("watchpoint id = %lld\n",thread.GetStopReasonDataAtIndex(0));
						break;
					case lldb::eStopReasonSignal:
						select_thread = true;
						if (m_verbose)
							printf("signal %d\n",(int)thread.GetStopReasonDataAtIndex(0));
						break;
				}
				if (select_thread && !selected_thread)
				{
					//m_thread = thread;
					selected_thread = plugin->process.SetSelectedThread(thread);
				}
			}
		}
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateAction(LLDBPlugin* plugin, PDDebugAction action, void* actionData)
{
	switch (action)
	{
		case PD_DEBUG_ACTION_BREAK : onBreak(plugin, actionData); break;
		case PD_DEBUG_ACTION_STEP : onStep(plugin, actionData); break;
		case PD_DEBUG_ACTION_CONTINUE : onContinue(plugin, actionData); break;
		case PD_DEBUG_ACTION_STEP_OVER : onStepOver(plugin, actionData); break;
		case PD_DEBUG_ACTION_SET_CODE_BREAKPOINT : onSetCodeBreakpoint(plugin, actionData); break;
		case PD_DEBUG_ACTION_NONE : break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void actionCallback(void* userData, PDDebugAction action, void* actionData)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	switch (plugin->debugState)
	{
		case DebugState_updateEvent : updateLLDBEvent(plugin); break;
		case DebugState_stopException :
		case DebugState_stopBreakpoint :
		{
			updateAction(plugin, action, actionData);
			break;
		}

		case DebugState_default : return;	// nothing to do yet
	}

	printf("callback\n");

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool startCallback(void* userData, PDLaunchAction action, void* launchData)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	printf("Trying to start debug session\n");

	lldb::SBDebugger::Initialize();
	//lldb::SBHostOS::ThreadCreated ("<lldb-tester.app.main>");
 	plugin->debugger = lldb::SBDebugger::Create(false);
 	plugin->listener = plugin->debugger.GetListener(); 

	/*
	const char* cat[] =
	{
		"all",
		0
	};
	*/

	// very hacky right now but is only used for testing anyway
	plugin->target = plugin->debugger.CreateTarget((const char*)launchData);

	// dump some data
	//
	
#if 0
	for (uint32_t i = 0; i < 1;/*plugin->target.GetNumModules()*/ ++i)
	{
		lldb::SBStream desc;
    	lldb::SBModule module = plugin->target.GetModuleAtIndex(i);

    	module.GetDescription(desc);

    	printf("module desc %s\n", desc.GetData());
    	printf("CompileUnits %d\n", module.GetNumCompileUnits());

		for (size_t k = 0; k < module.GetNumCompileUnits(); ++k)
		{
			lldb::SBCompileUnit unit = module.GetCompileUnitAtIndex((uint32_t)k);

			for (size_t p = 0; p < unit.GetNumSupportFiles(); ++p)
			{
				lldb::SBFileSpec fileSpec = unit.GetSupportFileAtIndex((uint32_t)p);
				printf("support file %s\n", fileSpec.GetFilename());
			}
		}

		for (uint32_t k = 0; k < module.GetNumSymbols(); ++k)
		{
			lldb::SBSymbol symbol = module.GetSymbolAtIndex(k);
			printf("%d %s\n", k, symbol.GetName());
		}

		/*
    	printf("ModuleDesc %s\n", desc.GetData());
    	
    	for (uint32_t p = 0; p < module.GetNumSections(); ++p)
		{
			lldb::SBSection section = module.GetSectionAtIndex(p);
			printSectionsReursive(section, 0);
		}
		*/
	}
#endif


	/*
	if (breakpoint.IsValid())
	{
		printf("Added valid breakpoint\n");
	}

	if (!plugin->debugger.EnableLog("lldb", cat))
	{
		printf("Unable to enable log!\n");
	}
	*/
	
	if (!plugin->target.IsValid())
		return false;

	printf("Target is valid, launching\n");

	lldb::SBLaunchInfo launchInfo(0);
	lldb::SBError error;

	plugin->process = plugin->target.Launch(launchInfo, error);

	if (!error.Success())
	{
		printf("error false\n");
		return false;
	}

	if (!plugin->process.IsValid())
	{
		printf("process not valid\n");
		return false;
	}

	plugin->process.GetBroadcaster().AddListener(
			plugin->listener, 
			lldb::SBProcess::eBroadcastBitStateChanged | lldb::SBProcess::eBroadcastBitInterrupt);

	plugin->debugState = DebugState_updateEvent;

	printf("Started ok!\n");

	return true;

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

/*
if (call_test_step)
{
	do_the_call:
	if (m_verbose)
		printf("RUNNING STEP %d\n",m_step);
	ActionWanted action;
	TestStep(m_step, action);
	m_step++;
	SBError err;
	switch (action.type)
	{
	case ActionWanted::Type::eContinue:
		err = m_process.Continue();
		break;
	case ActionWanted::Type::eStepOut:
		if (action.thread.IsValid() == false)
		{
			if (m_verbose)
			{
				Xcode::RunCommand(m_debugger,"bt all",true);
				printf("error: invalid thread for step out on step %d\n", m_step);
			}
			exit(501);
		}
		m_process.SetSelectedThread(action.thread);
		action.thread.StepOut();
		break;
	case ActionWanted::Type::eStepOver:
		if (action.thread.IsValid() == false)
		{
			if (m_verbose)
			{
				Xcode::RunCommand(m_debugger,"bt all",true);
				printf("error: invalid thread for step over %d\n",m_step);
			}
			exit(500);
		}
		m_process.SetSelectedThread(action.thread);
		action.thread.StepOver();
		break;
	case ActionWanted::Type::eRelaunch:
		if (m_process.IsValid())
		{
			m_process.Kill();
			m_process.Clear();
		}
		Launch(action.launch_info);
		break;
	case ActionWanted::Type::eKill:
		if (m_verbose)
			printf("kill\n");
		m_process.Kill();
		return;
	case ActionWanted::Type::eCallNext:
		goto do_the_call;
		break;
	}
*/

