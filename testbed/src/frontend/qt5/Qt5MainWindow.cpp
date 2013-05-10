#include "Qt5MainWindow.h"
#include "Qt5CodeEditor.h"
#include "Qt5HexEditWindow.h"
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newFile()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::openHexEditor()
{
	Qt5HexEditWindow* window = new Qt5HexEditWindow;
	window->resize(1024, 768);
	window->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::basicDebugAction()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5MainWindow::Qt5MainWindow()
{
	//setAttribute(Qt::WA_DeleteOnClose);

	// file menu actions

	QAction* newAct = new QAction(tr("&New"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create a new file"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

	// connect file menu

	m_fileMenu = menuBar()->addMenu(tr("&File"));
	m_fileMenu->addAction(newAct);

	// experiments menu actions

	QAction* hexEditAct = new QAction(tr("Hex Editor Test"), this);
	hexEditAct->setStatusTip(tr("Launch hex editor test"));
	connect(hexEditAct, SIGNAL(triggered()), this, SLOT(openHexEditor()));

	QAction* basicDebugAction = new QAction(tr("Basic Debugging Test"), this);
	basicDebugAction->setStatusTip(tr("Start basic debugging test"));
	connect(basicDebugAction, SIGNAL(triggered()), this, SLOT(startDebuggingTest()));

	// connect experiments menu

	m_experimentsMenu = menuBar()->addMenu(tr("&Experiments"));
	m_experimentsMenu->addAction(hexEditAct);
	m_experimentsMenu->addAction(basicDebugAction);

	m_codeEditor = new CodeEditor(this);
	setCentralWidget(m_codeEditor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::readSourceFile(const char* filename)
{
	m_codeEditor->readSourceFile(filename);
}


}

