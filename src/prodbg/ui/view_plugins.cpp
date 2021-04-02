#include "view_plugins.h"

#include <pd_common.h>
#include <pd_ui_register_plugin.h>
#include <vector>
#include "../core/shared_object.h"
#include "../core/logging.h"
#include "api/include/pd_memory_view.h"
#include "api/include/pd_view.h"

#include <QtCore/QDir>
#include <QtCore/QLibrary>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<PDMemoryView*> s_memory_view_plugins;
static std::vector<PDView*> s_view_plugins;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegPlugins : public PDPluginRegister {
    void register_memory_view(PDMemoryView* view) {
        s_memory_view_plugins.push_back(view);
    }

    void register_view(PDView* view) {
        s_view_plugins.push_back(view);
    }
} s_reg_plugins;

typedef void (*InitPlugin)(PDPluginRegister* reg);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ViewPlugins::add_plugins(const QString& plugin_dir) {
    QDir dir(plugin_dir);

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

/*
PDMemoryView* ViewPlugins::find_plugin(const QString& name) {
    for (const auto& plugin : s_plugins) {
        if (plugin->name() == name) {
            return plugin;
        }
    }

    return nullptr;
}
*/

