#include "plugin_handler.h"
#include "log.h"
#include "core.h"
#include "core/alloc.h"
#include <pd_common.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <uv.h>
#include <stb.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData** s_plugins;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void registerPlugin(const char* type, void* plugin, void* privateData)
{
    PluginData* pluginData = (PluginData*)alloc_zero(sizeof(PluginData));

    // TODO: Verify that we don't add a plugin with the same plugin name in the same plugin

    pluginData->plugin = plugin;
    pluginData->type = type;
    pluginData->filename = (const char*)privateData;

    log_debug("Register plugin (type %s plugin %p filename %s)\n", pluginData->type, pluginData->plugin, pluginData->filename);

    stb_arr_push(s_plugins, pluginData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char* buildLoadingPath(const char* basePath, const char* plugin)
{
    char* output = 0;

    size_t baseLen = strlen(basePath);
    size_t pluginLen = strlen(plugin);

#ifdef PRODBG_MAC
    output = (char*)malloc(baseLen + pluginLen + 12); // + 12 for separator /lib.dylib + terminator
    sprintf(output, "%s/lib%s.dylib", basePath, plugin);
#elif PRODBG_WIN
    output = (char*)malloc(baseLen + pluginLen + 6); // + 5 for separator /.dll + terminator
    sprintf(output, "%s/lib%s.dylib", basePath, plugin);
#else
    output = (char*)malloc(baseLen + pluginLen + 5); // + 4 for separator \.so + terminator
    sprintf(filename, "%s/%s.so", basePath, plugin);
#endif

    return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin)
{
    uv_lib_t lib;
    void* function;
    void* (* initPlugin)(RegisterPlugin* registerPlugin, void* privateData);

    const char* filename = buildLoadingPath(basePath, plugin);

    if (uv_dlopen(filename, &lib) == -1)
    {
        // TODO: Show error message
        log_error("Unable to open %s error:", uv_dlerror(&lib))
        free((void*)filename);
        return false;
    }

    if (uv_dlsym(&lib, "InitPlugin", &function) == -1)
    {
        // TODO: Show error message
        log_error("Unable to find InitPlugin function in plugin %s\n", plugin);
        uv_dlclose(&lib);
        free((void*)filename);
        return false;
    }

    *(void**)(&initPlugin) = function;

    initPlugin(registerPlugin, (void*)filename);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData** PluginHandler_getPlugins(int* count)
{
    *count = stb_arr_len(s_plugins);
    return s_plugins;
}

