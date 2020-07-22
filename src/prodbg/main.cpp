#include <stdio.h>
#include <stdlib.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QResource>
#include <QtCore/QTextStream>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QTextEdit>
#include "Core/PluginHandler.h"
#include "MainWindow.h"
#include "Config/Config.h"
#include "edbee/edbee.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv) {
    QApplication app(argc, (char**)argv);

    QCoreApplication::setOrganizationName(QStringLiteral("TBL"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("prodbg.com"));
    QCoreApplication::setApplicationName(QStringLiteral("ProDBG"));

    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    // Init source code component
    edbee::Edbee* edbee = edbee::Edbee::instance();
    edbee->setGrammarPath(QStringLiteral("data/syntaxfiles"));
    edbee->setThemePath(QStringLiteral("data/themes"));
    edbee->autoInit();
    edbee->init();

    prodbg::Config::create_instance();

    //prodbg::PluginHandler_addPlugin(QStringLiteral("dummy_backend_plugin"));
    prodbg::PluginHandler_addPlugin(QStringLiteral("lldb_plugin"));

    prodbg::MainWindow main_window;

    main_window.show();

    return app.exec();
}
