#pragma once

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QString>
#include "api/include/pd_ui.h"

class QPluginLoader;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

class View;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ViewHandler : public QObject {
    Q_OBJECT;

public:
    ViewHandler(QObject* parent);
    ~ViewHandler();

    struct PluginInfo {
        QPluginLoader* plugin;
        QString plugin_name;
    };

    const QVector<PluginInfo>& plugin_types() { return m_plugin_types; }

    bool load_plugins(const QString& plugin_dir);

    QWidget* create_view_by_name(const QString& name);
    QWidget* create_view_by_index(int index);

private:
    void plugin_view_closed(QObject* obj);

    struct ViewInstance {
        PDUIInterface* view_plugin;
        QWidget* widget;
    };

    QVector<PluginInfo> m_plugin_types;
    QVector<ViewInstance> m_views;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}  // namespace prodbg
