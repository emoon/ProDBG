#include <QtGui>
#include <QApplication>
#include <QWidget>
#include <core/PluginHandler.h>
#include <ProDBGAPI.h>
#include "Qt5CodeEditor.h"
#include "Qt5MainWindow.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int realMain(int argc, char* argv[])
{
	QApplication app(argc, argv);
	Qt5MainWindow window;
	window.resize(800, 600);
	window.show();
	window.setWindowTitle(QApplication::translate("toplevel", "ProDBG"));

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

	window.readSourceFile("/Users/emoon/temp/foo.c");

	// test

	// Try to load plugin (hard coded to for now)

	/*
	if (PluginHandler_addPlugin("tundra-output/macosx-clang-debug-default/libLLDBPlugin.dylib"))
	{
		int count;

		// Hard coded for now as well
	
		Plugin* plugin = PluginHandler_getPlugins(&count);

		if (count == 1)
		{
			// try to start debugging session of a plugin


			PDDebugPlugin* debugPlugin = (PDDebugPlugin*)plugin->data;
			void* userData = debugPlugin->createInstance(0);
			debugPlugin->start(userData, PD_DEBUG_LAUNCH, (void*)"/Users/emoon/temp/foo");
		}
	}
	*/

	return app.exec();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	return prodbg::realMain(argc, argv);
}

