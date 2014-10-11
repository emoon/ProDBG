#include "plugin_instance.h"
#include "ui/plugin.h"
#include "core/alloc.h"
#include <pd_view.h>
#include <string.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ViewPluginInstance* PluginInstance_createViewPlugin(PDViewPlugin* plugin)
{
    struct ViewPluginInstance* instance = (ViewPluginInstance*)alloc_zero(sizeof(struct ViewPluginInstance));

    void* userData = plugin->createInstance(&instance->ui, 0);
    instance->plugin = plugin;
    instance->userData = userData;

    PluginUI_init(instance);

    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

