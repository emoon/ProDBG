#include <ProDBGAPI.h>

#ifndef _WIN32

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
#include <SBValueList.h>
#include <SBCommandInterpreter.h> 
#include <SBCommandReturnObject.h> 

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
	PDDebugStateFileLine filelineData;

} LLDBPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* createInstance(ServiceFunc* serviceFunc)
{
	lldb::SBDebugger::Initialize();

	printf("Create instance\n");
	LLDBPlugin* plugin = new LLDBPlugin; 

	plugin->debugger = lldb::SBDebugger::Create(false);
	plugin->debugState = DebugState_default;
 	plugin->listener = plugin->debugger.GetListener(); 

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

static void onStep(LLDBPlugin* plugin)
{
	printf("Do step inside the lldb plugin\n");

	// TODO: Handle more than one thread here

	lldb::SBThread thread(plugin->process.GetThreadAtIndex((size_t)0));

	lldb::SBEvent evt;

	printf("thread stopReason %d\n", thread.GetStopReason());
	printf("threadValid %d\n", thread.IsValid());

	thread.StepInto();

	plugin->debugState = DebugState_updateEvent;

	/*
	// FIXME!

	lldb::StateType state = lldb::SBProcess::GetStateFromEvent(evt);
	plugin->listener.WaitForEvent(1, evt);

	while (state == lldb::eStateRunning)
	{
		plugin->listener.WaitForEvent(1, evt);
		state = lldb::SBProcess::GetStateFromEvent(evt);
	}

	printf("event = %s\n", lldb::SBDebugger::StateAsCString(state));

	printf("thread stopReason %d\n", thread.GetStopReason());
	printf("threadValid %d\n", thread.IsValid());

	printf("%d %d %d\n", thread.IsSuspended(), thread.IsStopped(), thread.GetNumFrames());

	lldb::SBFrame frame(thread.GetSelectedFrame());
	lldb::SBCompileUnit compileUnit = frame.GetCompileUnit();

	printf("frame valid %d\n", frame.IsValid());
	printf("processState %d\n", plugin->process.GetState());

	printf("compileUnit.GetNumSupportFiles() %d\n", compileUnit.GetNumSupportFiles()); 

	if (compileUnit.GetNumSupportFiles() > 0)
	{
		lldb::SBFileSpec fileSpec = compileUnit.GetSupportFileAtIndex(0);
		fileSpec.GetPath((char*)fileLine->filename, 4096); 
	}

	printf("step filename %s\n", fileLine->filename);

	lldb::SBSymbolContext context(frame.GetSymbolContext(0x0000006e));
	lldb::SBModule module(context.GetModule());
	lldb::SBLineEntry entry(context.GetLineEntry());
	lldb::SBFileSpec entry_filespec(plugin->process.GetTarget().GetExecutable());

	fileLine->line = (int)entry.GetLine();
	*/
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
	{
		printf("lldb::SBProcess::GetRestartedFromEvent(evt)\n");
		return;
	}

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
						plugin->debugState = DebugState_stopBreakpoint;
						if (m_verbose)
							printf("trace\n");
						break;
						
					case lldb::eStopReasonPlanComplete:
						select_thread = true;
						plugin->debugState = DebugState_stopBreakpoint;
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

	const int bufferSize = 2048;
	char buffer[bufferSize];
	size_t amountRead = 0;
	while ((amountRead = plugin->process.GetSTDOUT(buffer, bufferSize)) > 0)
	{
		printf("%s", buffer);
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateAction(LLDBPlugin* plugin, PDDebugAction action, void* actionData)
{
	switch (action)
	{
		case PD_DEBUG_ACTION_BREAK : onBreak(plugin, actionData); break;
		case PD_DEBUG_ACTION_STEP : onStep(plugin); break;
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool startDebugging(void* userData, PDLaunchAction action, void* launchData, PDBreakpointFileLine* breakpoints, int bpCount)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	// TODO: Check action here

	plugin->target = plugin->debugger.CreateTarget((const char*)launchData);

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

	// Add breakpoints if we have any

	for (int i = 0; i < bpCount; ++i)
	{
		// TODO: Yeah modifing this one is a race with the main-thread so we should lock at this point

		PDBreakpointFileLine* fileLine = &breakpoints[i]; 

		printf("Trying to add breakpoint %s %d %d\n", fileLine->filename, fileLine->line, fileLine->id);

		if (fileLine->id >= 0)
			continue;

   		lldb::SBBreakpoint breakpoint = plugin->target.BreakpointCreateByLocation(fileLine->filename, (uint32_t)fileLine->line);

		if (breakpoint.IsValid())
		{
			printf("Added breakpoint %s %d\n", fileLine->filename, fileLine->line);
			fileLine->id = breakpoint.GetID();
		}
	}

	plugin->process.GetBroadcaster().AddListener(
			plugin->listener, 
			lldb::SBProcess::eBroadcastBitStateChanged |
			lldb::SBProcess::eBroadcastBitInterrupt);// |
			//lldb::SBProcess::eBroadcastBitSTDOUT);

	plugin->debugState = DebugState_updateEvent;

	// TODO: We should callback to main-thread here so we can update the the UI with the valid breakpoints after we started

	printf("Started ok!\n");

	// TODO

	lldb::SBCommandReturnObject result;
    plugin->debugger.GetCommandInterpreter().HandleCommand("log enable lldb all", result);

	return true;

	//process.Destroy();
	//lldb::SBDebugger::Destroy(plugin->debugger);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void getLocals(void* userData, PDLocals* locals, int* maxEntries)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;
	lldb::SBThread thread(plugin->process.GetThreadAtIndex(0));
	lldb::SBFrame frame = thread.GetSelectedFrame();
	
    lldb::SBValueList variables = frame.GetVariables(true, true, true, false);
    
    uint32_t localVarsCount = variables.GetSize();

    // TODO: Fix me
	*maxEntries = (int)localVarsCount; 

	/*
    if (localVarsCount < (uint32_t)*maxEntries)
    	*maxEntries	= (int)localVarsCount;
    else
    	localVarsCount = (uint32_t)*maxEntries;
    */
    	
    for (uint32_t i = 0; i < localVarsCount; ++i)
    {
    	PDLocals* local = &locals[i]; 
    	lldb::SBValue value = variables.GetValueAtIndex(i);
    	
    	// TODO: Verify this line
    	sprintf(local->address, "%016llx", (uint64_t)value.GetAddress().GetFileAddress());
    	
    	if (value.GetValue())
    		strcpy(local->value, value.GetValue());
   		else
    		strcpy(local->value, "Unknown"); 
    		
    	if (value.GetTypeName())
    		strcpy(local->type, value.GetTypeName());
    	else
    		strcpy(local->type, "Unknown"); 

		if (value.GetName())
    		strcpy(local->name, value.GetName());
    	else
    		strcpy(local->name, "Unknown"); 
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void getCallStack(void* userData, PDCallstack* callStack, int* maxEntries)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;
	
	lldb::SBThread thread(plugin->process.GetThreadAtIndex(0));

	int frameCount = (int)thread.GetNumFrames();
	
	//if (frameCount > *maxEntries)
	//	frameCount = *maxEntries;
		
	// TODO: fix me
	*maxEntries = frameCount;
		
	for (int i = 0; i < frameCount; ++i)
	{
		lldb::SBFrame frame = thread.GetFrameAtIndex((uint32_t)i); 
		lldb::SBModule module = frame.GetModule();
		lldb::SBCompileUnit compileUnit = frame.GetCompileUnit();
		lldb::SBSymbolContext context(frame.GetSymbolContext(0x0000006e));
		lldb::SBLineEntry entry(context.GetLineEntry());

		callStack[i].address = (uint64_t)frame.GetPC();
		module.GetFileSpec().GetPath(callStack[i].moduleName, 1024);
		
		if (compileUnit.GetNumSupportFiles() > 0)
		{
			char filename[2048];
			lldb::SBFileSpec fileSpec = compileUnit.GetSupportFileAtIndex(0);
			fileSpec.GetPath(filename, sizeof(filename));
			sprintf(callStack[i].fileLine, "%s:%d", filename, entry.GetLine());
		}
		else
		{
			strcpy(callStack[i].fileLine, "<unknown>"); 
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState getState(void* userData, PDDebugDataState* dataState)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	switch (plugin->debugState)
	{
		case DebugState_updateEvent : break;
		case DebugState_stopException :
		case DebugState_stopBreakpoint :
		{
			// Get the filename & line of the exception/breakpoint
			// TODO: Right now we assume that we only got the break/exception at the first thread.

			lldb::SBThread thread(plugin->process.GetThreadAtIndex(0));
			lldb::SBFrame frame(thread.GetFrameAtIndex(0));
			lldb::SBCompileUnit compileUnit = frame.GetCompileUnit();
			lldb::SBFileSpec filespec(plugin->process.GetTarget().GetExecutable());

			//filespec.GetPath((char*)&plugin->filelineData.filename, 4096);

			if (compileUnit.GetNumSupportFiles() > 0)
			{
				char filename[2048];
				lldb::SBFileSpec fileSpec = compileUnit.GetSupportFileAtIndex(0);
				fileSpec.GetPath((char*)&dataState->filename, sizeof(filename));
			}

			//auto fp = frame.GetFP();
			lldb::SBThread thread_dup = frame.GetThread();
			char path[1024];
			filespec.GetPath(&path[0],1024);
			//auto state = plugin->process.GetState();
			//auto pCount_dup = plugin->process.GetNumThreads();
			//auto byte_size = plugin->process.GetAddressByteSize();
			//auto pc = frame.GetPC();
			lldb::SBSymbolContext context(frame.GetSymbolContext(0x0000006e));
			lldb::SBModule module(context.GetModule());
			lldb::SBLineEntry entry(context.GetLineEntry());
			lldb::SBFileSpec entry_filespec(plugin->process.GetTarget().GetExecutable());
			//char entry_path[1024];
			//entry_filespec.GetPath(&entry_path[0], 1024);
			auto line_1 = entry.GetLine();
			//auto line_2 = entry.GetLine();
			//auto fname = frame.GetFunctionName();
			dataState->line = (int)line_1;

			//dataState->callStackCount = 32;
			//dataState->localsCount = 64;

			getCallStack(plugin, (PDCallStack*)&dataState->callStack, &dataState->callStackCount);
			getLocals(plugin, (PDLocals*)&dataState->locals, &dataState->localsCount);

			return PDDebugState_breakpoint;
		}

		case DebugState_default : return PDDebugState_default;	// nothing to do yet
	}

	return PDDebugState_default;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int addBreakpoint(void* userData, PDBreakpointType type, void* breakpointData)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	if (!plugin->target.IsValid())
		return -2;

	switch (type)
	{
		case PDBreakpointType_FileLine :
		{
			PDBreakpointFileLine* fileLine = (PDBreakpointFileLine*)breakpointData;
    		lldb::SBBreakpoint breakpoint = plugin->target.BreakpointCreateByLocation(fileLine->filename, (uint32_t)fileLine->line);
    		if (!breakpoint.IsValid())
    			return -1;

    		return (int)breakpoint.GetID();
		}

		case PDBreakpointType_watchPoint :
		case PDBreakpointType_address :
		case PDBreakpointType_custom :
			break;
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void removeBreakpoint(void* userData, int id)
{
	LLDBPlugin* plugin = (LLDBPlugin*)userData;

	if (!plugin->target.IsValid())
		return;

	plugin->target.BreakpointDelete((lldb::break_id_t)id);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugPlugin plugin =
{
	createInstance,
	destroyInstance,
	startDebugging,
	actionCallback,
	getState,
	addBreakpoint,
	removeBreakpoint,
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

#endif
