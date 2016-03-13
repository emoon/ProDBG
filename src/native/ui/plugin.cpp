#include "plugin.h"
//#include "core/alloc.h"
//#include "core/service.h"
//#include "core/plugin_handler.h"
#include "api/include/pd_view.h"
#include <stdio.h>
#include <stdarg.h>

PluginUI* g_pluginUI = 0;

static char s_statusText[4096];

/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ViewPluginInstance* PluginUI::createViewPlugin(PluginData* pluginData) {
    struct ViewPluginInstance* instance = (ViewPluginInstance*)alloc_zero(sizeof(struct ViewPluginInstance));

    PDViewPlugin* plugin = (PDViewPlugin*)pluginData->plugin;

    void* userData = plugin->create_instance(&instance->ui, Service_getService);
    instance->plugin = plugin;
    instance->userData = userData;
    instance->count = pluginData->count;

    init(instance);

    pluginData->count++;

    return instance;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginUI::setStatusText(const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    vsprintf(s_statusText, format, ap);
    setStatusTextNoFormat(s_statusText);
    va_end(ap);
}


