#include "plugin_instance.h"
#include "core/alloc.h"
#include <pd_view.h>
#include <string.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ViewPluginInstance* PluginInstance_createViewPlugin()
{
    struct ViewPluginInstance* instance = (ViewPluginInstance*)alloc_zero(sizeof(struct ViewPluginInstance));
    return instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginInstance_init(struct ViewPluginInstance* instance, PDViewPlugin* plugin)
{
    void* userData = plugin->createInstance(&instance->ui, 0);
    instance->plugin = plugin;
    instance->userData = userData;

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

