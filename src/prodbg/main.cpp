#include "Core/PluginHandler.h"
#include "MainWindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QResource>
#include <QTextEdit>
#include <QTextStream>
#include <stdio.h>
#include <stdlib.h>
#include <QStyleFactory>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
    QApplication app(argc, (char**)argv);

    QCoreApplication::setOrganizationName(QStringLiteral("TBL"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("prodbg.com"));
    QCoreApplication::setApplicationName(QStringLiteral("ProDBG"));

    /*
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
 
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53,53,53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25,25,25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53,53,53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        
    app.setPalette(darkPalette);
    
    app.setStyleSheet(QStringLiteral("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }"));
    */

    // load hard-coded list of backends for now

    prodbg::PluginHandler_addPlugin(QStringLiteral("dummy_backend_plugin"));
    prodbg::PluginHandler_addPlugin(QStringLiteral("amiga_uae_plugin"));

    prodbg::MainWindow main_window;

    main_window.show();

    return app.exec();
}
