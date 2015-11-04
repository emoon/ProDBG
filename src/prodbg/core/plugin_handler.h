#pragma once

// leave this includes here (added because of some bad dependencies when these changes)
#include "pd_view.h"
#include "pd_backend.h"
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PluginData {
    void* plugin;
    const char* type;
    const char* filename;
    const char* fullFilename;
    uint64_t lib;
    int count;
    int menuStart;
    int menuEnd;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginHandler_create(bool useShadowDir);
void PluginHandler_destroy();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Adds a path for that is used for plugin loading. This path is first base first load priority meaning that the
// path that gets added first has the highest priority

void PluginHandler_addSearchPath(const char* path);

bool PluginHandler_addPlugin(const char* basePath, const char* plugin);
void PluginHandler_unloadAllPlugins();

bool PluginHandler_unloadPlugin(PluginData* plugin);

PluginData* PluginHandler_reloadPlugin(PluginData* pluginData);

PluginData* PluginHandler_findPlugin(const char** paths, const char* pluginFile, const char* pluginName, bool load);
PluginData* PluginHandler_findPluginByFilename(const char* pluginFile);

PluginData* PluginHandler_getViewPluginData(void* plugin);
PluginData** PluginHandler_getViewPlugins(int* count);
PluginData** PluginHandler_getBackendPlugins(int* count);
PluginData* PluginHandler_getPluginData(void* plugin);


