#pragma once

class QString;
class QWidget;

struct PDBackendPlugin;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Plugin
{
    void* data;
    const char* type;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const QString& pluginName);
Plugin* PluginHandler_getPlugins(int* count);

PDBackendPlugin* PluginHandler_findBackendPlugin(const char* name);
QWidget* PluginHandler_tempLoadUIPlugin(QWidget* parent, const QString& plugin);

}
