#ifndef _WIN32

#include "pd_backend.h"
#include "pd_host.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <LLDB/SBModuleSpec.h>
#include <LLDB/SBTarget.h>
#include <LLDB/SBThread.h>
#include <LLDB/SBListener.h>
#include <LLDB/SBProcess.h>
#include <LLDB/SBDebugger.h>
#include <LLDB/SBHostOS.h>
#include <LLDB/SBEvent.h>
#include <LLDB/SBBreakpoint.h>
#include <LLDB/SBStream.h>
#include <LLDB/SBValueList.h>
#include <LLDB/SBCommandInterpreter.h>
#include <LLDB/SBCommandReturnObject.h>
#include <map>

//static PDMessageFuncs* s_messageFuncs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Breakpoint
{
	const char* filename;
	int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct LLDBPlugin
{
    lldb::SBDebugger debugger;
    lldb::SBTarget target;
    lldb::SBListener listener;
    lldb::SBProcess process;
    PDDebugState state;
    bool hasValidTarget;
    uint64_t selectedThreadId;
    const char* targetName;
	std::map<lldb::tid_t, uint32_t> frameSelection;
	std::vector<Breakpoint> breakpoints;

} LLDBPlugin;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* createInstance(ServiceFunc* serviceFunc)
{
    lldb::SBDebugger::Initialize();

    //s_messageFuncs = (PDMessageFuncs*)serviceFunc(PDMESSAGEFUNCS_GLOBAL);

    LLDBPlugin* plugin = new LLDBPlugin;

    plugin->debugger = lldb::SBDebugger::Create(false);
    plugin->state = PDDebugState_NoTarget;
    plugin->listener = plugin->debugger.GetListener();
    plugin->hasValidTarget = false;
    plugin->selectedThreadId = 0;

    return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t getThreadFrame(LLDBPlugin* plugin, lldb::tid_t threadId)
{
    uint32_t frameIndex = 0;

    auto frameIter = plugin->frameSelection.find(threadId);

    if (frameIter != plugin->frameSelection.end())
    	frameIndex = frameIter->second;

	return frameIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroyInstance(void* user_data)
{
	LLDBPlugin* plugin = (LLDBPlugin*)user_data;
	delete plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const bool m_verbose = true;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStop(LLDBPlugin* plugin)
{
    (void)plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onBreak(LLDBPlugin* plugin)
{
    (void)plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStep(LLDBPlugin* plugin)
{
    lldb::SBEvent evt;

    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selectedThreadId));

    printf("thread stopReason %d\n", thread.GetStopReason());
    printf("threadValid %d\n", thread.IsValid());

    thread.StepInto();

    plugin->state = PDDebugState_Running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void onStepOver(LLDBPlugin* plugin)
{
    lldb::SBEvent evt;

    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selectedThreadId));

    printf("thread stopReason %d\n", thread.GetStopReason());
    printf("threadValid %d\n", thread.IsValid());

    thread.StepOver();

    plugin->state = PDDebugState_Running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void onRun(LLDBPlugin* plugin)
{
    // if we haven't started the executable start it here

    if (plugin->state == PDDebugState_NoTarget)
    {
        lldb::SBLaunchInfo launchInfo(0);
        lldb::SBError error;

        plugin->process = plugin->target.Launch(launchInfo, error);

        printf("try start\n");

        if (!error.Success())
        {
			//s_messageFuncs->error("Error to start executable", "The LLDB backend was unable to start the selected executable. Currently ProDBG requires the user to run it as sudo due to the fact that debugging on Mac needs that. This will be made a bit more friendly in the future");
			printf("Error to start executable\nThe LLDB backend was unable to start the selected executable. Currently ProDBG requires the user to run it as sudo due to the fact that debugging on Mac needs that. This will be made a bit more friendly in the future\n");
            return;
        }

        if (!plugin->process.IsValid())
        {
            printf("process not valid\n");
            return;
        }

        printf("Started valid process\n");

        plugin->process.GetBroadcaster().AddListener(
                plugin->listener,
                lldb::SBProcess::eBroadcastBitStateChanged |
                lldb::SBProcess::eBroadcastBitInterrupt);

        plugin->state = PDDebugState_Running;
        plugin->hasValidTarget = true;

        return;
    }

    plugin->process.Continue();
    plugin->state = PDDebugState_Running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCallstack(LLDBPlugin* plugin, PDWriter* writer)
{
    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selectedThreadId));

    printf("set callstack\n");

    int frameCount = (int)thread.GetNumFrames();

    if (frameCount == 0)
        return;

    // TODO: Write type of callstack

    PDWrite_event_begin(writer, PDEventType_SetCallstack);
    PDWrite_array_begin(writer, "callstack");

    for (int i = 0; i < frameCount; ++i)
    {
        char fileLine[2048];
        char moduleName[2048];

        lldb::SBFrame frame = thread.GetFrameAtIndex((uint32_t)i);
        lldb::SBModule module = frame.GetModule();
        lldb::SBCompileUnit compileUnit = frame.GetCompileUnit();
        lldb::SBSymbolContext context(frame.GetSymbolContext(0x0000006e));
        lldb::SBLineEntry entry(context.GetLineEntry());

        uint64_t address = (uint64_t)frame.GetPC();

        module.GetFileSpec().GetPath(moduleName, sizeof(moduleName));

        PDWrite_array_entry_begin(writer);

        if (compileUnit.GetNumSupportFiles() > 0)
        {
            char filename[2048];
            lldb::SBFileSpec fileSpec = compileUnit.GetSupportFileAtIndex(0);
            fileSpec.GetPath(filename, sizeof(filename));
            sprintf(fileLine, "%s:%d", filename, entry.GetLine());

            printf("callstack %s:%d\n", fileLine, entry.GetLine());

            PDWrite_string(writer, "filename", filename);
            PDWrite_u32(writer, "line", entry.GetLine());
        }

        PDWrite_string(writer, "module_name", moduleName);
        PDWrite_u64(writer, "address", address);

        PDWrite_entry_end(writer);
    }

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExceptionLocation(LLDBPlugin* plugin, PDWriter* writer)
{
    char filename[2048];
    memset(filename, 0, sizeof(filename));

    // Get the filename & line of the exception/breakpoint
    // \todo: Right now we assume that we only got the break/exception at the first thread.

    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selectedThreadId));

    uint32_t frameIndex = getThreadFrame(plugin, plugin->selectedThreadId);

    lldb::SBFrame frame(thread.GetFrameAtIndex(frameIndex));
    lldb::SBCompileUnit compileUnit = frame.GetCompileUnit();
    lldb::SBFileSpec filespec(plugin->process.GetTarget().GetExecutable());

    if (compileUnit.GetNumSupportFiles() > 0)
    {
        lldb::SBFileSpec fileSpec = compileUnit.GetSupportFileAtIndex(0);
        fileSpec.GetPath(filename, sizeof(filename));
    }

    lldb::SBSymbolContext context(frame.GetSymbolContext(lldb::eSymbolContextEverything));
    lldb::SBLineEntry entry(context.GetLineEntry());
    uint32_t line = entry.GetLine();

    PDWrite_event_begin(writer, PDEventType_SetExceptionLocation);
    PDWrite_string(writer, "filename", filename);
    PDWrite_u32(writer, "line", line);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setTty(LLDBPlugin* plugin, PDWriter* writer)
{
    const int bufferSize = 4 * 1024;
    char buffer[bufferSize];

    size_t amountRead = plugin->process.GetSTDOUT(buffer, bufferSize);

    if (amountRead > 0)
    {
        PDWrite_event_begin(writer, PDEventType_SetTty);
        PDWrite_string(writer, "tty", buffer);
        PDWrite_event_end(writer);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExecutable(LLDBPlugin* plugin, PDReader* reader)
{
    const char* filename = 0;

    PDRead_find_string(reader, &filename, "filename", 0);

    if (!filename)
    {
        printf("Unable to find filename which is required when starting a LLDB debug session\n");
        return;
    }

    printf("found filename \"%s\"\n", filename);

    plugin->target = plugin->debugger.CreateTarget(filename);

    if (!plugin->target.IsValid())
	{
        printf("Unable to create valid target (%s)\n", filename);
	}

	for (Breakpoint& bp : plugin->breakpoints)
	{
		lldb::SBBreakpoint breakpoint = plugin->target.BreakpointCreateByLocation(bp.filename, (uint32_t)bp.line);

		if (!breakpoint.IsValid())
		{
			// TODO: Send message back that this breakpoint could't be set
			printf("Unable to set breakpoint %s:%d\n", bp.filename, bp.line);
		}
	}

    printf("Valid target %s\n", filename);

    onRun(plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setLocals(LLDBPlugin* plugin, PDWriter* writer)
{
    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selectedThreadId));
    lldb::SBFrame frame = thread.GetSelectedFrame();

    lldb::SBValueList variables = frame.GetVariables(true, true, true, false);

    uint32_t count = variables.GetSize();

    if (count <= 0)
        return;

    PDWrite_event_begin(writer, PDEventType_SetLocals);
    PDWrite_array_begin(writer, "locals");

    for (uint32_t i = 0; i < count; ++i)
    {
        lldb::SBValue value = variables.GetValueAtIndex(i);

        PDWrite_array_entry_begin(writer);

        PDWrite_u64(writer, "address", value.GetAddress().GetFileAddress());

        if (value.GetValue())
            PDWrite_string(writer, "value", value.GetValue());

        if (value.GetTypeName())
            PDWrite_string(writer, "type", value.GetTypeName());

        if (value.GetName())
            PDWrite_string(writer, "name", value.GetName());

        PDWrite_entry_end(writer);
    }

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setThreads(LLDBPlugin* plugin, PDWriter* writer)
{
    uint32_t threadCount = plugin->process.GetNumThreads();

    if (threadCount == 0)
    	return;

    PDWrite_event_begin(writer, PDEventType_SetThreads);
    PDWrite_array_begin(writer, "threads");

    for (uint32_t i = 0; i < threadCount; ++i)
	{
    	lldb::SBThread thread = plugin->process.GetThreadAtIndex(i);
    	lldb::SBFrame frame = thread.GetFrameAtIndex(0);

		uint64_t threadId = thread.GetThreadID();
    	const char* threadName = thread.GetName();
    	const char* queueName = thread.GetQueueName();
    	const char* functionName = frame.GetFunctionName();

        PDWrite_array_entry_begin(writer);

        PDWrite_u64(writer, "id", threadId);

		if (threadName)
	        PDWrite_string(writer, "name", threadName);
		else if (queueName)
	        PDWrite_string(writer, "name", queueName);
		else
	        PDWrite_string(writer, "name", "unknown_thread");

		if (functionName)
        	PDWrite_string(writer, "function", functionName);
		else
        	PDWrite_string(writer, "function", "unknown_function");

        PDWrite_entry_end(writer);
	}

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setBreakpoint(LLDBPlugin* plugin, PDReader* reader, PDWriter* writer)
{
    const char* filename;
    uint32_t line;

    PDRead_find_string(reader, &filename, "filename", 0);
    PDRead_find_u32(reader, &line, "line", 0);

    // TODO: Handle failure here

    lldb::SBBreakpoint breakpoint = plugin->target.BreakpointCreateByLocation(filename, line);
    if (!breakpoint.IsValid())
    {
		printf("adding breakpoints to breakpoint list %s:%d\n", filename, line);

    	// Unable to set breakpoint as the target doesn't seem to be valid. This is the usual case
    	// if we haven't actually started an executable yet. So we save them here for later and
    	// then set them before launching the executable

		Breakpoint bp = { strdup(filename), (int)line };
		plugin->breakpoints.push_back(bp);

        return;
    }

    printf("Set breakpoint at %s:%d\n", filename, line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void doAction(LLDBPlugin* plugin, PDAction action)
{
    switch (action)
    {
        case PDAction_Stop : onStop(plugin); break;
        case PDAction_Break : onBreak(plugin); break;
        case PDAction_Run : onRun(plugin); break;
        case PDAction_Step : onStep(plugin); break;
        case PDAction_StepOut : onStepOver(plugin); break;
        case PDAction_StepOver : onStepOver(plugin); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void eventAction(LLDBPlugin* plugin, PDReader* reader)
{
    uint32_t action = 0;

    printf("LLDBPlugin; %d\n", (PDRead_find_u32(reader, &action, "action", 0) & 0xff) >> 8);
    printf("LLDBPlugin: got action (from event) %d\n", action);

    doAction(plugin, (PDAction)action);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static const char* eventTypes[] =
{
    "PDEventType_none",
    "PDEventType_getLocals",
    "PDEventType_setLocals",
    "PDEventType_getCallstack",
    "PDEventType_setCallstack",
    "PDEventType_getWatch",
    "PDEventType_setWatch",
    "PDEventType_getRegisters",
    "PDEventType_setRegisters",
    "PDEventType_getMemory",
    "PDEventType_setMemory",
    "PDEventType_getTty",
    "PDEventType_setTty",
    "PDEventType_getExceptionLocation",
    "PDEventType_setExceptionLocation",
    "PDEventType_getDisassembly",
    "PDEventType_setDisassembly",
    "PDEventType_setBreakpoint",
    "PDEventType_getBreakpoint",
    "PDEventType_setExecutable",
    "PDEventType_attachToProcess",
    "PDEventType_attachToRemoteSession",
    "PDEventType_action",
};
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void selectThread(LLDBPlugin* plugin, PDReader* reader, PDWriter* writer)
{
	uint64_t threadId;

    PDRead_find_u64(reader, &threadId, "thread_id", 0);

    printf("trying te set thread %llu\n", threadId);

	if (plugin->selectedThreadId == threadId)
		return;

	printf("selecting thread %llu\n", threadId);

	plugin->selectedThreadId = threadId;

	setCallstack(plugin, writer);

    PDWrite_event_begin(writer, PDEventType_SelectFrame);
    PDWrite_u32(writer, "frame", getThreadFrame(plugin, threadId));
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void selectFrame(LLDBPlugin* plugin, PDReader* reader, PDWriter* writer)
{
	uint32_t frameIndex;

	printf("selectFrame...\n");

    PDRead_find_u32(reader, &frameIndex, "frame", 0);

	plugin->frameSelection[plugin->selectedThreadId] = frameIndex;

	setExceptionLocation(plugin, writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setSourceFiles(LLDBPlugin* plugin, PDWriter* writer)
{
	if (!plugin->hasValidTarget)
		return;

    PDWrite_event_begin(writer, PDEventType_SetSourceFiles);
    PDWrite_array_begin(writer, "files");

    const uint32_t moduleCount = plugin->target.GetNumModules();

    for (uint32_t im = 0; im < moduleCount; ++im)
	{
		lldb::SBModule module(plugin->target.GetModuleAtIndex(im));

    	const uint32_t compileUnitCount = module.GetNumCompileUnits();

		for (uint32_t ic = 0; ic < compileUnitCount; ++ic)
		{
			lldb::SBCompileUnit compileUnit(module.GetCompileUnitAtIndex(ic));

			const uint32_t supportFileCount = compileUnit.GetNumSupportFiles();

			for (uint32_t is = 0; is < supportFileCount; ++is)
			{
				char filename[4096];

				lldb::SBFileSpec fileSpec(compileUnit.GetSupportFileAtIndex(is));

				filename[0] = 0;

        		fileSpec.GetPath(filename, sizeof(filename));

        		if (filename[0] == 0)
        			continue;

				PDWrite_array_entry_begin(writer);
				PDWrite_string(writer, "file", filename);
				PDWrite_entry_end(writer);
			}
		}
	}

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void processEvents(LLDBPlugin* plugin, PDReader* reader, PDWriter* writer)
{
    uint32_t event;

    while ((event = PDRead_get_event(reader)))
    {
        //printf("LLDBPlugin: %d Got event %s\n", event, eventTypes[event]);

        switch (event)
        {
            case PDEventType_GetExceptionLocation : setExceptionLocation(plugin, writer); break;
            case PDEventType_GetCallstack : setCallstack(plugin, writer); break;
            case PDEventType_SetExecutable : setExecutable(plugin, reader); break;
            case PDEventType_SelectThread : selectThread(plugin, reader, writer); break;
            case PDEventType_SelectFrame : selectFrame(plugin, reader, writer); break;
            case PDEventType_GetLocals : setLocals(plugin, writer); break;
            case PDEventType_GetThreads : setThreads(plugin, writer); break;
            case PDEventType_GetSourceFiles : setSourceFiles(plugin, writer); break;
            case PDEventType_SetBreakpoint : setBreakpoint(plugin, reader, writer); break;
            case PDEventType_Action : eventAction(plugin, reader); break;
        }
    }

    setTty(plugin, writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sendExceptionState(LLDBPlugin* plugin, PDWriter* writer)
{
	printf("sending exception state\n");
    //setCallstack(plugin, writer);
    setExceptionLocation(plugin, writer);
    //setLocals(plugin, writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateLLDBEvent(LLDBPlugin* plugin, PDWriter* writer)
{
    if (!plugin->process.IsValid())
	{
		printf("process invalid\n");
        return;
	}

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
                        plugin->state = PDDebugState_Trace;
                        if (m_verbose)
                            printf("trace\n");
                        break;

                    case lldb::eStopReasonPlanComplete:
                        select_thread = true;

                        sendExceptionState(plugin, writer);

                        plugin->state = PDDebugState_Trace;
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
                    {
                        select_thread = true;

                        printf("%d %d\n", plugin->state, PDDebugState_StopException);

                        if (plugin->state != PDDebugState_StopException)
                            sendExceptionState(plugin, writer);

                        plugin->state = PDDebugState_StopException;

                        if (m_verbose)
                            printf("exception\n");
                        fatal = true;

                        break;
                    }

                    case lldb::eStopReasonBreakpoint:
                    {
                        select_thread = true;

                        if (plugin->state != PDDebugState_StopBreakpoint)
                            sendExceptionState(plugin, writer);

                        plugin->state = PDDebugState_StopBreakpoint;

                        if (m_verbose)
                            printf("breakpoint id = %lld.%lld\n",thread.GetStopReasonDataAtIndex(0),thread.GetStopReasonDataAtIndex(1));

                        break;
                    }

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
                    selected_thread = plugin->process.SetSelectedThread(thread);
                    plugin->selectedThreadId = thread.GetThreadID();
                }
            }
        }
        break;
    }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data, PDAction action, PDReader* reader, PDWriter* writer)
{
    LLDBPlugin* plugin = (LLDBPlugin*)user_data;

    processEvents(plugin, reader, writer);

    doAction(plugin, action);

    if (plugin->state == PDDebugState_Running)
        updateLLDBEvent(plugin, writer);

    return plugin->state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin =
{
    "LLDB",
    createInstance,
    destroyInstance,
    0,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif
