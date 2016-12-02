#include "Core/PluginHandler.h"
#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QResource>
#include <QTextEdit>
#include <QTextStream>
#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
    QApplication app(argc, (char**)argv);

    QCoreApplication::setOrganizationName("TBL");
    QCoreApplication::setOrganizationDomain("prodbg.com");
    QCoreApplication::setApplicationName("ProDBG");

    // load hard-coded list of backends for now

    prodbg::PluginHandler_addPlugin(QStringLiteral("dummy_backend_plugin"));
    prodbg::PluginHandler_addPlugin(QStringLiteral("amiga_uae_plugin"));

    prodbg::MainWindow main_window;

    main_window.show();

    return app.exec();
}
