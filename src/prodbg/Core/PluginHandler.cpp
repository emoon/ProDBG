//#include "SharedObject.h"
#include "PluginHandler.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QLibrary>
#include <QString>
#include <assert.h>
#include <pd_common.h>
#include <stdio.h>
#include <stdlib.h>
#include "PluginUI/generated/c_api.h"
//#include "PluginUI/wrui.h"

class QWidget;

#define sizeof_array(t) (sizeof(t) / sizeof(t[0]))

extern struct PU* PU_create_instance(void* user_data, QWidget* parent);

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Plugin s_plugins[128];
static unsigned int s_pluginCount = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void registerPlugin(const char* type, void* data, void* privateData)
{
    Plugin plugin;

    plugin.data = data;
    plugin.type = type;

    printf("Register plugin (type %s data %p)\n", type, data);

    assert(s_pluginCount < sizeof_array(s_plugins));

    s_plugins[s_pluginCount++] = plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Search for plugin

QLibrary* findPlugin(const QString& plugin)
{
    QDir currentDir = QDir(QCoreApplication::applicationDirPath());
#ifdef __APPLE__
    QString pluginName = QString::fromUtf8("lib", 3) + plugin;
#else
    QString pluginName = plugin;
#endif

	do {
        QString path = currentDir.filePath(pluginName);
        QLibrary* lib = new QLibrary(path);

        if (lib->load()) {
            return lib;
        }

        delete lib;

    } while (currentDir.cdUp());

    return nullptr;
}

typedef void* (*InitPlugin)(RegisterPlugin* registerPlugin, void* privateData);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const QString& plugin)
{
    QLibrary* lib = findPlugin(plugin);

    if (!lib) {
        qDebug() << "Unable to find " << plugin;
        return false;
    }

    InitPlugin initPlugin = (InitPlugin)lib->resolve("InitPlugin");

    if (!initPlugin) {
        qDebug() << "Unable to find InitPlugin for plugin " << lib->fileName();
        delete lib;
        return false;
    }

    initPlugin(registerPlugin, 0);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Plugin* PluginHandler_getPlugins(int* count)
{
    *count = (int)s_pluginCount;
    return &s_plugins[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin* PluginHandler_findBackendPlugin(const char* name)
{
    for (unsigned int i = 0; i < s_pluginCount; ++i) {
        Plugin* plugin = &s_plugins[i];

        if (strcmp(plugin->type, "ProDBG Backend 1")) {
            continue;
        }

        PDPluginBase* base = (PDPluginBase*)plugin->data;

        if (!strcmp(base->name, name)) {
            return (PDBackendPlugin*)plugin->data;
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void* (*CreateUIPlugin)(PU* ui);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QWidget* PluginHandler_tempLoadUIPlugin(QWidget* parent, const QString& plugin) {
    QLibrary* lib = findPlugin(plugin);

    if (!lib) {
        qDebug() << "Unable to find " << plugin;
        return nullptr;
    }

    auto create_ui_plugin = (CreateUIPlugin)lib->resolve("init_plugin");

	PU* pu = PU_create_instance(0, parent);
    create_ui_plugin(pu);

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
