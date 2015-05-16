#include "plugin_instance.h"
#include "core/plugin_handler.h"
#include "ui/bgfx/plugin.h"
#include "core/alloc.h"
#include <pd_view.h>
#include <string.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ViewPluginInstance* PluginInstance_createViewPlugin(PluginData* pluginData)
{
    struct ViewPluginInstance* instance = (ViewPluginInstance*)alloc_zero(sizeof(struct ViewPluginInstance));

    PDViewPlugin* plugin = (PDViewPlugin*)pluginData->plugin;

    void* userData = plugin->createInstance(&instance->ui, 0);
    instance->plugin = plugin;
    instance->userData = userData;
    instance->count = pluginData->count;

    PluginUI_init(instance);

    pluginData->count++;

    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

