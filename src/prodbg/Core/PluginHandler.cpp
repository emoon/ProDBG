#include "PluginHandler.h"
#include "SharedObject.h"
#include <pd_common.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define sizeof_array(t) (sizeof(t) / sizeof(t[0]))

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Plugin s_plugins[128];
static unsigned int s_pluginCount = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void registerPlugin(const char* type, void* data, void* privateData)
{
    Plugin plugin;

    plugin.data = data;
    plugin.type = type;

    printf("Register plugin (type %s data %p)\n", type, data);

    assert(s_pluginCount < sizeof_array(s_plugins));

    s_plugins[s_pluginCount++] = plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin)
{
    Handle handle;
    void* (*initPlugin)(RegisterPlugin* registerPlugin, void* privateData);

    if (!(handle = SharedObject_open(basePath, plugin)))
        return false;

    void* function = SharedObject_getSym(handle, "InitPlugin");

    if (!function)
    {
        printf("Unable to find InitPlugin function in plugin %s\n", plugin);
        SharedObject_close(handle);
        return false;
    }

    *(void**)(&initPlugin) = function;

    initPlugin(registerPlugin, 0);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Plugin* PluginHandler_getPlugins(int* count)
{
    *count = (int)s_pluginCount;
    return &s_plugins[0];
}

}

