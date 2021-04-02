#include "config.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QPalette>
#include <QWidget>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Config* Config::m_instance = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Move to core something

template <class T>
const T& clamp(const T& v, const T& lo, const T& hi) {
    assert(!(hi < lo));
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Config::Config() : m_default_palette(qApp->palette()) {
    qDebug() << "setting filename " << m_settings.fileName();

    int current_theme = m_settings.value(QStringLiteral("color_theme"), 0).toInt();
    load_theme(current_theme);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QString load_style_sheet(const QString& name) {
    QFile f(name);
    QString stylesheet;

    if (!f.exists()) {
        qDebug() << "Config: Unable to load " << name;
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        stylesheet = ts.readAll();
    }

    return stylesheet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void apply_style_sheet(const QString& name, const QColor& text_color) {
    QString stylesheet = load_style_sheet(name);

    if (stylesheet == QStringLiteral("")) {
        return;
    }

    QPalette p = qApp->palette();
    p.setColor(QPalette::Text, text_color);
    qApp->setPalette(p);
    qApp->setStyleSheet(stylesheet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void apply_dark_style(const QString& name) {
    QString stylesheet = load_style_sheet(name);

    if (stylesheet == QStringLiteral("")) {
        return;
    }

#ifdef Q_OS_MACX
    // see https://github.com/ColinDuquesnoy/QDarkStyleSheet/issues/22#issuecomment-96179529
    stylesheet += QStringLiteral(
        "QDockWidget::title"
        "{"
        "    background-color: #31363b;"
        "    text-align: center;"
        "    height: 12px;"
        "}");
#endif
    QPalette p = qApp->palette();
    p.setColor(QPalette::Text, Qt::white);
    qApp->setPalette(p);
    qApp->setStyleSheet(stylesheet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Config::apply_native_style(const QString& name) {
    QString stylesheet = load_style_sheet(name);

    if (stylesheet == QStringLiteral("")) {
        return;
    }

    qApp->setStyleSheet(stylesheet);
    qApp->setPalette(m_default_palette);

    // Taken from Cutter
    // Some widgets does not change its palette when QApplication changes one so
    // this loop force all widgets do this,
    /// but all widgets take palette from QApplication::palette() when they are
    /// created so line above is necessary too.
    for (auto widget : qApp->allWidgets()) {
        widget->setPalette(m_default_palette);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Config::load_theme(int index) {
    auto themes = theme_list();
    int theme = clamp(index, 0, themes.size() - 1);

    const QString& name = themes[index];

    m_settings.setValue(QStringLiteral("color_theme"), theme);

    if (name == QStringLiteral("DarkStyle")) {
        apply_dark_style(QStringLiteral(":qdarkstyle/style.qss"));
    } else if (name == QStringLiteral("Midnight")) {
        apply_style_sheet(QStringLiteral(":midnight/style.css"), Qt::white);
    } else if (name == QStringLiteral("Light")) {
        apply_style_sheet(QStringLiteral(":lightstyle/light.qss"), Qt::black);
    } else {
        apply_native_style(QStringLiteral(":native/native.qss"));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Config::current_theme() { return m_settings.value(QStringLiteral("color_theme"), 0).toInt(); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const QList<QString>& Config::theme_list() {
    static const QList<QString> list = {QStringLiteral("Native"), QStringLiteral("DarkStyle"),
                                        QStringLiteral("Midnight"), QStringLiteral("Light")};

    return list;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Config::create_instance() { m_instance = new Config; }
