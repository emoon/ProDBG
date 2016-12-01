#include <QApplication>
#include <QTextEdit>
#include "MemoryView/MemoryViewWidget.h"
#include <stdio.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv) {
    QApplication app(argc, (char**)argv);

    char* dummy_data = (char*)malloc(1024 * 1024);

    for (int i = 0; i < 1024 * 1024; ++i)
        dummy_data[i] = (char)rand();

    prodbg::MemoryViewWidget hex_edit;

    hex_edit.setData(QByteArray(dummy_data, 1024 * 1024));

    hex_edit.show();

    //QTextEdit text_edit;
    //text_edit.show();

    return app.exec();
}

