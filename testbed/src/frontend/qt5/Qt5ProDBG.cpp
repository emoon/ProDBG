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

	if (!PluginHandler_addPlugin("tundra-output/macosx-clang-debug-default/libLLDBPlugin.dylib"))
	{
		printf("Unable to load LLDBPlugin\n");
	}

	app.setApplicationName("ProDBG");
	app.setOrganizationName("ProDBG");

	Qt5MainWindow window;
	window.resize(900, 600);
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

