#include "ViewHandler.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QPluginLoader>
#include "View.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ViewHandler::ViewHandler(QObject* parent) : QObject(parent) {}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ViewHandler::~ViewHandler() {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ViewHandler::load_plugins(const QString& plugin_dir) {
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

        qDebug() << filename;
        QPluginLoader* plugin_loader = new QPluginLoader(dir.absoluteFilePath(filename));
        QJsonObject meta_data = plugin_loader->metaData();

        QJsonValue plugin_name_obj = meta_data.value(QStringLiteral("MetaData"));
        qDebug() << meta_data;
        qDebug() << plugin_name_obj;

        if (plugin_name_obj == QJsonValue::Undefined) {
            qDebug() << "no metadata";
            delete plugin_loader;
            continue;
        }

        plugin_name_obj = plugin_name_obj.toObject().value(QStringLiteral("prodbg_view_plugin_name"));

        qDebug() << "Found plugin " << plugin_name_obj.toString();

        ViewHandler::PluginInfo plugin_info = {
            plugin_loader,
            plugin_name_obj.toString(),
        };

        m_plugin_types.append(plugin_info);

        plugins_found++;
    }

    printf("found %d\n", plugins_found );

    return plugins_found > 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QWidget* ViewHandler::create_view_by_name(const QString& plugin_name) {
    for (int i = 0, e = m_plugin_types.length(); i < e; ++i) {
        if (m_plugin_types[i].plugin_name == plugin_name) {
            return create_view_by_index(i);
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ViewHandler::plugin_view_closed(QObject* obj) {
    for (size_t i = 0, len = m_views.size(); i < len; ++i) {
        if (m_views[i].widget == obj) {
            m_views.remove(i);
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QWidget* ViewHandler::create_view_by_index(int index) {
    if (index >= m_plugin_types.length()) {
        return nullptr;
    }

    // Create new plugin instance
    QObject* plugin_obj = m_plugin_types[index].plugin->instance();

    if (!plugin_obj) {
        qDebug() << "Unable to create plugin " << m_plugin_types[index].plugin_name << " "
                 << m_plugin_types[index].plugin->errorString();
        return nullptr;
    }

    auto view_plugin = qobject_cast<PDUIInterface*>(plugin_obj);

    if (!view_plugin) {
        qDebug() << "Unable to cast plugin " << m_plugin_types[index].plugin_name;
        return nullptr;
    }

    QWidget* widget = new QWidget(nullptr);

    PDUIInterface* instance = view_plugin->create(widget);
    widget->setAttribute(Qt::WA_DeleteOnClose);

    QObject::connect(widget, &QObject::destroyed, this, &ViewHandler::plugin_view_closed);

    m_views.append({instance, widget});

    return widget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
