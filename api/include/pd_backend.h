#pragma once

#include "pd_common.h"
#include "pd_message_readwrite.h"

#ifdef __cplusplus
extern "C" {
#endif

struct PDSaveState;
struct PDLoadState;

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

typedef struct PDBackendPlugin {
    const char* name;

    void* (*create_instance)(ServiceFunc* service_func);
    void (*destroy_instance)(void* user_data);

    PDDebugState (*update)(void* user_data, PDAction action, PDReadMessage* reader, PDWriteMessage* writer);

    // save/load state
    int (*save_state)(void* user_data, struct PDSaveState* save_state);
    int (*load_state)(void* user_data, struct PDLoadState* load_state);

} PDBackendPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#define PD_EXPORT __declspec(dllexport)
#else
#define PD_EXPORT
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif
