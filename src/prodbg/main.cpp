#include "MainWindow.h"
#include <QApplication>
#include <QTextEdit>
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
    QApplication app(argc, (char**)argv);

    prodbg::MainWindow main_window;

    main_window.show();

    // QTextEdit text_edit;
    // text_edit.show();

    return app.exec();
}
