/* ********** NOTICE : THIS IS JUST A DRAFT ***********************/

#ifndef _PRODBGAPI_H_
#define _PRODBGAPI_H_

#include <stdint.h>
#include <stdbool.h>
#include "PDReadWrite.h"

#ifdef _cplusplus
extern "C" {
#endif

struct bson;

/*! \fn void* ServiceFunc(const char* serviceName)
    Service Function. Provides services for the plugin to use.
    Example:
     ProDBGUI* ui = serviceFunc(PRODBG_UI_SERVICE);
     ProDBServerInfo* serverInfo = serviceFunc(PRODBG_SERVERINFO_SERVICE);
     It's ok for the plugin to hold a pointer to the requested service during its life time.
    \param serviceName The name of the requested service. It's *highly* recommended to use the defines for the wanted service.
*/

typedef void* ServiceFunc(const char* serviceName);
typedef void RegisterPlugin(int type, void* data);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
    PD_API_VERSION = 1
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDAction
{
    PDAction_none,
    PDAction_break,
    PDAction_run,
    PDAction_step,
    PDAction_stepOut,
    PDAction_stepOver,
    PDAction_custom = 0x1000
} PDAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDEventType
{
    PDEventType_getLocals,
    PDEventType_setLocals,
    PDEventType_getCallStack,
    PDEventType_setCallStack,
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

    PDEventType_setBreakpointAddress,
    PDEventType_getBreakpointAddress,
    PDEventType_setBreakpointSourceLine,
    PDEventType_geBreakpointSourceLine,
    PDEventType_setExecutable,
    PDEventType_attachToProcess,
    PDEventType_attachToRemoteSession,

	/// Custom events. Here you can have your own events. Note that they must start with PDEventType_custom  and up
    PDEventType_custom = 0x1000

} PDEventType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDBackendPlugin
{
    int version;
    const char* name;

    // Create and destroy instance of the plugin
    
    void* (*createInstance)(ServiceFunc* serviceFunc);
    void (*destroyInstance)(void* userData);

	// Writer functions used for writing data back to the host
	PDReader reader;

	// Writer functions used for writing data back to the host
	PDWriter writer;

    // Updates and Returns the current state of the plugin.
    void (*update)(void* userData, PDAction action, PDReader* inEvents, PDWriter* outEvents);

} PDBackendPlugin;

#ifdef _cplusplus
}
#endif

#endif

