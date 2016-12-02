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

#define sizeof_array(t) (sizeof(t) / sizeof(t[0]))

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

    while (currentDir.cdUp()) {
        QString path = currentDir.filePath(pluginName);
        QLibrary* lib = new QLibrary(path);

        if (lib->load()) {
            return lib;
        }

        delete lib;
    }

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
}
