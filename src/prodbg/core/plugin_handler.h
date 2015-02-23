#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PluginData
{
    void* plugin;
    const char* type;
    const char* filename;
    int count;
    int menuStart;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin);
void PluginHandler_addStaticPlugin(PluginData* pluginData);
void PluginHandler_unloadAllPlugins();

//bool PluginHandler_unloadPlugin(PluginData* plugin);
//bool PluginHandler_unloadAllPlugins();

PluginData* PluginHandler_findPlugin(const char** paths, const char* pluginFile, const char* pluginName, bool load);

PluginData* PluginHandler_getPluginData(void* plugin);
PluginData** PluginHandler_getPlugins(int* count);


