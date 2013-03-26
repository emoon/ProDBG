#include <QtGui>
#include <QApplication>
#include <QWidget>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QWidget window;
	window.resize(800, 600);
	window.show();
	window.setWindowTitle(QApplication::translate("toplevel", "ProDBG"));
	return app.exec();
}

