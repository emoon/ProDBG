#pragma once

#include <QList>
#include <QObject>
#include <QPalette>
#include <QSettings>
#include <QString>

class QString;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Global configuration object for themes, etc
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Config : public QObject {
    Q_OBJECT

   public:
    Config();

    static void create_instance();
    inline static Config* instance() { return m_instance; };
    const QList<QString>& theme_list();
    Q_SLOT void load_theme(int index);
    int current_theme();

   private:
    void apply_native_style(const QString& name);

    QPalette m_default_palette;
    QSettings m_settings;

    static Config* m_instance;
};

