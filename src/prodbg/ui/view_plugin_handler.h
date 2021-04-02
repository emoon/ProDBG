#pragma once

#include <QVector>
#include <QString>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ViewPluginHandler {
    bool load_plugins(const QString& plugin_dir);

    QWidget* create_plugin_by_name(const QString& plugin_name);
    QWidget* create_plugin_by_index(int index);

private:
};

