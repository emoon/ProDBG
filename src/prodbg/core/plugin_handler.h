#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PluginData
{
    void* plugin;
    const char* type;
    const char* filename;
    int count;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin);
//bool PluginHandler_unloadPlugin(PluginData* plugin);
//bool PluginHandler_unloadAllPlugins();


PluginData* PluginHandler_getPluginData(void* plugin);
PluginData** PluginHandler_getPlugins(int* count);


