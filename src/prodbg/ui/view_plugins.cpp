#include "view_plugins.h"

#include <pd_common.h>
#include <vector>
#include "../core/shared_object.h"
#include "../core/logging.h"
#include "api/include/pd_memory_view.h"
#include "api/include/pd_view.h"

#ifndef _WIN32
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

#if defined(PRODBG_NIX)
#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#endif

#if defined(PRODBG_LINUX)
#define SO_PREFIX "lib"
#define SO_SUFFIX ".so"
#elif defined(PRODBG_MAC)
#define SO_PREFIX "lib"
#define SO_SUFFIX ".dylib"
#elif defined(PRODBG_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define SO_PREFIX ""
#define SO_SUFFIX ".dll"
#else
#error unsupported platform
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<prodbg::MemoryView*> s_memory_view_plugins;
static std::vector<prodbg::View*> s_view_plugins;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegPlugins : public prodbg::PluginRegister {
    void register_memory_view(prodbg::MemoryView* view) {
        s_memory_view_plugins.push_back(view);
    }

    void register_view(prodbg::MemoryView* view) {
        s_view_plugins.push_back(view);
    }
} s_reg_plugins;

typedef void (*InitPlugin)(prodbg::PluginRegister* reg);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewPlugins::add_plugin(const QString& plugin_dir) {
    QDir dir(plugin_dir);
    int plugins_found = 0;

#if defined(Q_OS_WIN)
    QString shared_extension = QStringLiteral(".dll");
#elif defined(Q_OS_MAC)
    QString shared_extension = QStringLiteral(".dylib");
#else
    QString shared_extension = QStringLiteral(".so");
#endif

    const QStringList entries = dir.entryList(QDir::Files);

    for (const QString& filename : entries) {
        if (!filename.endsWith(&shared_extension, Qt::CaseSensitive)) {
            continue;
        }

        QLibrary* lib = new QLibrary(dir.absoluteFilePath(filename));

        if (!lib->load()) {
            delete lib;
            continue;
        }

        InitPlugin init_plugin = (InitPlugin)lib->resolve("pd_init_plugin");

        if (!init_plugin) {
            QByteArray fname = filename.toUtf8();
            log_warn("Unable to find \"pd_init_plugin\" in plugin %s\n", fname.constData());
            delete lib;
        }

        //RegPlugins reg;
        init_plugin(&s_reg_plugins);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

prodbg::MemoryView* ViewPlugins::find_plugin(const QString& name) {
    for (const auto& plugin : s_plugins) {
        if (plugin->name() == name) {
            return plugin;
        }
    }

    return nullptr;
}

