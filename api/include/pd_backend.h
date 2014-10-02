#ifndef _PRODBGAPI_H_
#define _PRODBGAPI_H_

#include "pd_common.h"
#include "pd_readwrite.h"

#ifdef _cplusplus
extern "C" {
#endif

#define PD_BACKEND_API_VERSION "ProDBG Backend 1"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDAction
{
    PDAction_none,
    PDAction_stop,
    PDAction_break,
    PDAction_run,
    PDAction_step,
    PDAction_stepOut,
    PDAction_stepOver,
    PDAction_custom = 0x1000
} PDAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDebugState
{
    PDDebugState_noTarget,
    PDDebugState_running,
    PDDebugState_stopBreakpoint,
    PDDebugState_stopException,
    PDDebugState_trace,
    PDDebugState_count
} PDDebugState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDEventType
{
    PDEventType_getLocals,
    PDEventType_setLocals,
    PDEventType_getCallstack,
    PDEventType_setCallstack,
    PDEventType_getWatch,
    PDEventType_setWatch,
    PDEventType_getRegisters,
    PDEventType_setRegisters,
    PDEventType_getMemory,
    PDEventType_setMemory,
    PDEventType_getTty,
    PDEventType_setTty,
    PDEventType_getExceptionLocation,
    PDEventType_setExceptionLocation,
    PDEventType_getDisassembly,
    PDEventType_setDisassembly,
	PDEventType_getStatus,
	PDEventType_setStatus,

    PDEventType_setBreakpoint,
    PDEventType_getBreakpoint,
    PDEventType_setExecutable,
    PDEventType_action,
    PDEventType_attachToProcess,
    PDEventType_attachToRemoteSession,

    /// Custom events. Here you can have your own events. Note that they must start with PDEventType_custom and up
    PDEventType_custom = 0x1000

} PDEventType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDBackendPlugin
{
    const char* version;
    const char* name;

    void* (*createInstance)(ServiceFunc* serviceFunc);
    void (*destroyInstance)(void* userData);

    // Updates and Returns the current state of the plugin.
    PDDebugState (*update)(void* userData, PDAction action, PDReader* inEvents, PDWriter* outEvents);

} PDBackendPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Needed for exporting the entry point on Windows

#ifdef _WIN32
#define PD_EXPORT __declspec(dllexport)
#else
#define PD_EXPORT
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _cplusplus
}
#endif

#endif

