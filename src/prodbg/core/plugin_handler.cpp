#include "plugin_handler.h"
#include "log.h"
#include "core.h"
#include "core/alloc.h"
#include <pd_common.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <foundation/fs.h>
#include <foundation/library.h>
#include <foundation/array.h>
#include <foundation/string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Move this to some general configuration about plugins and types

static const char* s_pluginTypes[] =
{
    "ProDBG View",
    "ProDBG Backend",
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    PRODBG_VIEW_PLUGIN,
    PRODBG_BACKEND_PLUGIN,
    PRODBG_PLUGIN_COUNT,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData** s_plugins[PRODBG_PLUGIN_COUNT];
static const char** s_searchPaths;
static bool s_useShadowDir = true;
static const char* s_shadowDirName = "shadow_plugins";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginHandler_create(bool shadowDirectory) {
    s_useShadowDir = shadowDirectory;

    if (!shadowDirectory)
        return;

    // Cleanup directory first

    if (fs_is_file(s_shadowDirName))
        fs_remove_file(s_shadowDirName);

    if (fs_is_directory(s_shadowDirName))
        fs_remove_directory(s_shadowDirName);

    fs_make_directory(s_shadowDirName);

    // Listen to changes in the current directry for code
    // TODO: Add this to settings so we can have more dirs here

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginHandler_destroy() {
    PluginHandler_unloadAllPlugins();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginHandler_addSearchPath(const char* path) {
    array_push(s_searchPaths, path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData* findPlugin(PluginData** plugins, const char* pluginFile, const char* pluginName) {
    int count = array_size(plugins);

    for (int i = 0; i < count; ++i) {
        PluginData* pluginData = plugins[i];
        PDPluginBase* base = (PDPluginBase*)pluginData->plugin;

        if (!strcmp(base->name, pluginName) && !strcmp(pluginData->filename, pluginFile))
            return pluginData;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData* findPluginAll(const char* pluginFile, const char* pluginName) {
    PluginData* plugin = 0;

    for (int i = 0; i < PRODBG_PLUGIN_COUNT; ++i) {
        if ((plugin = findPlugin(s_plugins[i], pluginFile, pluginName)))
            return plugin;
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData* PluginHandler_findPluginByFilename(const char* filename) {
    for (int i = 0; i < PRODBG_PLUGIN_COUNT; ++i) {
        int count = array_size(s_plugins[i]);

        for (int t = 0; t < count; ++t) {
            PluginData* pluginData = s_plugins[i][t];

            if (string_find_string(pluginData->fullFilename, filename, 0) != STRING_NPOS)
                return pluginData;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void removePlugin(PluginData* pluginData) {
    // Remove the plugin data

    for (int i = 0; i < PRODBG_PLUGIN_COUNT; ++i) {
        int count = array_size(s_plugins[i]);

        for (int t = 0; t < count; ++t) {
            PluginData* plugin = s_plugins[i][t];

            if (pluginData != plugin)
                continue;

            printf("removed plugin %s\n", plugin->fullFilename);

            library_unload(plugin->lib);
            free((void*)plugin->fullFilename);
            free(plugin);
            array_erase(s_plugins[i], t);

            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData* PluginHandler_reloadPlugin(PluginData* pluginData) {
    const char* filename = pluginData->filename;
    const char* fullName = string_clone(pluginData->fullFilename);

    printf("removing plugin...\n");

    removePlugin(pluginData);

    printf("adding plugin...%s\n", filename);

    PluginHandler_addPlugin(OBJECT_DIR, filename);

    printf("finding plugin\n");

    printf("trying to find %s\n", fullName);

    PluginData* newPluginData = PluginHandler_findPluginByFilename(fullName);

    printf("found plugin %p\n", newPluginData);

    return newPluginData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PluginPrivateData {
    const char* name;
    const char* fullFilename;
    object_t lib;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void registerPlugin(const char* type, void* plugin, void* private_data) {
    PluginPrivateData* priv_data= (PluginPrivateData*)private_data;

    const char* filename = priv_data->name;

    for (int i = 0; i < PRODBG_PLUGIN_COUNT; ++i) {
        if (strstr(type, s_pluginTypes[i])) {
            if (findPlugin(s_plugins[i], filename, ((PDPluginBase*)plugin)->name))
                return;

            // TODO: Verify that we don't add a plugin with the same plugin name in the same plugin

            PluginData* pluginData = (PluginData*)alloc_zero(sizeof(PluginData));
            pluginData->plugin = plugin;
            pluginData->type = type;
            pluginData->filename = filename;
            pluginData->fullFilename = priv_data->fullFilename;
            pluginData->lib = priv_data->lib;

            return (void)array_push(s_plugins[i], pluginData);
        }
    }

    pd_error("Unknown pluginType %s - %s", type, ((PDPluginBase*)plugin)->name);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static char* buildLoadingPath(const char* basePath, const char* plugin) {
    char* output = 0;

    size_t baseLen = strlen(basePath);
    size_t pluginLen = strlen(plugin);

#ifdef PRODBG_MAC
    output = (char*)malloc(baseLen + pluginLen + 12); // + 12 for separator /lib.dylib + terminator
    sprintf(output, "%s/lib%s.dylib", basePath, plugin);
#elif PRODBG_WIN
    output = (char*)malloc(baseLen + pluginLen + 6); // + 5 for separator /.dll + terminator
    sprintf(output, "%s\\%s.dll", basePath, plugin);
#else
    output = (char*)malloc(baseLen + pluginLen + 12); // + 4 for separator \.so + terminator
    sprintf(output, "%s/lib%s.so", basePath, plugin);
#endif

    return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin) {
    void* (* initPlugin)(RegisterPlugin* registerPlugin, void* private_data);
    struct PluginPrivateData data;
    object_t lib = 0;

    const char* filename = 0;
    void* function;

    if (!basePath || !plugin)
        goto error;

    filename = buildLoadingPath(basePath, plugin);

    lib = library_load(filename);

    if (!library_valid(lib)) {
        // TODO: Show error message
        pd_error("Unable to open %s\n", filename);
        goto error;
    }

    if (!(function = library_symbol(lib, "InitPlugin"))) {
        // TODO: Show error message
        pd_error("Unable to find InitPlugin function in plugin %s\n", filename);
        goto error;
    }

    *(void**)(&initPlugin) = function;

    data.name = plugin;
    data.lib = lib;
    data.fullFilename = filename;

    initPlugin(registerPlugin, (void*)&data);

    return true;

    error:

    if (library_valid(lib))
        library_unload(lib);

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginHandler_unloadAllPlugins() {
    // TODO: Actually unload everything

    for (int i = 0; i < PRODBG_PLUGIN_COUNT; ++i) {
        int count = array_size(s_plugins[i]);

        for (int t = 0; t < count; ++t) {
            PluginData* plugin = s_plugins[i][t];
            library_unload(plugin->lib);
            free((void*)plugin->fullFilename);
            free(plugin);
        }

        array_clear(s_plugins[i]);
    }

    //for (int i = 0; i < PRODBG_PLUGIN_COUNT; ++i)
    //	free(s_plugins[i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData* PluginHandler_findPlugin(const char** paths, const char* pluginFile, const char* pluginName, bool load) {
    PluginData* pluginData;

    // TODO: Support paths
    (void)paths;

    // If not found and not !load (that is we will not try to load it)

    if ((pluginData = findPluginAll(pluginFile, pluginName)))
        return pluginData;

    if (!load)
        return 0;

    // TODO: Support base paths

    if (!PluginHandler_addPlugin(OBJECT_DIR, pluginFile))
        return 0;

    if ((pluginData = findPluginAll(pluginFile, pluginName)))
        return pluginData;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData** PluginHandler_getBackendPlugins(int* count) {
    *count = array_size(s_plugins[PRODBG_BACKEND_PLUGIN]);
    return s_plugins[PRODBG_BACKEND_PLUGIN];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData** PluginHandler_getViewPlugins(int* count) {
    *count = array_size(s_plugins[PRODBG_VIEW_PLUGIN]);
    return s_plugins[PRODBG_VIEW_PLUGIN];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PluginData* getPluginData(PluginData** plugins, void* plugin) {
    int count = array_size(plugins);

    for (int i = 0; i < count; ++i) {
        if (plugins[i]->plugin == plugin)
            return plugins[i];
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PluginData* PluginHandler_getPluginData(void* plugin) {
    PluginData* data = 0;

    for (int i = 0; i < PRODBG_PLUGIN_COUNT; ++i) {
        if ((data = getPluginData(s_plugins[i], plugin)))
            return data;
    }

    return 0;
}


