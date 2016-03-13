#ifndef _PDCOMMON_H_
#define _PDCOMMON_H_

#ifdef _cplusplus
extern "C" {
#endif

/*! \fn void* ServiceFunc(const char* serviceName)
    Service Function. Provides services for the plugin to use.
    Example:
     ProDBGUI* ui = serviceFunc(PRODBG_UI_SERVICE);
     ProDBServerInfo* serverInfo = serviceFunc(PRODBG_SERVERINFO_SERVICE);
     It's ok for the plugin to hold a pointer to the requested service during its life time.
    \param serviceName The name of the requested service. It's *highly* recommended to use the defines for the wanted service.
 */
typedef void* ServiceFunc(const char* service_name);
typedef void RegisterPlugin(const char* type, void* data, void* private_data);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDPluginBase {
    const char* name;
};

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


