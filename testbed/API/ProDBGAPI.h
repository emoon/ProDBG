/* ********** NOTICE : THIS IS JUST A DRAFT ***********************/

#ifndef _PRODBGAPI_H_
#define _PRODBGAPI_H_

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

//

typedef enum PDDebugAction
{
	PD_DEBUG_ACTION_BREAK,
	PD_DEBUG_ACTION_STEP,
	PD_DEBUG_ACTION_STEP_OVER,
	PD_DEBUG_ACTION_SET_CODE_BREAKPOINT

} PDDebugAction;

typedef enum PDLaunchAction
{
	PD_DEBUG_LAUNCH,
	PD_DEBUG_ATTATCH

} PDLaunchAction;

typedef struct PDDebugPlugin
{
	void* (*createInstance)(ServiceFunc* serviceFunc);
	void (*destroyInstance)(void* userData);
	
	bool (*start)(void* userData, PDLaunchAction action, void* launchData);
	void (*action)(void* userData, PDDebugAction action, void* actionData);

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

