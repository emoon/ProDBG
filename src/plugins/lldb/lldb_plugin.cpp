#ifndef _WIN32

#include <LLDB/SBBreakpoint.h>
#include <LLDB/SBCommandInterpreter.h>
#include <LLDB/SBCommandReturnObject.h>
#include <LLDB/SBDebugger.h>
#include <LLDB/SBEvent.h>
#include <LLDB/SBHostOS.h>
#include <LLDB/SBListener.h>
#include <LLDB/SBModuleSpec.h>
#include <LLDB/SBProcess.h>
#include <LLDB/SBStream.h>
#include <LLDB/SBTarget.h>
#include <LLDB/SBThread.h>
#include <LLDB/SBValueList.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include "pd_backend.h"
#include "pd_host.h"
#include "pd_backend_messages.h"

// static PDMessageFuncs* s_messageFuncs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Breakpoint {
    const char* filename;
    int line;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
typedef struct LLDBPlugin {
    lldb::SBDebugger debugger;
    lldb::SBTarget target;
    lldb::SBListener listener;
    lldb::SBProcess process;
    PDDebugState state;
    bool has_valid_target;
    uint64_t selected_thread_id;
    const char* targetName;
    std::map<lldb::tid_t, uint32_t> frame_selection;
    std::vector<Breakpoint> breakpoints;

} LLDBPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* create_instance(ServiceFunc* serviceFunc) {
    printf("lldb: create instance\n");
    lldb::SBDebugger::Initialize();

    LLDBPlugin* plugin = new LLDBPlugin;

    plugin->debugger = lldb::SBDebugger::Create(false);
    plugin->state = PDDebugState_NoTarget;
    plugin->listener = plugin->debugger.GetListener();
    plugin->has_valid_target = false;
    plugin->selected_thread_id = 0;

    return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t getThreadFrame(LLDBPlugin* plugin, lldb::tid_t threadId) {
    uint32_t frameIndex = 0;

    auto frameIter = plugin->frame_selection.find(threadId);

    if (frameIter != plugin->frame_selection.end()) {
        frameIndex = frameIter->second;
    }

    return frameIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroy_instance(void* user_data) {
    LLDBPlugin* plugin = (LLDBPlugin*)user_data;
    delete plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const bool m_verbose = true;

static void on_step(LLDBPlugin* plugin) {
    lldb::SBEvent evt;

    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selected_thread_id));

    printf("thread stopReason %d\n", thread.GetStopReason());
    printf("threadValid %d\n", thread.IsValid());

    thread.StepInto();

    plugin->state = PDDebugState_Running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static void on_stop(LLDBPlugin* plugin) {
    (void)plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void on_break(LLDBPlugin* plugin) {
    (void)plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void on_step_over(LLDBPlugin* plugin) {
    lldb::SBEvent evt;

    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selected_thread_id));

    printf("thread stopReason %d\n", thread.GetStopReason());
    printf("threadValid %d\n", thread.IsValid());

    thread.StepOver();

    plugin->state = PDDebugState_Running;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void on_run(LLDBPlugin* plugin) {
    // if we haven't started the executable start it here

    if (plugin->state == PDDebugState_NoTarget) {
        lldb::SBLaunchInfo launchInfo(0);
        lldb::SBError error;

        plugin->process = plugin->target.Launch(launchInfo, error);

        printf("try start\n");

        if (!error.Success()) {
            // s_messageFuncs->error("Error to start executable", "The LLDB backend was unable to start the selected
            // executable. Currently ProDBG requires the user to run it as sudo due to the fact that debugging on Mac
            // needs that. This will be made a bit more friendly in the future");

            printf("LLDB: Unable to start debugging %s\n", error.GetCString());
            return;
        }

        if (!plugin->process.IsValid()) {
            printf("process not valid\n");
            return;
        }

        printf("Started valid process\n");

        plugin->process.GetBroadcaster().AddListener(
            plugin->listener, lldb::SBProcess::eBroadcastBitStateChanged | lldb::SBProcess::eBroadcastBitInterrupt);

        plugin->state = PDDebugState_Running;
        plugin->has_valid_target = true;

        return;
    }

    plugin->process.Continue();
    plugin->state = PDDebugState_Running;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void reply_callstack(LLDBPlugin* plugin, PDWriter* writer) {
    flatbuffers::FlatBufferBuilder builder(1024);
    std::vector<flatbuffers::Offset<CallstackEntry>> entries;

    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selected_thread_id));

    for (uint32_t i = 0, c = thread.GetNumFrames(); i < c; ++i) {
        char filename[4096];
        char module_name[4096];

        lldb::SBFrame frame = thread.GetFrameAtIndex(i);
        lldb::SBModule mod = frame.GetModule();
        lldb::SBSymbolContext context(frame.GetSymbolContext(lldb::eSymbolContextEverything));
        lldb::SBLineEntry entry(context.GetLineEntry());

        uint64_t address = (uint64_t)frame.GetPC();

        mod.GetFileSpec().GetPath(module_name, sizeof(module_name));

        uint32_t line = entry.GetLine();
        entry.GetFileSpec().GetPath(filename, sizeof(filename));

        entries.push_back(CreateCallstackEntryDirect(builder, address, module_name, filename, (int)line));
    }

    auto t = builder.CreateVector<flatbuffers::Offset<CallstackEntry>>(entries);

    CallstackBuilder reply(builder);
    reply.add_entries(t);

    PDMessage_end_msg(writer, reply, builder);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void exception_location_reply(LLDBPlugin* plugin, PDWriter* writer) {
    char filename[2048] = { 0 };

    // Get the filename & line of the exception/breakpoint
    // \todo: Right now we assume that we only got the break/exception at the first thread.

    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selected_thread_id));

    uint32_t frameIndex = getThreadFrame(plugin, plugin->selected_thread_id);

    printf("frameIndex %d\n", frameIndex);

    lldb::SBFrame frame(thread.GetFrameAtIndex(frameIndex));

    lldb::SBSymbolContext context(frame.GetSymbolContext(lldb::eSymbolContextEverything));
    lldb::SBLineEntry entry(context.GetLineEntry());
    uint32_t line = entry.GetLine();
    entry.GetFileSpec().GetPath(filename, sizeof(filename));

    // TODO: Handle binary address also
    flatbuffers::FlatBufferBuilder builder(1024);
    auto name = builder.CreateString(filename);

    printf("exception reply %s:%d\n", filename, line);

    ExceptionLocationReplyBuilder reply(builder);
    reply.add_filename(name);
    reply.add_line(line);
    reply.add_address(0);

    PDMessage_end_msg(writer, reply, builder);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void reply_source_files(LLDBPlugin* plugin, PDWriter* writer) {
    std::vector<flatbuffers::Offset<flatbuffers::String>> filenames;
    flatbuffers::FlatBufferBuilder builder(8192);

    if (!plugin->has_valid_target) {
        return;
    }

    std::map<std::string, bool> dupe_lookup;

    const uint32_t module_count = plugin->target.GetNumModules();

    for (uint32_t im = 0; im < module_count; ++im) {
        lldb::SBModule mod(plugin->target.GetModuleAtIndex(im));

        const uint32_t compile_unit_count = mod.GetNumCompileUnits();

        for (uint32_t ic = 0; ic < compile_unit_count; ++ic) {
            lldb::SBCompileUnit compileUnit(mod.GetCompileUnitAtIndex(ic));

            const uint32_t support_file_count = compileUnit.GetNumSupportFiles();

            for (uint32_t is = 0; is < support_file_count; ++is) {
                char filename[4096];

                lldb::SBFileSpec fileSpec(compileUnit.GetSupportFileAtIndex(is));

                filename[0] = 0;

                fileSpec.GetPath(filename, sizeof(filename));

                if (filename[0] == 0) {
                    continue;
                }

                std::string name(filename);

                if (dupe_lookup.find(name) != dupe_lookup.end()) {
                    continue;
                }

                dupe_lookup[name] = true;

                filenames.push_back(builder.CreateString(filename));
            }
        }
    }

    auto t = builder.CreateVector<flatbuffers::Offset<flatbuffers::String>>(filenames);

    SourceFilesReplyBuilder reply(builder);
    reply.add_entries(t);

    PDMessage_end_msg(writer, reply, builder);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void basic_reply(LLDBPlugin* plugin, const BasicRequest* request, PDWriter* writer) {
    switch (request->id()) {
        case BasicRequestEnum_Callstack: reply_callstack(plugin, writer); break;
        case BasicRequestEnum_SourceFiles: reply_source_files(plugin, writer); break;
        default: break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static void setTty(LLDBPlugin* plugin, PDWriter* writer) {
    const int bufferSize = 4 * 1024;
    char buffer[bufferSize];

    size_t amountRead = plugin->process.GetSTDOUT(buffer, bufferSize);

    if (amountRead > 0) {
        PDWrite_event_begin(writer, PDEventType_SetTty);
        PDWrite_string(writer, "tty", buffer);
        PDWrite_event_end(writer);
    }
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void target_reply(LLDBPlugin* plugin, const FileTargetRequest* request, PDWriter* writer) {
    flatbuffers::FlatBufferBuilder builder(1024);

    const char* filename = request->path()->c_str();

    printf("lldb_plugin: target request %s\n", filename);

    plugin->target = plugin->debugger.CreateTarget(filename);

    if (!plugin->target.IsValid()) {
        char error_msg[4096];
        sprintf(error_msg, "LLDBPlugin: Unable to create valid target for: %s", filename);
        auto error_str = builder.CreateString(error_msg);
        TargetReplyBuilder reply(builder);
        reply.add_status(false);
        reply.add_error_message(error_str);
        PDMessage_end_msg(writer, reply, builder);
    } else {
        auto error_str = builder.CreateString("");

        for (Breakpoint& bp : plugin->breakpoints) {
            lldb::SBBreakpoint breakpoint = plugin->target.BreakpointCreateByLocation(bp.filename, (uint32_t)bp.line);

            if (!breakpoint.IsValid()) {
                // TODO: Send message back that this breakpoint could't be set
                printf("Unable to set breakpoint %s:%d\n", bp.filename, bp.line);
            }
        }
        printf("LLDBPlugin: Ok target\n");

        TargetReplyBuilder reply(builder);
        reply.add_error_message(error_str);
        reply.add_status(true);
        PDMessage_end_msg(writer, reply, builder);
    }

    //on_run(plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void locals_reply(LLDBPlugin* plugin, const LocalsRequest* request, PDWriter* writer) {
    std::vector<flatbuffers::Offset<Variable>> locals;
    flatbuffers::FlatBufferBuilder builder(4096);

    // TODO: Support expanding locals here
    (void)request;

    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selected_thread_id));
    lldb::SBFrame frame = thread.GetSelectedFrame();

    lldb::SBValueList variables = frame.GetVariables(true, true, true, false);

    for (uint32_t i = 0, c = variables.GetSize(); i < c; ++i) {
        lldb::SBValue value = variables.GetValueAtIndex(i);
        uint64_t adr = value.GetAddress().GetFileAddress();
        bool exp = value.MightHaveChildren();
        const char* name = value.GetName();
        const char* vt = value.GetValue();
        const char* type = value.GetTypeName();

        locals.push_back(CreateVariableDirect(builder, name, vt, type, adr, exp));
    }

    auto t = builder.CreateVector<flatbuffers::Offset<Variable>>(locals);

    LocalsReplyBuilder reply(builder);
    reply.add_variables(t);

    PDMessage_end_msg(writer, reply, builder);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void toggle_breakpoint(LLDBPlugin* plugin, const FileLineBreakpoint* request, PDWriter* writer) {
    flatbuffers::FlatBufferBuilder builder(1024);

    // TODO: Handle add/delete
    const char* filename = request->filename()->c_str();
    int line = request->line();

    lldb::SBBreakpoint breakpoint = plugin->target.BreakpointCreateByLocation(filename, line);

    if (!breakpoint.IsValid()) {
        printf("adding breakpoints to breakpoint list %s:%d\n", filename, line);

        // Unable to set breakpoint as the target doesn't seem to be valid. This is the usual case
        // if we haven't actually started an executable yet. So we save them here for later and
        // then set them before launching the executable

        Breakpoint bp = {strdup(filename), (int)line};
        plugin->breakpoints.push_back(bp);

        return;
    }

    printf("Set breakpoint at %s:%d\n", filename, line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
static void setLocals(LLDBPlugin* plugin, PDWriter* writer) {
    lldb::SBThread thread(plugin->process.GetThreadByID(plugin->selected_thread_id));
    lldb::SBFrame frame = thread.GetSelectedFrame();

    lldb::SBValueList variables = frame.GetVariables(true, true, true, false);

    uint32_t count = variables.GetSize();

    if (count <= 0)
        return;

    PDWrite_event_begin(writer, PDEventType_SetLocals);
    PDWrite_array_begin(writer, "locals");

    for (uint32_t i = 0; i < count; ++i) {
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

static void setThreads(LLDBPlugin* plugin, PDWriter* writer) {
    uint32_t threadCount = plugin->process.GetNumThreads();

    if (threadCount == 0)
        return;

    PDWrite_event_begin(writer, PDEventType_SetThreads);
    PDWrite_array_begin(writer, "threads");

    for (uint32_t i = 0; i < threadCount; ++i) {
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

static void setBreakpoint(LLDBPlugin* plugin, PDReader* reader, PDWriter* writer) {
    const char* filename;
    uint32_t line;

    PDRead_find_string(reader, &filename, "filename", 0);
    PDRead_find_u32(reader, &line, "line", 0);

    // TODO: Handle failure here

    lldb::SBBreakpoint breakpoint = plugin->target.BreakpointCreateByLocation(filename, line);
    if (!breakpoint.IsValid()) {
        printf("adding breakpoints to breakpoint list %s:%d\n", filename, line);

        // Unable to set breakpoint as the target doesn't seem to be valid. This is the usual case
        // if we haven't actually started an executable yet. So we save them here for later and
        // then set them before launching the executable

        Breakpoint bp = {strdup(filename), (int)line};
        plugin->breakpoints.push_back(bp);

        return;
    }

    printf("Set breakpoint at %s:%d\n", filename, line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void event_action(LLDBPlugin* plugin, PDReader* reader) {
    uint32_t action = 0;

    printf("LLDBPlugin; %d\n", (PDRead_find_u32(reader, &action, "action", 0) & 0xff) >> 8);
    printf("LLDBPlugin: got action (from event) %d\n", action);

    do_action(plugin, (PDAction)action);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
*/
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
    "PDEventType_exception_location_reply",
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

/*
static void selectThread(LLDBPlugin* plugin, PDReader* reader, PDWriter* writer) {
    uint64_t threadId;

    PDRead_find_u64(reader, &threadId, "thread_id", 0);

    printf("trying te set thread %lu\n", threadId);

    if (plugin->selected_thread_id == threadId)
        return;

    printf("selecting thread %lu\n", threadId);

    plugin->selected_thread_id = threadId;

    setCallstack(plugin, writer);

    PDWrite_event_begin(writer, PDEventType_SelectFrame);
    PDWrite_u32(writer, "frame", getThreadFrame(plugin, threadId));
    PDWrite_event_end(writer);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void select_frame(LLDBPlugin* plugin, const FrameSelectRequest* request, PDWriter* writer) {
    plugin->frame_selection[plugin->selected_thread_id] = request->frame_index();
    exception_location_reply(plugin, writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void do_action(LLDBPlugin* plugin, PDAction action) {
    switch (action) {
        /*
        case PDAction_Stop:
            on_stop(plugin);
            break;
        case PDAction_Break:
            on_break(plugin);
            break;
        */
        case PDAction_Run:
            on_run(plugin);
            break;
        case PDAction_Step:
            on_step(plugin);
            break;
        /*
        case PDAction_StepOut:
            on_step_over(plugin);
            break;
        case PDAction_StepOver:
            on_step_over(plugin);
            break;
        */
        default:
            break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void process_events(LLDBPlugin* plugin, PDReader* reader, PDWriter* writer) {
    uint32_t event;
    void* data = nullptr;
    uint64_t size = 0;

    while ((event = PDRead_get_event(reader))) {
        // TODO: This wrapping is temporary right now
        PDRead_find_data(reader, &data, &size, "data", 0);
        const Message* msg = GetMessage(data);

        switch (msg->message_type()) {
            case MessageType_exception_location_request: {
                exception_location_reply(plugin, writer);
                break;
            }

            case MessageType_basic_request: {
                basic_reply(plugin, msg->message_as_basic_request(), writer);
                break;
            }

            case MessageType_locals_request: {
                locals_reply(plugin, msg->message_as_locals_request(), writer);
                break;
            }

            case MessageType_file_target_request: {
                target_reply(plugin, msg->message_as_file_target_request(), writer);
                break;
            }

            case MessageType_file_line_breakpoint_request: {
                toggle_breakpoint(plugin, msg->message_as_file_line_breakpoint_request(), writer);
                break;
            }

            case MessageType_frame_select_request: {
                select_frame(plugin, msg->message_as_frame_select_request(), writer);
                break;
            }

            default: break;
        }


        // printf("LLDBPlugin: %d Got event %s\n", event, eventTypes[event]);

        /*
        switch (event) {
            case PDEventType_GetExceptionLocation:
                exception_location_reply(plugin, writer);
                break;
                case PDEventType_GetCallstack:
                    setCallstack(plugin, writer);
                    break;
                case PDEventType_SetExecutable:
                    setExecutable(plugin, reader);
                    break;
                case PDEventType_SelectThread:
                    selectThread(plugin, reader, writer);
                    break;
                case PDEventType_SelectFrame:
                    select_frame(plugin, reader, writer);
                    break;
                case PDEventType_GetLocals:
                    setLocals(plugin, writer);
                    break;
                case PDEventType_GetThreads:
                    setThreads(plugin, writer);
                    break;
                case PDEventType_GetSourceFiles:
                    set_source_files(plugin, writer);
                    break;
                case PDEventType_SetBreakpoint:
                    setBreakpoint(plugin, reader, writer);
                    break;
                case PDEventType_Action:
                    event_action(plugin, reader);
                    break;
        }
        */
    }

    //setTty(plugin, writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void send_exception_state(LLDBPlugin* plugin, PDWriter* writer) {
    printf("sending exception state\n");
    // setCallstack(plugin, writer);
    exception_location_reply(plugin, writer);
    // setLocals(plugin, writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void update_lldb_event(LLDBPlugin* plugin, PDWriter* writer) {
    if (!plugin->process.IsValid()) {
        printf("process invalid\n");
        return;
    }

    lldb::SBEvent evt;

    plugin->listener.WaitForEvent(1, evt);
    lldb::StateType state = lldb::SBProcess::GetStateFromEvent(evt);

    printf("event = %s\n", lldb::SBDebugger::StateAsCString(state));

    if (lldb::SBProcess::GetRestartedFromEvent(evt)) {
        printf("lldb::SBProcess::GetRestartedFromEvent(evt)\n");
        return;
    }

    switch (state) {
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
        case lldb::eStateSuspended: {
            // call_test_step = true;
            // bool fatal = false;
            bool selected_thread = false;
            for (uint32_t thread_index = 0; thread_index < plugin->process.GetNumThreads(); thread_index++) {
                lldb::SBThread thread(plugin->process.GetThreadAtIndex((size_t)thread_index));
                lldb::SBFrame frame(thread.GetFrameAtIndex(0));
                bool select_thread = false;
                lldb::StopReason stop_reason = thread.GetStopReason();

                if (m_verbose)
                    printf("tid = 0x%lx pc = 0x%lx ", thread.GetThreadID(), frame.GetPC());

                printf("stop reason %d\n", stop_reason);

                switch (stop_reason) {
                    case lldb::eStopReasonNone: {
                        if (m_verbose)
                            printf("none\n");
                        break;
                    }

                    case lldb::eStopReasonTrace: {
                        select_thread = true;
                        plugin->state = PDDebugState_Trace;
                        if (m_verbose)
                            printf("trace\n");
                        break;
                    }

                    case lldb::eStopReasonPlanComplete:
                        select_thread = true;

                        send_exception_state(plugin, writer);

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

                    case lldb::eStopReasonException: {
                        select_thread = true;

                        printf("%d %d\n", plugin->state, PDDebugState_StopException);

                        if (plugin->state != PDDebugState_StopException) {
                            send_exception_state(plugin, writer);
                        }

                        plugin->state = PDDebugState_StopException;

                        if (m_verbose)
                            printf("exception\n");
                        // fatal = true;

                        break;
                    }

                    case lldb::eStopReasonBreakpoint: {
                        select_thread = true;

                        printf("lldb::eStopReasonBreakpoint\n");

                        if (plugin->state != PDDebugState_StopBreakpoint) {
                            selected_thread = plugin->process.SetSelectedThread(thread);
                            plugin->selected_thread_id = thread.GetThreadID();
                            send_exception_state(plugin, writer);
                        }

                        plugin->state = PDDebugState_StopBreakpoint;

                        if (m_verbose) {
                            printf("lldb::eStopReasonBreakpoint breakpoint id = %ld.%ld\n", thread.GetStopReasonDataAtIndex(0),
                                   thread.GetStopReasonDataAtIndex(1));
                        }

                        break;
                    }

                    case lldb::eStopReasonWatchpoint:
                        select_thread = true;
                        if (m_verbose)
                            printf("watchpoint id = %ld\n", thread.GetStopReasonDataAtIndex(0));
                        break;
                    case lldb::eStopReasonSignal:
                        select_thread = true;
                        if (m_verbose)
                            printf("signal %d\n", (int)thread.GetStopReasonDataAtIndex(0));

                        if (plugin->state != PDDebugState_StopException) {
                            selected_thread = plugin->process.SetSelectedThread(thread);
                            plugin->selected_thread_id = thread.GetThreadID();
                            send_exception_state(plugin, writer);
                        }

                        plugin->state = PDDebugState_StopException;

                        break;
                    default:
                        break;
                }

                if (select_thread && !selected_thread) {
                    selected_thread = plugin->process.SetSelectedThread(thread);
                    plugin->selected_thread_id = thread.GetThreadID();
                }
            }
        } break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data, PDAction action, PDReader* reader, PDWriter* writer) {
    LLDBPlugin* plugin = (LLDBPlugin*)user_data;

    process_events(plugin, reader, writer);

    do_action(plugin, action);

    if (plugin->state == PDDebugState_Running) {
        update_lldb_event(plugin, writer);
    }

    return plugin->state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin = {
    "LLDB", create_instance, destroy_instance, update,
    0,  // save_state
    0,  // load_state
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
