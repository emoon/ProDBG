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

/*! \fn int RegisterPlugin(const char* serviceName)
	\breif This function will be called to invoko the plugin
*/

int CreatePlugin(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin, void* reserved);

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
	void (*uiEvent)(void* userData, uint64_t id, uint64_t eventType);

} PDMemoryViewPlugin;

#ifdef _cplusplus
}
#endif

#endif

