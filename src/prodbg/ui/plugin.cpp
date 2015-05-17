#include "plugin.h"
#include "core/alloc.h"
#include "core/plugin_handler.h"
#include "api/include/pd_view.h"

PluginUI* g_pluginUI = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ViewPluginInstance* PluginUI::createViewPlugin(PluginData* pluginData)
{
    struct ViewPluginInstance* instance = (ViewPluginInstance*)alloc_zero(sizeof(struct ViewPluginInstance));

    PDViewPlugin* plugin = (PDViewPlugin*)pluginData->plugin;

    void* userData = plugin->createInstance(&instance->ui, 0);
    instance->plugin = plugin;
    instance->userData = userData;
    instance->count = pluginData->count;

    init(instance);

    pluginData->count++;

    return instance;
}

