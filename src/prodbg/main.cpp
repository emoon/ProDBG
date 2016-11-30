#include <QApplication>
#include <QTextEdit>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv) {
    QApplication app(argc, (char**)argv);

    QTextEdit text_edit;
    text_edit.show();

    return app.exec();
}

