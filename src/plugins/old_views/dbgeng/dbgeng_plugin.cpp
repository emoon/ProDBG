#ifdef _WIN32

#include <pd_backend.h>
#include <assert.h>
#include <stdio.h>
#include <Dbgeng.h>
#include <string>

struct DbgEngPlugin : public DebugBaseEventCallbacks {
    PDDebugState state = PDDebugState_NoTarget;
    bool hasValidTarget = false;

    std::string targetName;

    IDebugClient* debugClient = nullptr;
    IDebugControl* debugControl = nullptr;

    // IUnknown
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

    // IDebugEventCallbacks
    STDMETHOD(GetInterestMask)(THIS_
                               OUT PULONG Mask);
    STDMETHOD(Breakpoint)(THIS_
                          IN PDEBUG_BREAKPOINT Bp);
    STDMETHOD(Exception)(THIS_
                         IN PEXCEPTION_RECORD64 Exception, IN ULONG FirstChance);
    STDMETHOD(CreateProcess)(THIS_
                             IN ULONG64 ImageFileHandle,
                             IN ULONG64 Handle,
                             IN ULONG64 BaseOffset,
                             IN ULONG ModuleSize,
                             IN PCSTR ModuleName,
                             IN PCSTR ImageName,
                             IN ULONG CheckSum,
                             IN ULONG TimeDateStamp,
                             IN ULONG64 InitialThreadHandle,
                             IN ULONG64 ThreadDataOffset,
                             IN ULONG64 StartOffset);
    STDMETHOD(LoadModule)(THIS_
                          IN ULONG64 ImageFileHandle,
                          IN ULONG64 BaseOffset,
                          IN ULONG ModuleSize,
                          IN PCSTR ModuleName,
                          IN PCSTR ImageName,
                          IN ULONG CheckSum,
                          IN ULONG TimeDateStamp);
    STDMETHOD(SessionStatus)(THIS_
                             IN ULONG Status);
};

STDMETHODIMP_(ULONG) DbgEngPlugin::AddRef(THIS)
{
    return 1;
}

STDMETHODIMP_(ULONG) DbgEngPlugin::Release(THIS)
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DbgEngPlugin::GetInterestMask(THIS_
                                           OUT PULONG Mask) {
    *Mask =
        DEBUG_EVENT_BREAKPOINT |
        DEBUG_EVENT_EXCEPTION |
        DEBUG_EVENT_CREATE_PROCESS |
        DEBUG_EVENT_LOAD_MODULE |
        DEBUG_EVENT_SESSION_STATUS;
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DbgEngPlugin::Breakpoint(THIS_
                                      IN PDEBUG_BREAKPOINT Bp) {
    printf("DbgEngPlugin: Breakpoint\n");
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DbgEngPlugin::Exception(THIS_
                                     IN PEXCEPTION_RECORD64 Exception,
                                     IN ULONG               FirstChance) {
    printf("DbgEngPlugin: Exception\n");
    state = PDDebugState_StopException;
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DbgEngPlugin::CreateProcess(THIS_
                                         IN ULONG64 ImageFileHandle,
                                         IN ULONG64 Handle,
                                         IN ULONG64 BaseOffset,
                                         IN ULONG   ModuleSize,
                                         IN PCSTR   ModuleName,
                                         IN PCSTR   ImageName,
                                         IN ULONG   CheckSum,
                                         IN ULONG   TimeDateStamp,
                                         IN ULONG64 InitialThreadHandle,
                                         IN ULONG64 ThreadDataOffset,
                                         IN ULONG64 StartOffset) {
    printf("DbgEngPlugin: CreateProcess\n");
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DbgEngPlugin::LoadModule(THIS_
                                      IN ULONG64 ImageFileHandle,
                                      IN ULONG64 BaseOffset,
                                      IN ULONG   ModuleSize,
                                      IN PCSTR   ModuleName,
                                      IN PCSTR   ImageName,
                                      IN ULONG   CheckSum,
                                      IN ULONG   TimeDateStamp) {
    printf("DbgEngPlugin: LoadModule\n");
    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

STDMETHODIMP DbgEngPlugin::SessionStatus(THIS_
                                         IN ULONG Status) {
    switch (Status) {
        case DEBUG_SESSION_ACTIVE:
            printf("DbgEngPlugin: SessionStatus DEBUG_SESSION_ACTIVE\n");
            break;
        case DEBUG_SESSION_END_SESSION_ACTIVE_TERMINATE:
            printf("DbgEngPlugin: SessionStatus DEBUG_SESSION_END_SESSION_ACTIVE_TERMINATE\n");
            break;
        case DEBUG_SESSION_END_SESSION_ACTIVE_DETACH:
            printf("DbgEngPlugin: SessionStatus DEBUG_SESSION_END_SESSION_ACTIVE_DETACH\n");
            break;
        case DEBUG_SESSION_END_SESSION_PASSIVE:
            printf("DbgEngPlugin: SessionStatus DEBUG_SESSION_END_SESSION_PASSIVE\n");
            break;
        case DEBUG_SESSION_END:
            printf("DbgEngPlugin: SessionStatus DEBUG_SESSION_END\n");
            break;
        case DEBUG_SESSION_REBOOT:
            printf("DbgEngPlugin: SessionStatus DEBUG_SESSION_REBOOT\n");
            break;
        case DEBUG_SESSION_HIBERNATE:
            printf("DbgEngPlugin: SessionStatus DEBUG_SESSION_HIBERNATE\n");
            break;
        case DEBUG_SESSION_FAILURE:
            printf("DbgEngPlugin: SessionStatus DEBUG_SESSION_FAILURE\n");
            break;
    }

    return S_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateDbgEngEvent(DbgEngPlugin* plugin, PDWriter* writer) {
    HRESULT hr = plugin->debugControl->WaitForEvent(DEBUG_WAIT_DEFAULT, 100);

    if (hr == S_FALSE) {
        // WaitForEvent timeout occurred
        return;
    }

    // TODO: check and handle execution status
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void onRun(DbgEngPlugin* plugin) {
    printf("DbgEngPlugin: onRun\n");

    if (plugin->state == PDDebugState_NoTarget) {
        assert(!plugin->targetName.empty());

        HRESULT hr = plugin->debugClient->CreateProcess(0, PSTR(plugin->targetName.c_str()), DEBUG_ONLY_THIS_PROCESS);
        assert(SUCCEEDED(hr));

        if (!SUCCEEDED(hr)) {
            printf("Error: could not create process '%s'\n", plugin->targetName.c_str());
        }else {
            printf("Valid target %s\n", plugin->targetName.c_str());
        }

        plugin->state = PDDebugState_Running;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void onStop(DbgEngPlugin* plugin) {
    printf("DbgEngPlugin: onStop\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onBreak(DbgEngPlugin* plugin) {
    printf("DbgEngPlugin: onBreak\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStep(DbgEngPlugin* plugin) {
    printf("DbgEngPlugin: onStep\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStepOver(DbgEngPlugin* plugin) {
    printf("DbgEngPlugin: onStepOver\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void onStepOut(DbgEngPlugin* plugin) {
    printf("DbgEngPlugin: onStepOut\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void doAction(DbgEngPlugin* plugin, PDAction action) {
    printf("DbgEngPlugin: doAction\n");

    switch (action) {
        case PDAction_stop:
            onStop(plugin); break;
        case PDAction_break:
            onBreak(plugin); break;
        case PDAction_run:
            onRun(plugin); break;
        case PDAction_step:
            onStep(plugin); break;
        case PDAction_stepOut:
            onStepOut(plugin); break;
        case PDAction_stepOver:
            onStepOver(plugin); break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExceptionLocation(DbgEngPlugin* plugin, PDWriter* writer) {
    printf("DbgEngPlugin: setExceptionLocation\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setCallstack(DbgEngPlugin* plugin, PDWriter* writer) {
    const ULONG maxFrames = 1024;
    DEBUG_STACK_FRAME frames[maxFrames];
    ULONG frameSize = sizeof(frames[0]);
    ULONG framesFilled = 0;

    plugin->debugControl->GetStackTrace(0, 0, 0, frames, frameSize, &framesFilled);
    printf("DbgEngPlugin: setCallstack\n");

    if (framesFilled == 0)
        return;

    PDWrite_event_begin(writer, PDEventType_setCallstack);
    PDWrite_array_begin(writer, "callstack");

    for (ULONG i = 0; i < framesFilled; ++i) {
        const DEBUG_STACK_FRAME& frame = frames[i];

        PDWrite_array_entry_begin(writer);
        PDWrite_u64(writer, "address", frame.InstructionOffset);
        PDWrite_entry_end(writer);
    }

    PDWrite_array_end(writer);
    PDWrite_event_end(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setExecutable(DbgEngPlugin* plugin, PDReader* reader) {
    printf("DbgEngPlugin: setExecutable\n");

    const char* filename = 0;

    PDRead_find_string(reader, &filename, "filename", 0);

    if (!filename) {
        printf("Unable to find filename which is required when starting a LLDB debug session\n");
        return;
    }

    printf("found filename \"%s\"\n", filename);

    plugin->targetName = filename;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setLocals(DbgEngPlugin* plugin, PDWriter* writer) {
    printf("DbgEngPlugin: setLocals\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void setBreakpoint(DbgEngPlugin* plugin, PDReader* reader, PDWriter* writer) {
    printf("DbgEngPlugin: setBreakpoint\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void eventAction(DbgEngPlugin* plugin, PDReader* reader) {
    printf("DbgEngPlugin: eventAction\n");

    uint32_t action = 0;

    printf("DbgEngPlugin; %d\n", (PDRead_find_u32(reader, &action, "action", 0) & 0xff) >> 8);
    printf("DbgEngPlugin: got action (from event) %d\n", action);

    doAction(plugin, (PDAction)action);
}

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void processEvents(DbgEngPlugin* plugin, PDReader* reader, PDWriter* writer) {
    printf("DbgEngPlugin: processEvents\n");

    PDEventType event;

    while ((event = (PDEventType)PDRead_get_event(reader))) {
        printf("DbgEngPlugin: %d Got event %s\n", (int)event, eventTypes[event]);

        switch (event) {
            case PDEventType_getExceptionLocation:
                setExceptionLocation(plugin, writer); break;
            case PDEventType_getCallstack:
                setCallstack(plugin, writer); break;
            case PDEventType_setExecutable:
                setExecutable(plugin, reader); break;
            case PDEventType_getLocals:
                setLocals(plugin, writer); break;
            case PDEventType_setBreakpoint:
                setBreakpoint(plugin, reader, writer); break;
            case PDEventType_action:
                eventAction(plugin, reader); break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* createInstance(ServiceFunc* serviceFunc) {
    DbgEngPlugin* plugin = new DbgEngPlugin;

    HRESULT hr = DebugCreate(__uuidof(IDebugClient), (void**)&plugin->debugClient);
    assert(SUCCEEDED(hr));

    hr = plugin->debugClient->SetEventCallbacks(plugin);
    assert(SUCCEEDED(hr));

    hr = plugin->debugClient->QueryInterface(__uuidof(IDebugControl), (void**)&plugin->debugControl);
    assert(SUCCEEDED(hr));

    return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroyInstance(void* user_data) {
    DbgEngPlugin* plugin = reinterpret_cast<DbgEngPlugin*>(user_data);

    if (plugin->debugControl) {
        plugin->debugControl->Release();
        plugin->debugControl = nullptr;
    }

    if (plugin->debugClient) {
        plugin->debugClient->Release();
        plugin->debugClient = nullptr;
    }

    delete plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data, PDAction action, PDReader* reader, PDWriter* writer) {
    DbgEngPlugin* plugin = reinterpret_cast<DbgEngPlugin*>(user_data);

    processEvents(plugin, reader, writer);

    doAction(plugin, action);

    if (plugin->state == PDDebugState_Running) {
        updateDbgEngEvent(plugin, writer);
    }

    return plugin->state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin = {
    "Microsoft Debugger Engine",
    createInstance,
    destroyInstance,
    0,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

#endif
