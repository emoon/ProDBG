#include "Application.h"
#include "MainWindow.h"
#include <QApplication>

namespace prodbg
{

QApplication* g_application = nullptr;

// test

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Application_init(int argc, const char** argv)
{
    QApplication app(argc, (char**)argv);
    g_application = &app;

    app.setApplicationName("ProDBG");
    app.setApplicationVersion("0.1");

    app.setOrganizationName("ProDBG");
    app.setOrganizationDomain("www.prodbg.com");

    MainWindow window;
    window.show();

    return app.exec();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}


