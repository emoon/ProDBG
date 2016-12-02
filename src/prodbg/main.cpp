#include "Core/PluginHandler.h"
#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QResource>
#include <QTextEdit>
#include <QTextStream>
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
    QApplication app(argc, (char**)argv);

    // load hard-coded list of backends for now

    prodbg::PluginHandler_addPlugin(QStringLiteral("dummy_backend_plugin"));
    prodbg::PluginHandler_addPlugin(QStringLiteral("amiga_uae_plugin"));

    /*
    QResource::registerResource("data/dark_theme/style.rcc");

    QFile f("data/dark_theme/style.qss");

    if (!f.exists()) {
        printf("Unable to stylesheet\n");
    } else {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        app.setStyleSheet(ts.readAll());
    }
    */

    prodbg::MainWindow main_window;

    main_window.show();

    return app.exec();
}
