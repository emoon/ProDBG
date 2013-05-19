#include "Qt5MainWindow.h"
#include "Qt5ChildWindow.h"
#include "Qt5CodeEditor.h"
#include "Qt5HexEditWindow.h"
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5ChildWindow* Qt5MainWindow::createChildWindow()
{
	Qt5ChildWindow* child = new Qt5ChildWindow;
   	m_mdiArea->addSubWindow(child);
   	return child;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newFile()
{
	Qt5ChildWindow* child = createChildWindow();
	//
	child->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::openHexEditor()
{
	Qt5HexEditWindow* window = new Qt5HexEditWindow;
	window->resize(1024, 768);
	window->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::startDebuggingTest()
{
	// Find currently active child window
	Qt5ChildWindow* child = activeChildWindow();
	if (child != nullptr)
	{
		child->beginDebug("tundra-output/macosx-clang-debug-default/Fake6502");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::getDebugStatus()
{
	//printf("Qt5MainWindow::getDebugStatus\n");

	// Find currently active child window
	Qt5ChildWindow* child = activeChildWindow();
	if (child != nullptr)
	{
		//child->getDebugStatus();
	}
}

void Qt5MainWindow::updateMenus()
{
	const bool hasChildWindow = (activeChildWindow() != 0);
	
	m_closeAction->setEnabled(hasChildWindow);
	m_closeAllAction->setEnabled(hasChildWindow);
	m_tileAction->setEnabled(hasChildWindow);
	m_cascadeAction->setEnabled(hasChildWindow);
	m_nextAction->setEnabled(hasChildWindow);
	m_previousAction->setEnabled(hasChildWindow);
	m_separatorAction->setVisible(hasChildWindow);
}

void Qt5MainWindow::updateWindowMenu()
{
	m_windowMenu->clear();
	m_windowMenu->addAction(m_closeAction);
	m_windowMenu->addAction(m_closeAllAction);
	m_windowMenu->addSeparator();
	m_windowMenu->addAction(m_tileAction);
	m_windowMenu->addAction(m_cascadeAction);
	m_windowMenu->addSeparator();
	m_windowMenu->addAction(m_nextAction);
	m_windowMenu->addAction(m_previousAction);
	m_windowMenu->addAction(m_separatorAction);

	QList<QMdiSubWindow*> windows = m_mdiArea->subWindowList();
	m_separatorAction->setVisible(!windows.isEmpty());

	for (int childIndex = 0; childIndex < windows.size(); ++childIndex)
	{
		Qt5ChildWindow* child = qobject_cast<Qt5ChildWindow*>(windows.at(childIndex)->widget());

		QString text;
		QString testLabel(tr("Test ProDBG View"));

		if (childIndex < 9)
		{
			text = tr("&%1 %2").arg(childIndex + 1)
							   .arg(testLabel);
		}
		else
		{
			text = tr("%1 %2").arg(childIndex + 1)
							  .arg(testLabel);
		}

		QAction* action  = m_windowMenu->addAction(text);
		action->setCheckable(true);
		action->setChecked(child == activeChildWindow());
		connect(action, SIGNAL(triggered()), m_windowMapper, SLOT(map()));
		m_windowMapper->setMapping(action, windows.at(childIndex));
	}
}

void Qt5MainWindow::setActiveSubWindow(QWidget* window)
{
	if (!window)
		return;

	m_mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

void Qt5MainWindow::createActions()
{
	m_closeAction = new QAction(tr("Cl&ose"), this);
	m_closeAction->setStatusTip(tr("Close the active window"));
	connect(m_closeAction, SIGNAL(triggered()),
			m_mdiArea, SLOT(closeActiveSubWindow()));

	m_closeAllAction = new QAction(tr("Close &All"), this);
	m_closeAllAction->setStatusTip(tr("Close all the windows"));
	connect(m_closeAllAction, SIGNAL(triggered()),
			m_mdiArea, SLOT(closeAllSubWindows()));

	m_tileAction = new QAction(tr("&Tile"), this);
	m_tileAction->setStatusTip(tr("Tile the windows"));
	connect(m_tileAction, SIGNAL(triggered()), m_mdiArea, SLOT(tileSubWindows()));

	m_cascadeAction = new QAction(tr("&Cascade"), this);
	m_cascadeAction->setStatusTip(tr("Cascade the windows"));
	connect(m_cascadeAction, SIGNAL(triggered()), m_mdiArea, SLOT(cascadeSubWindows()));

	m_nextAction = new QAction(tr("Ne&xt"), this);
	m_nextAction->setShortcuts(QKeySequence::NextChild);
	m_nextAction->setStatusTip(tr("Move the focus to the next window"));
	connect(m_nextAction, SIGNAL(triggered()),
			m_mdiArea, SLOT(activateNextSubWindow()));

	m_previousAction = new QAction(tr("Pre&vious"), this);
	m_previousAction->setShortcuts(QKeySequence::PreviousChild);
	m_previousAction->setStatusTip(tr("Move the focus to the previous "
								 "window"));
	connect(m_previousAction, SIGNAL(triggered()),
			m_mdiArea, SLOT(activatePreviousSubWindow()));

	m_separatorAction = new QAction(this);
	m_separatorAction->setSeparator(true);
}

void Qt5MainWindow::createMenus()
{
	m_windowMenu = menuBar()->addMenu(tr("&Window"));
	updateWindowMenu();
	connect(m_windowMenu, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()));

	menuBar()->addSeparator();
}

void Qt5MainWindow::createToolBars()
{

}

void Qt5MainWindow::createStatusBar()
{

}

void Qt5MainWindow::readSettings()
{
	QSettings settings("ProDBG", "ProDBG");
	QPoint position = settings.value("position", QPoint(200, 200)).toPoint();
	QSize size = settings.value("size", QSize(400, 400)).toSize();
	move(position);
	resize(size);
}

void Qt5MainWindow::writeSettings()
{
	QSettings settings("ProDBG", "ProDBG");
	settings.setValue("position", pos());
	settings.setValue("size", size());
}

Qt5ChildWindow* Qt5MainWindow::activeChildWindow()
{
	if (QMdiSubWindow* activeSubWindow = m_mdiArea->activeSubWindow())
		return qobject_cast<Qt5ChildWindow*>(activeSubWindow->widget());

	return nullptr;
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

	QAction* getStatusAction = new QAction(tr("Basic Debugging GetStatus"), this);
	getStatusAction->setStatusTip(tr("Get debugStatus"));
	connect(getStatusAction, SIGNAL(triggered()), this, SLOT(getDebugStatus()));

	// connect experiments menu

	m_experimentsMenu = menuBar()->addMenu(tr("&Experiments"));
	m_experimentsMenu->addAction(hexEditAct);
	m_experimentsMenu->addAction(basicDebugAction);
	m_experimentsMenu->addAction(getStatusAction);

	m_mdiArea = new QMdiArea;
	m_mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(m_mdiArea);

	connect(m_mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateMenus()));
	m_windowMapper = new QSignalMapper(this);
	connect(m_windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	updateMenus();

	readSettings();

	setWindowTitle(QApplication::translate("toplevel", "ProDBG"));
	setUnifiedTitleAndToolBarOnMac(true);
}

void Qt5MainWindow::closeEvent(QCloseEvent* event)
{
	m_mdiArea->closeAllSubWindows();

	if (m_mdiArea->currentSubWindow())
	{
		event->ignore();
	}
	else
	{
		writeSettings();
		event->accept();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::readSourceFile(const char* filename)
{
	// Find currently active child window
	Qt5ChildWindow* child = activeChildWindow();
	if (child != nullptr)
	{
		child->readSourceFile(filename);
	}
}


}

