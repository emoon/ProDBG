#include "Qt5MainWindow.h"
#include "Qt5BaseView.h"
#include "Qt5CallStackView.h"
#include "Qt5RegistersView.h"
#include "Qt5LocalsView.h"
#include "Qt5SourceCodeView.h"
#include "Qt5HexEditView.h"
#include "Qt5DebugOutputView.h"
#include <QAction>
#include <QMenuBar>
#include <QStatusBar>
#include <QLabel>

#include <core/PluginHandler.h>
#include <core/Core.h>
#include <ProDBGAPI.h>
#include "Qt5DebuggerThread.h"
#include "Qt5CallStack.h"
#include "Qt5Locals.h"
#include "ProDBGAPI.h"
#include "Qt5DebugSession.h"

namespace prodbg
{

//const char* QT5_BUILD_VERSION = "v0.0.1";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newDynamicView()
{
	Qt5DockWidget* dock = new Qt5DockWidget(tr("Dynamic View"), this, this, m_nextView);
	dock->setAttribute(Qt::WA_DeleteOnClose, true);

	Qt5DynamicView* dynamicView = new Qt5DynamicView(this, dock, this);
	dock->setWidget(dynamicView);
	addDockWidget(Qt::LeftDockWidgetArea, dock);

	emit dynamicView->signalDelayedSetCentralWidget(dynamicView->m_statusLabel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newCallStackView()
{
	newView<Qt5CallStackView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newRegistersView()
{
	newView<Qt5RegistersView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newLocalsView()
{
	newView<Qt5LocalsView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newSourceCodeView()
{
	newView<Qt5SourceCodeView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newHexEditView()
{
	newView<Qt5HexEditView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::newDebugOutputView()
{
	newView<Qt5DebugOutputView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::assignCallStackView()
{
	assignView<Qt5CallStackView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::assignLocalsView()
{
	assignView<Qt5LocalsView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::assignSourceCodeView()
{
	assignView<Qt5SourceCodeView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::assignHexEditView()
{
	assignView<Qt5HexEditView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::assignDebugOutputView()
{
	assignView<Qt5DebugOutputView>();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::fileSettingsFinished(int result)
{
	(void)result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct MenuDescriptor
{
	const char* name;
	int id;
	bool active;
	const char* shortCut;
	MenuDescriptor* subMenu;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum MenuIds
{
	//
	
	MENU_SEPARATOR = 1,

 	MENU_FILE_NEW_PROJECT = 0x1000,	
 	MENU_FILE_CLOSE_PROJECT,	
 	MENU_FILE_LOAD_PROJECT,	
 	MENU_FILE_LOAD_EXECUTABLE,	
 	MENU_FILE_LOAD_SOURCE,	
 	MENU_FILE_LOAD_DEFAULT_LAYOUT,	

 	// Debug

	MENU_DEBUG_GO,
	MENU_DEBUG_STOP,
	MENU_DEBUG_RELOAD,
	MENU_DEBUG_STEP,
	MENU_DEBUG_STEP_OVER,
	MENU_DEBUG_STEP_OUT,
	MENU_DEBUG_ATTACH_TO_PROCESS,
	MENU_DEBUG_ATTACH_TO_REMOTE_PROCESS,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MenuDescriptor s_loadMenu[] =
{
	{ "Project...", MENU_FILE_LOAD_PROJECT, false, "Ctrl+L", 0 },
	{ "Executable...", MENU_FILE_LOAD_EXECUTABLE, true, "Ctrl+F3", 0 },
	{ "Source...", MENU_FILE_LOAD_SOURCE, true, "Ctrl+O", 0 },
	{ "Default Layout", MENU_FILE_LOAD_DEFAULT_LAYOUT, false, "", 0 },
	{ 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MenuDescriptor s_fileMenu[] =
{
	{ "New Project", MENU_FILE_NEW_PROJECT, false, "Ctrl+N", 0 },
	{ "Load", 0, true, "", s_loadMenu },
	{ "", MENU_SEPARATOR, true, "", 0 },
	{ "Close Project", MENU_FILE_CLOSE_PROJECT, false, "Ctrl+D", 0 },
	{ "", MENU_SEPARATOR, true, "", 0 },
	{ 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static MenuDescriptor s_debugMenu[] =
{
	{ "Go", MENU_DEBUG_GO, true, "F5", 0 },
	{ "Stop", MENU_DEBUG_STOP, true, "Shift+F5", 0 },
	{ "Reload", MENU_DEBUG_RELOAD, true, "Ctrl+F2", 0 },
	{ "", MENU_SEPARATOR, true, "", 0 },
	{ "Step", MENU_DEBUG_STEP, true, "F11", 0 },
	{ "Step Over", MENU_DEBUG_STEP_OVER, true, "F10", 0 },
	{ "Step Out", MENU_DEBUG_STEP_OVER, true, "Shift+F11", 0 },
	{ "Attach process...", MENU_DEBUG_ATTACH_TO_PROCESS, false, "", 0 },
	{ "Attach Remote process...", MENU_DEBUG_ATTACH_TO_REMOTE_PROCESS , true, "Ctrl+r", 0 },
	{ 0, 0, 0, 0, 0 },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::onMenu(int id)
{
	switch (id)
	{
		case MENU_FILE_LOAD_EXECUTABLE :
		{
			QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
															"/home",
															tr("*"));	

			if (filename.isNull())
				return;

			const char* file = strdup(filename.toLatin1().constData());

			printf("Qt5MainWindow::onMenu %s\n", file);

			g_debugSession->begin(file, true);

			free((void*)file);

			break;
		}


		case MENU_DEBUG_ATTACH_TO_REMOTE_PROCESS :
		{
			g_debugSession->beginRemote("127.0.0.1", 1340);
			break;
		}

		case MENU_DEBUG_GO :
		{
			g_debugSession->callAction(PDAction_run);
			break;
		}

		case MENU_DEBUG_STOP :
		{
			g_debugSession->callAction(PDAction_break);
			break;
		}

		case MENU_DEBUG_STEP :
		{
			g_debugSession->callAction(PDAction_step);
			break;
		}
	}

	printf("%d\n", id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::createMenu(MenuDescriptor* desc, QMenu* menu)
{
	while (desc->name)
	{
		if (desc->id == MENU_SEPARATOR)
			menu->addSeparator();
		else if (desc->subMenu)
		{
			QMenu* newMenu = menu->addMenu(desc->name);
			createMenu(desc->subMenu, newMenu);
		}
		else
		{
			QAction* action = new QAction(desc->name, this);
			action->setShortcut(QKeySequence(QString(desc->shortCut)));
			action->setEnabled(desc->active);
			m_signalMapper->setMapping(action, desc->id);
			menu->addAction(action);
			connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
		}

		desc++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::createActions()
{
	// Describe window menu actions
	///////////////////////////////////////////////////////////////////////////////////////////

	m_windowSplitVerticallyAction = new QAction(tr("Split Vertically"), this);
	m_windowSplitHorizontallyAction = new QAction(tr("Split Horizontally"), this);
	m_windowFillMainWindowAction = new QAction(tr("Fill Main Window"), this);
	m_windowUnfillMainWindowAction = new QAction(tr("Unfill Main Window"), this);
	m_windowDeleteViewAction = new QAction(tr("Delete View"), this);
	m_windowCloseViewAction = new QAction(tr("Close View"), this);
	m_windowCloseViewAction->setEnabled(false);

	// Describe dynamic view assignment actions
	m_windowNewDynamicViewAction = new QAction(tr("Dynamic View"), this);

	m_windowNewCallStackViewAction = new QAction(tr("CallStack View"), this);
	m_windowNewLocalsViewAction = new QAction(tr("Locals View"), this);
	m_windowNewSourceCodeViewAction = new QAction(tr("Source Code View"), this);
	m_windowNewHexEditViewAction = new QAction(tr("Hex Edit View"), this);
	m_windowNewDebugOutputViewAction = new QAction(tr("Debug Output View"), this);

	m_windowAssignCallStackViewAction = new QAction(tr("CallStack View"), this);
	m_windowAssignLocalsViewAction = new QAction(tr("Locals View"), this);
	m_windowAssignSourceCodeViewAction = new QAction(tr("Source Code View"), this);
	m_windowAssignHexEditViewAction = new QAction(tr("Hex Edit View"), this);
	m_windowAssignDebugOutputViewAction = new QAction(tr("Debug Output View"), this);

	// Descibe help menu actions
	///////////////////////////////////////////////////////////////////////////////////////////

	m_helpAboutAction = new QAction(tr("&About"), this);
	m_helpContentsAction = new QAction(tr("&Contents..."), this);
	m_helpIndexAction = new QAction(tr("&Index..."), this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::createMenus()
{
	QMenuBar* mainMenuBar = menuBar();

	// Connect file menu
	///////////////////////////////////////////////////////////////////////////////////////////

	createMenu(s_fileMenu, mainMenuBar->addMenu(tr("&File")));
	createMenu(s_debugMenu, mainMenuBar->addMenu(tr("&Debug")));

	// Connect window menu
	///////////////////////////////////////////////////////////////////////////////////////////

	m_newWindowMenu = new QMenu("New View", mainMenuBar);
	m_newWindowMenu->addAction(m_windowNewDynamicViewAction);
	m_newWindowMenu->addAction(m_windowNewCallStackViewAction);
	m_newWindowMenu->addAction(m_windowNewLocalsViewAction);
	m_newWindowMenu->addAction(m_windowNewSourceCodeViewAction);
	m_newWindowMenu->addAction(m_windowNewHexEditViewAction);
	m_newWindowMenu->addAction(m_windowNewDebugOutputViewAction);

	m_dynamicWindowAssignViewMenu = new QMenu("Assign View", mainMenuBar);
	m_dynamicWindowAssignViewMenu->addAction(m_windowAssignCallStackViewAction);
	m_dynamicWindowAssignViewMenu->addAction(m_windowAssignLocalsViewAction);
	m_dynamicWindowAssignViewMenu->addAction(m_windowAssignSourceCodeViewAction);
	m_dynamicWindowAssignViewMenu->addAction(m_windowAssignHexEditViewAction);
	m_dynamicWindowAssignViewMenu->addAction(m_windowAssignDebugOutputViewAction);

	m_dynamicWindowMenu = new QMenu("Dynamic View", mainMenuBar);
	m_dynamicWindowMenu->addAction(m_windowSplitVerticallyAction);
	m_dynamicWindowMenu->addAction(m_windowSplitHorizontallyAction);
	m_dynamicWindowMenu->addAction(m_windowFillMainWindowAction);
	m_dynamicWindowMenu->addAction(m_windowDeleteViewAction);
	m_dynamicWindowMenu->addSeparator();
	m_dynamicWindowMenu->setEnabled(false);
	m_dynamicWindowMenu->addMenu(m_dynamicWindowAssignViewMenu);

	m_windowMenu = new QMenu("&Window", mainMenuBar);
	m_windowMenu->addMenu(m_newWindowMenu);
	m_windowMenu->addAction(m_windowCloseViewAction);
	m_windowMenu->addSeparator();
	m_windowMenu->addMenu(m_dynamicWindowMenu);	

	// Connect help menu
	///////////////////////////////////////////////////////////////////////////////////////////

	m_helpMenu = menuBar()->addMenu(tr("&Help"));
	m_helpMenu->addAction(m_helpContentsAction);
	m_helpMenu->addAction(m_helpIndexAction);
	m_helpMenu->addSeparator();
	m_helpMenu->addAction(m_helpAboutAction);

	connect(m_signalMapper, SIGNAL(mapped(int)), SLOT(onMenu(int)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::createToolBars()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::createStatusBar()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5MainWindow::Qt5MainWindow() : Qt5BaseView(this, nullptr, nullptr)
{
	m_backgroundWidget = new QWidget;
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	m_backgroundWidget->setSizePolicy(sizePolicy);
	m_backgroundWidget->setAutoFillBackground(true);
	m_backgroundWidget->setFocusProxy(this);
	setCentralWidget(m_backgroundWidget);

	m_signalMapper = new QSignalMapper(this);

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	setWindowTitle(QApplication::translate("ProDBG", "ProDBG"));
	//setUnifiedTitleAndToolBarOnMac(true);

	setCurrentWindow(nullptr, Qt5ViewType_Init);

	m_centralWidgetSet = false;

	QApplication::setStartDragDistance(48);

	m_currentViewType = Qt5ViewType_Init;
	m_lastViewType = Qt5ViewType_Init;

	m_currentView = nullptr;
	m_lastView = nullptr;
	m_currentWindowMenu = nullptr;
	m_deletingDockWidget = false;
	//m_settingsWindow = nullptr;
	m_shutdown = false;

	resetViews();

	m_id = m_mainWindow->addView();

	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
	connect(m_backgroundWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));

	QString objectName;
	objectName.setNum(m_id);
	setObjectName(objectName);

	connect(this, SIGNAL(destroyed(QObject*)), this, SLOT(shutdown(QObject*)));

	// TODO: This is a bit temporary but will do for now

	Qt5DebugSession::createSession();

	newSourceCodeView();
	newRegistersView();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5MainWindow::~Qt5MainWindow()
{
	m_shutdown = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::setupWorkspace()
{
	// Describe window menu actions
	///////////////////////////////////////////////////////////////////////////////////////////

	connect(m_windowSplitVerticallyAction, SIGNAL(triggered()), this, SLOT(windowSplitVertically()));
	connect(m_windowSplitHorizontallyAction, SIGNAL(triggered()), this, SLOT(windowSplitHorizontally()));
	connect(m_windowFillMainWindowAction, SIGNAL(triggered()), this, SLOT(windowFillMainWindow()));
	connect(m_windowUnfillMainWindowAction, SIGNAL(triggered()), this, SLOT(windowUnfillMainWindow()));
	connect(m_windowDeleteViewAction, SIGNAL(triggered()), this, SLOT(windowDeleteView()));
	connect(m_windowCloseViewAction, SIGNAL(triggered()), this, SLOT(windowCloseView()));

	// Describe dynamic view assignment actions
	connect(m_windowNewDynamicViewAction, SIGNAL(triggered()), this, SLOT(newDynamicView()));
	connect(m_windowNewCallStackViewAction, SIGNAL(triggered()), this, SLOT(newCallStackView()));
	connect(m_windowNewLocalsViewAction, SIGNAL(triggered()), this, SLOT(newLocalsView()));
	connect(m_windowNewSourceCodeViewAction, SIGNAL(triggered()), this, SLOT(newSourceCodeView()));
	connect(m_windowNewHexEditViewAction, SIGNAL(triggered()), this, SLOT(newHexEditView()));
	connect(m_windowNewDebugOutputViewAction, SIGNAL(triggered()), this, SLOT(newDebugOutputView()));

	connect(m_windowAssignCallStackViewAction, SIGNAL(triggered()), this, SLOT(assignCallStackView()));
	connect(m_windowAssignLocalsViewAction, SIGNAL(triggered()), this, SLOT(assignLocalsView()));
	connect(m_windowAssignSourceCodeViewAction, SIGNAL(triggered()), this, SLOT(assignSourceCodeView()));
	connect(m_windowAssignHexEditViewAction, SIGNAL(triggered()), this, SLOT(assignHexEditView()));
	connect(m_windowAssignDebugOutputViewAction, SIGNAL(triggered()), this, SLOT(assignDebugOutputView()));

	// Descibe help menu actions
	///////////////////////////////////////////////////////////////////////////////////////////

	connect(m_helpAboutAction, SIGNAL(triggered()), this, SLOT(helpAbout()));
	connect(m_helpContentsAction, SIGNAL(triggered()), this, SLOT(helpContents()));
	connect(m_helpIndexAction, SIGNAL(triggered()), this, SLOT(helpIndex()));

	focusInEvent(nullptr);
	m_type = Qt5ViewType_Main;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::setCurrentWindow(Qt5BaseView* widget, Qt5ViewType type)
{
	if (m_shutdown)
		return;

	if (type == Qt5ViewType_Reset || type == Qt5ViewType_Init)
	{
		// We reset views incase a view was destroyed, or any other "risky" situation.
		m_currentView = this;
		m_currentViewType = Qt5ViewType_Main;

		m_lastView = this;
		m_lastViewType = Qt5ViewType_Main;

		if (type == Qt5ViewType_Reset)
		{
			m_dynamicWindowMenu->setEnabled(false);

	        Qt5ContextMenu* windowMenu = new Qt5ContextMenu(m_mainWindow, m_mainWindow);
	        m_mainWindow->setWindowMenu(windowMenu);
		}

		return;
	}

	if (widget == m_currentView)
	{
		// If it's the same widget... do nothing.
	}
	else if (widget)
	{
		// If we have a valid new widget, back the old one up into LastView and assign.
		m_lastView = m_currentView;
		m_lastViewType = m_currentViewType;

		m_currentView = widget;
		m_currentViewType = type;
	}

	// Make sure we setup our menu permissions anyway

	Qt5BaseView* view = m_currentView;
	Qt5ViewType viewType = m_currentViewType;

	if (m_currentViewType == Qt5ViewType_Main)
	{
		viewType = m_lastViewType;
		view = m_lastView;
	}

	while (viewType >= Qt5ViewType_Dynamic && view->m_parent && view->m_parent != this)
	{
		view = view->m_parent;
		viewType = view->m_type;
	}

	if (view && viewType == Qt5ViewType_Dynamic)
	{
		if (view->m_parent == this && view->m_parentDock != nullptr)
		{
			m_windowFillMainWindowAction->setEnabled(true);
		}
		else
		{
			m_windowFillMainWindowAction->setEnabled(false);
		}

		if (((Qt5DynamicView*)view) == centralWidget())
		{
			m_windowUnfillMainWindowAction->setEnabled(true);
		}
		else
		{
			m_windowUnfillMainWindowAction->setEnabled(false);
		}
	}
	else
	{
		m_dynamicWindowMenu->setEnabled(false);
		m_windowCloseViewAction->setEnabled(false);
	}

	// If we are in the process of deleting a dock widget...
	if (m_deletingDockWidget)
	{
		m_windowCloseViewAction->setEnabled(false);
		m_deletingDockWidget = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5BaseView* Qt5MainWindow::getCurrentWindow(Qt5ViewType type)
{
	if (m_shutdown)
		return nullptr;

	if (type == Qt5ViewType_Dock || type == Qt5ViewType_Dynamic)
	{
		Qt5DynamicView* dynamicView = nullptr;

		if (m_currentViewType == Qt5ViewType_Main)
		{
			if (m_lastViewType > Qt5ViewType_Main)
			{
				if (m_lastViewType == Qt5ViewType_Dynamic)
				{
					dynamicView = (Qt5DynamicView*)m_lastView;
				}
				else if (m_lastViewType > Qt5ViewType_Dynamic)
				{
					dynamicView = (Qt5DynamicView*)m_lastView->parent();
				}
				else
				{
					abort();
				}
			}
			else
			{
				return nullptr;
			}
		}
		else if (m_currentViewType > Qt5ViewType_Main)
		{
			if (m_currentViewType == Qt5ViewType_Dynamic)
			{
				dynamicView = (Qt5DynamicView*)m_currentView;
			}
			else if (m_currentViewType > Qt5ViewType_Dynamic)
			{
				dynamicView = (Qt5DynamicView*)m_currentView->parent();
			}
			else
			{
				abort();
			}
		}
		else
		{
			return nullptr;
		}

		if (type == Qt5ViewType_Dock)
		{
			// Navigate the object hierarchy until we find the top-level dynamic view
			while (dynamicView->m_parentDock == nullptr && dynamicView->m_parent != this)
			{
				// This is an embedded Qt5DynamicView in the Qt5MainWindow
				if (dynamicView->m_parent == this && dynamicView->m_parentDock == nullptr)
				{
					return dynamicView;
				}

				dynamicView = (Qt5DynamicView*)dynamicView->m_parent;
			}
		}
		
		return dynamicView;
	}
	else if (m_currentViewType == type)
	{
		return m_currentView;
	}
	else if (m_currentViewType == Qt5ViewType_Main && m_lastViewType == type)
	{
		return m_lastView;
	}
	else
	{
		return nullptr;	
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::loadLayout(Qt5Layout* layout)
{
	(void)layout;
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::saveLayout()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::addLayout(Qt5LayoutEntry* layoutEntry)
{
	(void)layoutEntry;
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Qt5MainWindow::addView()
{
	if (m_nextView >= QT5_MAX_VIEWS)
		return -1;

	int nextView = -1;

	if (m_nextView <= -1)
	{
		for (int index = 0; index < QT5_MAX_VIEWS; ++index)
		{
			if (!m_viewTable[index])
			{
				nextView = index;
				m_viewTable[index] = true;
				break;
			}
		}
	}
	else
	{
		nextView = m_nextView;
	}

	for (int index = 0; index < QT5_MAX_VIEWS; ++index)
	{
		if (!m_viewTable[index])
		{
			m_nextView = index;
			m_viewTable[index] = true;
			break;
		}
	}

	return nextView;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::deleteView(int id)
{
	m_viewTable[id] = false;
	m_nextView = -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::resetViews()
{
	for (int index = 0; index < QT5_MAX_VIEWS; ++index)
	{
		m_viewTable[index] = false;
	}

	m_nextView = -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::closeEvent(QCloseEvent*)
{
	shutdown(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::contextMenuEvent(QContextMenuEvent*)
{
	if (m_shutdown)
		return;

	setCurrentWindow(this, Qt5ViewType_Main);
	
	Q_ASSERT(m_currentWindowMenu != nullptr);
	m_currentWindowMenu->display(cursor().pos());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::focusInEvent(QFocusEvent*)
{
	if (m_currentView == this || m_shutdown)
		return;

	setCurrentWindow(this, Qt5ViewType_Main);

	if (m_lastViewType >= Qt5ViewType_Dynamic)
		return;

	Qt5ContextMenu* windowMenu = new Qt5ContextMenu(this, this);

	setWindowMenu(windowMenu);

	// We don't set a context menu here because it needs to preserve the Window
	// menu of the menuBar.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::fileExit()
{
	close();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::fileResetLayout()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::fileSettings()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::fileSaveLayout()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void Qt5MainWindow::debugStart()
{
	//g_debugSession->begin("tundra-output/macosx-clang-debug-default/Fake6502");
	//g_debugSession->begin("tundra-output/macosx-clang-debug-default/prodbg-qt5");
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::helpIndex()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::helpContents()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::helpAbout()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::setWindowMenu(Qt5ContextMenu* menu)
{
	Q_ASSERT(menu != nullptr);

	if (m_shutdown)
		return;

	QMenuBar* mainMenuBar = menuBar();
	Q_ASSERT(mainMenuBar != nullptr);

	if (m_currentWindowMenu != nullptr)
	{		
		mainMenuBar->removeAction(m_currentWindowMenu->menuAction());
		m_currentWindowMenu->deleteLater();
	}
	else
	{
		Q_ASSERT(m_windowMenu != nullptr);
		mainMenuBar->removeAction(m_windowMenu->menuAction());
	}

	m_currentWindowMenu = menu;

	Q_ASSERT(m_helpMenu != nullptr);
	Q_ASSERT(m_currentWindowMenu != nullptr);
	mainMenuBar->insertAction(m_helpMenu->menuAction(), m_currentWindowMenu->menuAction());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5ContextMenu* Qt5MainWindow::getWindowMenu()
{
	if (m_shutdown)
		return nullptr;

	return m_currentWindowMenu;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::triggerSignalSettings()
{
	emit signalSettings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::triggerSignalApplyLayout(Qt5Layout* layout)
{
	emit signalApplyLayout(layout);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::windowSplitVertically()
{
	Qt5DynamicView* view = reinterpret_cast<Qt5DynamicView*>(getCurrentWindow(Qt5ViewType_Dynamic));
	if (view == nullptr)
		return;

	view->splitView(Qt::Horizontal);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::windowSplitHorizontally()
{
	Qt5DynamicView* view = reinterpret_cast<Qt5DynamicView*>(getCurrentWindow(Qt5ViewType_Dynamic));
	if (view == nullptr)
		return;

	view->splitView(Qt::Vertical);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::windowFillMainWindow()
{
	Qt5DynamicView* view = reinterpret_cast<Qt5DynamicView*>(getCurrentWindow(Qt5ViewType_Dynamic));
	if (view == nullptr)
		return;

	while (view != nullptr &&
		   view->m_type >= Qt5ViewType_Dynamic &&
		   view->m_parent != nullptr &&
		   view->m_parent != this)
	{
		view = reinterpret_cast<Qt5DynamicView*>(view->m_parent);
	}

	if (view == nullptr || view->m_parent == this)
		return;

	if (m_centralWidgetSet)
	{
		Qt5DynamicView* current = reinterpret_cast<Qt5DynamicView*>(centralWidget());
		Qt5DockWidget* dock = new Qt5DockWidget(tr("Dynamic View"), this, this, current->m_id);
		dock->setAttribute(Qt::WA_DeleteOnClose, true);
		dock->setWidget(current);
		current->m_parentDock = dock;
		addDockWidget(Qt::LeftDockWidgetArea, dock);
	}
	else if (centralWidget())
	{
		centralWidget()->deleteLater();
	}

	m_centralWidgetSet = true;

	if (view->m_parentDock != nullptr)
	{
		removeDockWidget(view->m_parentDock);
		view->m_parentDock->deleteLater();
		view->m_parentDock = nullptr;
	}

	view->setParent(this);

	emit signalDelayedSetCentralWidget(view);

	if (view->m_children[0] != nullptr)
	{
		setCurrentWindow(view->m_children[0], view->m_children[0]->m_type);
	}
	else
	{
		setCurrentWindow(view, Qt5ViewType_Dynamic);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::windowUnfillMainWindow()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::windowDeleteView()
{
	Qt5DynamicView* dynamicView = reinterpret_cast<Qt5DynamicView*>(getCurrentWindow(Qt5ViewType_Dynamic));
	if (dynamicView == nullptr)
		return;

	// Most the time the m_pParent of a dynamic view will be another Dynamic View.
	// However, for base Dynamic views, (Who have a mainwindow as a parent) it will be of type AR_Impl_Main.
	// Therefore, I can't give it the proper type in the header, so you have to cast it when you need to deal
	// with its specialized members.
	//
	// Even though it's annoying spam, it's very important because it means our Dynamic Views stay
	// agnostic and behave as they should no matter what they are embedded into.

	m_deletingDockWidget = true;
	setCurrentWindow(this, Qt5ViewType_Main);

	Qt5ContextMenu* windowMenu = new Qt5ContextMenu(this, this);
	setWindowMenu(windowMenu);

	// If we are embedded into the MainWindow then we just delete ourself!
	if (dynamicView->m_parent == this && dynamicView->m_parentDock == nullptr)
	{
		dynamicView->deleteLater();
		m_centralWidgetSet = false;
		m_backgroundWidget = new QWidget(this);
		m_backgroundWidget->setObjectName(QString::fromUtf8("backgroundWidget"));
		//m_backgroundWidget->setPalette(m_backgroundPalette);
		m_backgroundWidget->setAutoFillBackground(true);
		emit signalDelayedSetCentralWidget(m_backgroundWidget);
		return;
	}

	// If we are the only remaining DynamicView in a DockWidget then we
	// remove the dock, delete ourself, then delete the dock.
	if (dynamicView->m_parentDock != nullptr)
	{
		removeDockWidget(dynamicView->m_parentDock);
		dynamicView->m_parentDock->deleteLater();
		dynamicView->deleteLater();
		return;
	}

	Qt5DynamicView* parent = (Qt5DynamicView*)dynamicView->m_parent;
	Qt5DynamicView* child1 = (Qt5DynamicView*)parent->m_children[0];
	Qt5DynamicView* child2 = (Qt5DynamicView*)parent->m_children[1];

	// Otherwise, we delete ourself, our parent,
	// and put our sibling view where the parent used to be.
	if (child1 == dynamicView)
	{
		if (parent->m_parentDock != nullptr)
		{
			child2->m_parent = this;
			child2->setParent(this);
			child2->m_parentDock = parent->m_parentDock;
			child2->m_parentDock->setWidget(child2);
		}
		else if (parent->m_parentDock == nullptr && parent->m_parent == this)
		{
			child2->m_parent = this;
			child2->setParent(this);
			emit signalDelayedSetCentralWidget(child2);
		}
		else
		{
			int index = 0;
			if (((Qt5DynamicView*)parent->m_parent)->m_children[0] == parent)
			{
				index = ((Qt5DynamicView *)parent->m_parent)->m_splitter->indexOf(((Qt5DynamicView*)parent->m_parent)->m_children[0]);
				((Qt5DynamicView*)parent->m_parent)->m_children[0] = child2;
			}
			else if (((Qt5DynamicView*)parent->m_parent)->m_children[1] == parent)
			{
				index = ((Qt5DynamicView*)parent->m_parent)->m_splitter->indexOf(((Qt5DynamicView*)parent->m_parent)->m_children[1]);
				((Qt5DynamicView*)parent->m_parent)->m_children[1] = child2;
			}

			child2->m_parent = parent->m_parent;
			child2->setParent(parent->m_parent);

			((Qt5DynamicView*)parent->m_parent)->m_splitter->insertWidget(index, child2);
		}
	}
	else if (child2 == dynamicView)
	{
		if (parent->m_parentDock)
		{
			child1->m_parent = this;
			child1->setParent(this);
			child1->m_parentDock = parent->m_parentDock;
			child1->m_parentDock->setWidget(child1);
		}
		else if (parent->m_parentDock == nullptr && parent->m_parent == this)
		{
			child1->m_parent = this;
			child1->setParent(this);
			emit signalDelayedSetCentralWidget(child1);
		}
		else
		{
			int index = 0;
			if (((Qt5DynamicView*)parent->m_parent)->m_children[0] == parent)
			{
				index = ((Qt5DynamicView*)parent->m_parent)->m_splitter->indexOf(((Qt5DynamicView*)parent->m_parent)->m_children[0]);
				((Qt5DynamicView*)parent->m_parent)->m_children[0] = child1;
			}
			else if (((Qt5DynamicView*)parent->m_parent)->m_children[1] == parent)
			{
				index = ((Qt5DynamicView*)parent->m_parent)->m_splitter->indexOf(((Qt5DynamicView*)parent->m_parent)->m_children[1]);
				((Qt5DynamicView*)parent->m_parent)->m_children[1] = child1;
			}

			child1->m_parent = parent->m_parent;
			child1->setParent(parent->m_parent);

			((Qt5DynamicView*)parent->m_parent)->m_splitter->insertWidget(index, child1);
		}
	}

	dynamicView->deleteLater();
	parent->deleteLater();

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::windowCloseView()
{
	Qt5DynamicView* view = reinterpret_cast<Qt5DynamicView*>(getCurrentWindow(Qt5ViewType_Dynamic));
	if (view == nullptr)
		return;

	setCurrentWindow(this, Qt5ViewType_Main);

	Qt5ContextMenu* windowMenu = new Qt5ContextMenu(this, this);
	setWindowMenu(windowMenu);

	if (view->m_parentDock != nullptr)
	{
		removeDockWidget(view->m_parentDock);
		view->m_parentDock->deleteLater();
		view->m_parentDock = nullptr;
	}
	else
	{
		m_centralWidgetSet = false;
		m_backgroundWidget = new QWidget;
		m_backgroundWidget->setObjectName(QString::fromUtf8("backgroundWidget"));
		m_backgroundWidget->setAutoFillBackground(true);
		emit signalDelayedSetCentralWidget(m_backgroundWidget);
	}

	view->deleteLater();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::contextMenuProxy(const QPoint&)
{
	if (m_shutdown)
		return;

	setCurrentWindow(this, Qt5ViewType_Main);
	m_currentWindowMenu->display(cursor().pos());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::applySettings()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::buildLayout()
{
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::applyLayout(Qt5Layout* layout)
{
	(void)layout;
	printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::shutdown(QObject*)
{
	m_shutdown = true;

	/*if (m_settingsWindow != nullptr)
	{
		m_settingsWindow->close();
		m_settingsWindow->deleteLater();
		m_settingsWindow = nullptr;
	}*/

// saveSettings();
	saveLayout();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5MainWindow::errorMessage(const QString& message)
{
	QMessageBox::critical((QWidget*)parent(), QString("Error"), QString(message), QMessageBox::Ok, QMessageBox::Ok);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

