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
    sprintf(output, "%s/%s.so", basePath, plugin);
#endif

    return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin)
{
    uv_lib_t lib;
    void* function;
    void* (* initPlugin)(RegisterPlugin* registerPlugin, void* privateData);

    if (!basePath || !plugin)
        return false;

    const char* filename = buildLoadingPath(basePath, plugin);

    if (uv_dlopen(filename, &lib) == -1)
    {
        // TODO: Show error message
        log_error("Unable to open %s error:\n", uv_dlerror(&lib))
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

    initPlugin(registerPlugin, (void*)plugin);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData* findPlugin(const char* pluginFile, const char* pluginName)
{
    int count = stb_arr_len(s_plugins);

    for (int i = 0; i < count; ++i)
    {
        PluginData* pluginData = s_plugins[i];
        PDPluginBase* base = (PDPluginBase*)pluginData->plugin;

        if (!strcmp(base->name, pluginName) && !strcmp(pluginData->filename, pluginFile))
            return pluginData;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginHandler_unloadAllPlugins()
{
    // TODO: Actually unload everything
    stb_arr_setlen(s_plugins, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData* PluginHandler_findPlugin(const char** paths, const char* pluginFile, const char* pluginName, bool load)
{
    PluginData* pluginData;

    // TODO: Support paths
    (void)paths;

    // If not found and not !load (that is we will not try to load it)

    if ((pluginData = findPlugin(pluginFile, pluginName)))
        return pluginData;

    if (!load)
        return 0;

    // TODO: Support base paths

    if (!PluginHandler_addPlugin(OBJECT_DIR, pluginFile))
        return 0;

    if ((pluginData = findPlugin(pluginFile, pluginName)))
        return pluginData;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginHandler_addStaticPlugin(PluginData* pluginData)
{
    stb_arr_push(s_plugins, pluginData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData** PluginHandler_getPlugins(int* count)
{
    *count = stb_arr_len(s_plugins);
    return s_plugins;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData* PluginHandler_getPluginData(void* plugin)
{
    int count = stb_arr_len(s_plugins);

    for (int i = 0; i < count; ++i)
    {
        if (s_plugins[i]->plugin == plugin)
            return s_plugins[i];
    }

    return 0;
}


