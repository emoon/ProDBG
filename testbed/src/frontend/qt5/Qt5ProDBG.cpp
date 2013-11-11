#include <QtGui>
#include <QApplication>
#include <QWidget>
#include <core/PluginHandler.h>
#include <ProDBGAPI.h>
#include "Qt5CodeEditor.h"
#include "Qt5MainWindow.h"

#ifdef WIN32
#include <windows.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int realMain(int argc, char* argv[])
{
	QApplication app(argc, argv);

	QCoreApplication::setOrganizationName("ProDBG Crew");
	QCoreApplication::setOrganizationDomain("prodbg.com");
	QCoreApplication::setApplicationName("ProDBG");
	QCoreApplication::setApplicationVersion("0.0.1");

	if (!PluginHandler_addPlugin("tundra-output/macosx-clang-debug-default/libLLDBPlugin.dylib"))
	{
		printf("Unable to load LLDBPlugin\n");
	}

	app.setApplicationName("ProDBG");
	app.setOrganizationName("ProDBG");
	
	app.setStyle("plastique");

	{
		QFile f("data/darkorange.stylesheet");

		if (!f.exists())
		{
			printf("Unable to stylesheet\n");
		}
		else
		{
			f.open(QFile::ReadOnly | QFile::Text);
			QTextStream ts(&f);
			app.setStyleSheet(ts.readAll());
		}
	}

	Qt5MainWindow window;
	window.setupWorkspace();
	window.show();

	return app.exec();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return prodbg::realMain(0, 0);
}
			
#else
int main(int argc, char* argv[])
{
	return prodbg::realMain(argc, argv);
}
#endif

