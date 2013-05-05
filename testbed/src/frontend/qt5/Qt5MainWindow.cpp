#include "Qt5MainWindow.h"
#include "Qt5CodeEditor.h"
#include <QAction>
#include <QMenuBar>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newFile()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5MainWindow::Qt5MainWindow()
{
     QAction* newAct = new QAction(tr("&New"), this);
     newAct->setShortcuts(QKeySequence::New);
     newAct->setStatusTip(tr("Create a new file"));
     connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

     // connect menu

	 m_fileMenu = menuBar()->addMenu(tr("&File"));
	 m_fileMenu->addAction(newAct);

	 m_codeEditor = new CodeEditor(this);
	 setCentralWidget(m_codeEditor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::readSourceFile(const char* filename)
{
	m_codeEditor->readSourceFile(filename);
}


}

