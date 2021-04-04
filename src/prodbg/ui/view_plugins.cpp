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

#if 0

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<PDMemoryView*> s_memory_view_plugins;
static std::vector<PDView*> s_view_plugins;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegPlugins : public PDRegisterViewPlugin {
    void register_memory_view(PDMemoryView* view) {
        printf("added memory plugin\n");
        s_memory_view_plugins.push_back(view);
    }

    void register_view(PDView* view) {
        printf("added view plugin\n");
        s_view_plugins.push_back(view);
    }
} s_reg_plugins;

typedef void (*InitView)(PDPluginRegister* reg);

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

        QString fullpath = dir.absoluteFilePath(filename);
        QByteArray fname = fullpath.toUtf8();

        void* handle = shared_object_open_fullpath(fname.constData());

        if (!handle) {
            continue;
        }

        InitView init_view_plugin = shared_object_symbol(handle, "pd_register_view");

        if (init_view_plugin) {
            init_plugin(&s_reg_plugins);
        }

        // try to register backend plugin also

        BackendPluginHandler::add_by_handle(handle);
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

#endif
