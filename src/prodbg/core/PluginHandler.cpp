#include "PluginHandler.h"
#include "io/SharedObject.h"
#include "Log.h"
#include "Core.h"
#include <PDCommon.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Plugin
{
	void* data;
	const char* type;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Plugin s_plugins[128];
static unsigned int s_pluginCount = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void registerPlugin(const char* type, void* data)
{
    Plugin plugin;

    plugin.data = data;
    plugin.type = type;

    log_debug("Register plugin (type %s data %p)\n", type, data);

    assert(s_pluginCount < sizeof_array(s_plugins));

    s_plugins[s_pluginCount++] = plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin)
{
    Handle handle;
    char filepath[4096];
    void* (*initPlugin)(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin);

#ifdef PRODBG_MAC
	sprintf(filepath, "%s/lib%s.dylib", basePath, plugin);
#elif PRODBG_WIN
	sprintf(filepath, "%s/%s.dll", basePath, plugin);
#else
	sprintf(filepath, "%s/%s.so", basePath, plugin);
#endif

    printf("Trying to load %s\n", filepath);

    if (!(handle = SharedObject_open(filepath)))
    {
        // TODO: Implment proper logging and output

        log_error("Unable to open plugin %s\n", filepath);
        return false;
    }

    void* function = SharedObject_getSym(handle, "InitPlugin");

    if (!function)
    {
        log_error("Unable to find InitPlugin function in plugin %s\n", plugin);
        SharedObject_close(handle);
        return false;
    }

    *(void**)(&initPlugin) = function;

    initPlugin(1, 0, registerPlugin);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Plugin* PluginHandler_getPlugins(int* count)
{
    *count = (int)s_pluginCount;
    return &s_plugins[0];
}

}

