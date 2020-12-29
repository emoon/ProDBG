#include <stdio.h>
#include <stdlib.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QResource>
#include <QtCore/QTextStream>
#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QTextEdit>
#include "core/plugin_handler.h"
#include "../core/BackendPluginHandler.h"
#include "main_window.h"
#include "config/config.h"
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
    prodbg::BackendPluginHandler::add_plugin("lldb_plugin");
    if (!prodbg::BackendPluginHandler::add_plugin("file")) {
        printf("failed to add file plugin");
    }

    prodbg::MainWindow main_window;

    main_window.show();

    return app.exec();
}
