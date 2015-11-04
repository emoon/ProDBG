#ifndef _PRODBGAPI_BACKEND_H_
#define _PRODBGAPI_BACKEND_H_ 

#include "pd_common.h"
#include "pd_readwrite.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PDMenu;

#define PD_BACKEND_API_VERSION "ProDBG Backend 1"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDAction {
    PDAction_None,
    PDAction_Stop,
    PDAction_Break,
    PDAction_Run,
    PDAction_Step,
    PDAction_StepOut,
    PDAction_StepOver,
    PDAction_Custom = 0x1000
} PDAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDebugState {
    PDDebugState_NoTarget,
    PDDebugState_Running,
    PDDebugState_StopBreakpoint,
    PDDebugState_StopException,
    PDDebugState_Trace,
    PDDebugState_Count
} PDDebugState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDEventType {
    PDEventType_None,
    PDEventType_GetLocals,
    PDEventType_SetLocals,
    PDEventType_GetCallstack,
    PDEventType_SetCallstack,
    PDEventType_GetWatch,
    PDEventType_SetWatch,
    PDEventType_GetRegisters,
    PDEventType_SetRegisters,
    PDEventType_GetMemory,
    PDEventType_SetMemory,
    PDEventType_GetTty,
    PDEventType_SetTty,
    PDEventType_GetExceptionLocation,
    PDEventType_SetExceptionLocation,
    PDEventType_GetDisassembly,
    PDEventType_SetDisassembly,
    PDEventType_GetStatus,
    PDEventType_SetStatus,
    PDEventType_SetThreads,
    PDEventType_GetThreads,
    PDEventType_SelectThread,
    PDEventType_SelectFrame,
    PDEventType_GetSourceFiles,
    PDEventType_SetSourceFiles,

    PDEventType_SetSourceCodeFile,

    // setbreakpoint send a breakpoint to the backend with supplied id
    // Back end will reply if this worked correct with supplied ID

    PDEventType_SetBreakpoint,
    PDEventType_ReplyBreakpoint,

    PDEventType_DeleteBreakpoint,
    PDEventType_SetExecutable,
    PDEventType_Action,
    PDEventType_AttachToProcess,
    PDEventType_AttachToRemoteSession,

    PDEventType_ExecuteConsole,
    PDEventType_GetConsole,

	PDEventType_MenuEvent,

    // TODO: Somewhat temporary, need to figure this out

    PDEventType_ToggleBreakpointCurrentLine,

    // End of events

    PDEventType_End,

    /// Custom events. Here you can have your own events. Note that they must start with PDEventType_custom and up
    PDEventType_Custom = 0x1000

} PDEventType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDBackendPlugin {
    const char* name;

    void* (*create_instance)(ServiceFunc* service_func);
    void (*destroy_instance)(void* user_data);
    struct PDMenu* (*register_menu)();
    PDDebugState (*update)(void* user_data, PDAction action, PDReader* reader, PDWriter* writer);

} PDBackendPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Needed for exporting the entry point on Windows

#ifdef _WIN32
#define PD_EXPORT __declspec(dllexport)
#else
#define PD_EXPORT
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif

