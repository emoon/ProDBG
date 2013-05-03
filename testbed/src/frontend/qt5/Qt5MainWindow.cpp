#include "Qt5MainWindow.h"
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
}

}

