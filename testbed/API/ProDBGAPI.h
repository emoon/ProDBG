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
typedef void RegisterPlugin(int type, void* data);

/*! \fn int RegisterPlugin(const char* serviceName)
	\breif This function will be called to invoko the plugin
*/

//int CreatePlugin(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin, void* reserved);

/*
 *
 * Layout of a MemoryViewPlugin (used for testing
 *
 */

typedef struct PDMemoryViewPlugin
{
	void* (*createInstance)(void* parentWindow, ServiceFunc* serviceFunc);
	void (*destroyInstance)(void* userData);
	void (*displayMemory)(void* userData, void* memory);
	void (*uiEvent)(void* userData, int id, int eventType);

} PDMemoryViewPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDebugAction
{
	PD_DEBUG_ACTION_BREAK,
	PD_DEBUG_ACTION_STEP,
	PD_DEBUG_ACTION_STEP_OVER,
	PD_DEBUG_ACTION_CONTINUE,
	PD_DEBUG_ACTION_SET_CODE_BREAKPOINT,
	PD_DEBUG_ACTION_NONE

} PDDebugAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDebugState 
{
	PDDebugState_default,
	PDDebugState_noTarget,
	PDDebugState_breakpoint,
	PDDebugState_breakpointFileLine,
	PDDebugState_exception,
	PDDebugState_custom

} PDDebugState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Used when we have a break/exception on a certain line

typedef struct PDDebugStateFileLine
{
	char filename[4096];
	int line;

} PDDebugStateFileLine;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDLaunchAction
{
	PD_DEBUG_LAUNCH,
	PD_DEBUG_ATTATCH

} PDLaunchAction;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDBreakpointType
{
	PDBreakpointType_FileLine,
	PDBreakpointType_watchPoint,
	PDBreakpointType_address,
	PDBreakpointType_custom

} PDBreakpointType;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDBreakpointFileLine
{
	const char* filename;
	int line;
	int id;
} PDBreakpointFileLine;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDCallStack
{
	uint64_t address;
	char moduleName[1024];
	char fileLine[2048];
} PDCallstack;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDLocals
{
	char address[32];
	char value[32];
	char type[4096];
	char name[4096]; 

} PDLocals;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDDebugPlugin
{
	void* (*createInstance)(ServiceFunc* serviceFunc);
	void (*destroyInstance)(void* userData);

	bool (*start)(void* userData, PDLaunchAction action, void* launchData, PDBreakpointFileLine* breakpoints, int bpCount);
	void (*action)(void* userData, PDDebugAction action, void* actionData);

	PDDebugState (*getState)(void* userData, void** data);
	int (*addBreakpoint)(void* userData, PDBreakpointType type, void* breakpointData);
	void (*removeBreakpoint)(void* userData, int id); 
	
	void (*getCallStack)(void* userData, PDCallStack* callStack, int* maxEntries); 
	void (*getLocals)(void* userData, PDLocals* local, int* maxEntries); 

} PDDebugPlugin;

typedef enum ProDBGPluginType
{
	PD_PTYPE_DEBUGGER = 1
} ProDBGPluginType;

// 

#ifdef _cplusplus
}
#endif

#endif

