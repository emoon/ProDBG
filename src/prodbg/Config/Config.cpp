#include "Config.h"
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QPalette>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

Config* Config::m_instance = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Config::Config() {
    load_theme(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void load_style_sheet(const QString& name) {
    QFile f(name);
    QString stylesheet;
    if (!f.exists()) {
        qDebug() << "Config: Unable to load " << name;
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        stylesheet = ts.readAll();
        //QPalette p = qApp->palette();
        //p.setColor(QPalette::Text, text_color);
        //qApp->setPalette(p);
        qApp->setStyleSheet(stylesheet);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Config::load_theme(int theme) {
    // TODO: Enum
    switch (theme) {
        case 0 : { load_style_sheet(QStringLiteral(":native/native.qss")); break; }
        case 1 : { load_style_sheet(QStringLiteral(":qdarkstyle/style.qss")); break; }
        case 2 : { load_style_sheet(QStringLiteral(":midnight/style.css")); break; }
        case 3 : { load_style_sheet(QStringLiteral(":lightstyle/light.qss")); break; }
    }
}

/*
#ifdef Q_OS_MACX
        stylesheet += "QDockWidget::title"
        "{"
        "    background-color: #353434;"
        "    text-align: center;"
        "    height: 12px;"
        "}";
#endif
*/


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Config::create_instance() {
    m_instance = new Config;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

