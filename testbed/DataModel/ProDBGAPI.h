/* ********** NOTICE : THIS IS JUST A DRAFT ***********************/

#ifndef _PRODBGAPI_H_
#define _PRODBGAPI_H_

#include <stdint.h>

#ifdef _cplusplus
extern "C" {
#endif

/*! \fn void* ServiceFunc(const char* serviceName)
    \breif Service Function. Provides services for the plugin to use.
    Example:
     ProDBGUI* ui = serviceFunc(PRODBG_UI_SERVICE);
     ProDBServerInfo* serverInfo = serviceFunc(PRODBG_SERVERINFO_SERVICE);
     It's ok for the plugin to hold a pointer to the requested service during its life time.
    \param serviceName The name of the requested service. It's *highly* recommended to use the defines for the wanted service.
*/

typedef void* ServiceFunc(const char* serviceName);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    PD_API_VERSION = 1,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDebugState
{
    PDDebugState_noTarget,         // nothing is running 
    PDDebugState_running,          // target is being executed 
    PDDebugState_paused,           // exception, breakpoint, etc 
    PDDebugState_stepping,         // code is currently being stepped/traced/etc 
    PDDebugState_custom = 0x1000,  // code is currently being stepped/traced/etc 
} PDDebugState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDAction
{
    PDAction_none,
    PDAction_break,
    PDAction_run,
    PDAction_step,
    PDAction_stepOut,
    PDAction_stepStepOver,
    PDAction_custom = 0x1000,
} PDAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDEventID
{
    PDBEventID_locals,
    PDBEventID_callStack,
    PDBEventID_watch,
    PDBEventID_registers,
    PDBEventID_memory,
    PDBEventID_custom = 0x1000,

} PDEventID;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDBackendPlugin
{
    int version,
    const char* name;

    // Create and destroy instance of the plugin
    
    void* (*createInstance)(ServiceFunc* serviceFunc);
    void (*destroyInstance)(void* userData);

    // Updates and Returns the current state of the plugin.

    PDDebugState (*update)(void* userData);

    // Various actions can be send to the plugin. like
    // Break, continue, exit, detach etc.

    int (*action)(void* userData, PDAction action);

    // Get some data state  
    void (*getState)(void* userData, PDEventID eventId, SerializeFuncs* serialize);

    // set some data state  
    void (*setState)(void* userData, PDEventID eventId, SerializeFuncs* serialize);

} PDBackendPlugin;

#ifdef _cplusplus
}
#endif

#endif

