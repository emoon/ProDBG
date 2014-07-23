#include "PluginHandler.h"
#include "io/SharedObject.h"
#include "Log.h"
#include <ProDBGAPI.h>
#include <stdio.h>
#include <stdlib.h>

namespace prodbg
{

static Plugin s_plugins[128];
static int s_pluginCount = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void registerPlugin(int type, void* data)
{
    Plugin plugin;

    plugin.data = data;
    plugin.type = type;

    log_debug("Register plugin (type %d data %p)\n", type, data);

    assert(s_pluginCount < sizeof_array(s_plugins));

    s_plugins[s_pluginCount++] = plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* plugin)
{
    Handle handle;
    void* (* initPlugin)(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin);

    printf("Trying to load %s\n", plugin);

    if (!(handle = SharedObject_open(plugin)))
    {
        // TODO: Implment proper logging and output

        log_error("Unable to open plugin %s\n", plugin);
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
    *count = s_pluginCount;
    return &s_plugins[0];
}

}

