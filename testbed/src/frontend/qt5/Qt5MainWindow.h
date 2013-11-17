#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QMainWindow>
#include <QObject>
#include <QSignalMapper>

#include <core/Core.h>
#include <qt5/Qt5BaseView.h>
#include <qt5/Qt5Layout.h>
#include <qt5/Qt5DockWidget.h>
#include <qt5/Qt5DynamicView.h>
#include <qt5/Qt5Settings.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QMenu;
class QProgressBar;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDDebugPlugin;

namespace prodbg
{

class Qt5SettingsWindow;
class Qt5CallStackView;
class Qt5LocalsView;
class Qt5SourceCodeView;
class Qt5HexEditView;
class Qt5DebugOutputView;

class Qt5DebuggerThread;

#ifndef QT5_MAX_VIEWS
#define QT5_MAX_VIEWS 65535
#endif

struct MenuDescriptor;

extern Qt5Settings* g_settings;
extern QApplication* g_application;

class Qt5MainWindow : public Qt5BaseView
{
	Q_OBJECT

	friend class Qt5BaseView;
	friend class Qt5DockWidget;
	friend class Qt5ContextMenu;
	friend class Qt5DynamicViewContextMenu;

public:
	Qt5MainWindow();
	virtual ~Qt5MainWindow();

	void setupWorkspace();

	void setWindowMenu(Qt5ContextMenu* menu);
	Qt5ContextMenu* getWindowMenu();

	void triggerSignalSettings();
	void triggerSignalApplyLayout(Qt5Layout* layout);

	void loadLayout(Qt5Layout* layout);
	void saveLayout();
	void addLayout(Qt5LayoutEntry* layoutEntry);

	int addView();
	void deleteView(int id);
	void resetViews();

	void readSourceFile(const char* filename);

	virtual Qt5ViewType getViewType() const
	{
		return Qt5ViewType_Main;
	}

	virtual QString getViewTypeName() const
	{
		return "Main Window";
	}

public slots:
	void fileSettings();
	void fileSaveLayout();
	void fileResetLayout();
	void fileExit();

	// Dynamic Views
	void windowFillMainWindow();
	void windowUnfillMainWindow();
	void windowDeleteView();
	void windowCloseView();

	void helpIndex();
	void helpContents();
	void helpAbout();

	void contextMenuProxy(const QPoint& position);

	void applySettings();
	void buildLayout();
	void applyLayout(Qt5Layout* layout);

	void shutdown(QObject* object);
	void errorMessage(const QString& message);

protected:
	void closeEvent(QCloseEvent* event);
	void contextMenuEvent(QContextMenuEvent* event);
	void focusInEvent(QFocusEvent* event);

public:
	void setCurrentWindow(Qt5BaseView* widget, Qt5ViewType viewType);
	Qt5BaseView* getCurrentWindow(Qt5ViewType type);

private slots:
	void newCallStackView();
	void newLocalsView();
	void newRegistersView();
	void newSourceCodeView();
	void newHexEditView();
	void newDebugOutputView();

	void assignCallStackView();
	void assignLocalsView();
	void assignRegistersView();
	void assignSourceCodeView();
	void assignHexEditView();
	void assignDebugOutputView();

	void splitHorizontalCallStackView();
	void splitHorizontalLocalsView();
	void splitHorizontalRegistersView();
	void splitHorizontalSourceCodeView();
	void splitHorizontalHexEditView();
	void splitHorizontalDebugOutputView();

	void splitVerticalCallStackView();
	void splitVerticalLocalsView();
	void splitVerticalRegistersView();
	void splitVerticalSourceCodeView();
	void splitVerticalHexEditView();
	void splitVerticalDebugOutputView();

	void fileSettingsFinished(int result);
	void onMenu(int id);

protected:
	enum Qt5ViewType m_currentViewType;
	enum Qt5ViewType m_lastViewType;

	Qt5BaseView* m_currentView;
	Qt5BaseView* m_lastView;

	Qt5LayoutEntry* m_layoutEntries;

	int32 m_currentLayoutEntry;
	int32 m_viewCount;
	int32 m_nextView;

	bool m_viewTable[QT5_MAX_VIEWS];

	bool m_deletingDockWidget;
	bool m_shutdown;

	QWidget* m_backgroundWidget;

	QProgressBar* m_mainWindowProgressBar;
	int32 m_progressBarStateCount;

private:

	void createMenu(MenuDescriptor* desc, QMenu* parentMenu);

	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();

	QSignalMapper* m_signalMapper;

	QMenu* m_windowMenu;
	QMenu* m_newWindowMenu;
	QMenu* m_dynamicWindowMenu;
	QMenu* m_dynamicWindowAssignViewMenu;
	QMenu* m_dynamicWindowSplitHorizontalViewMenu;
	QMenu* m_dynamicWindowSplitVerticalViewMenu;
	QMenu* m_helpMenu;

	QAction* m_windowFillMainWindowAction;
	QAction* m_windowUnfillMainWindowAction;
	QAction* m_windowDeleteViewAction;
	QAction* m_windowCloseViewAction;

	// TODO: Encapsulate specific view logic into plugins or something
	QAction* m_windowNewCallStackViewAction;
	QAction* m_windowNewLocalsViewAction;
	QAction* m_windowNewRegistersViewAction;
	QAction* m_windowNewSourceCodeViewAction;
	QAction* m_windowNewHexEditViewAction;
	QAction* m_windowNewDebugOutputViewAction;

	QAction* m_windowAssignCallStackViewAction;
	QAction* m_windowAssignLocalsViewAction;
	QAction* m_windowAssignRegistersViewAction;
	QAction* m_windowAssignSourceCodeViewAction;
	QAction* m_windowAssignHexEditViewAction;
	QAction* m_windowAssignDebugOutputViewAction;

	QAction* m_windowSplitHorizontalCallStackViewAction;
	QAction* m_windowSplitHorizontalLocalsViewAction;
	QAction* m_windowSplitHorizontalRegistersViewAction;
	QAction* m_windowSplitHorizontalSourceCodeViewAction;
	QAction* m_windowSplitHorizontalHexEditViewAction;
	QAction* m_windowSplitHorizontalDebugOutputViewAction;

	QAction* m_windowSplitVerticalCallStackViewAction;
	QAction* m_windowSplitVerticalLocalsViewAction;
	QAction* m_windowSplitVerticalRegistersViewAction;
	QAction* m_windowSplitVerticalSourceCodeViewAction;
	QAction* m_windowSplitVerticalHexEditViewAction;
	QAction* m_windowSplitVerticalDebugOutputViewAction;

	QAction* m_helpAboutAction;
	QAction* m_helpContentsAction;
	QAction* m_helpIndexAction;

	Qt5ContextMenu* m_currentWindowMenu;
	Qt5SettingsWindow* m_settingsWindow;

	bool m_centralWidgetSet;

	// Temporary. Used to reduce some code bloat

	template<class T> void newView()
	{
		Qt5DockWidget* dock = new Qt5DockWidget(tr("Dynamic View"), this, this, m_nextView);
		dock->setAttribute(Qt::WA_DeleteOnClose, true);

		Qt5DynamicView* dynamicView = new Qt5DynamicView(this, dock, this);
		dock->setWidget(dynamicView);

		//emit dynamicView->signalDelayedSetCentralWidget(dynamicView->m_statusLabel);

		T* view = new T(this, dock, dynamicView);
		dynamicView->m_children[0] = view;
		dynamicView->assignView(view);

		addDockWidget(Qt::LeftDockWidgetArea, dock);
	}

	// Temporary. Used to reduce some code bloat

	template<class T> void assignView()
	{
		Qt5DynamicView* dynamicView = reinterpret_cast<Qt5DynamicView*>(getCurrentWindow(Qt5ViewType_Dynamic));
		if (dynamicView == nullptr)
			return;

		T* view = new T(this, nullptr, dynamicView);
		if (dynamicView->m_children[0] != nullptr)
		{
			dynamicView->m_children[0]->hide();
			dynamicView->m_children[0]->deleteLater();
		}

		dynamicView->m_children[0] = view;
		dynamicView->assignView(view);
	}

	template<class T> void splitHorizontalView()
	{
		Qt5DynamicView* view = reinterpret_cast<Qt5DynamicView*>(getCurrentWindow(Qt5ViewType_Dynamic));
		if (view == nullptr)
			return;

		view->splitView(Qt::Vertical);

		assignView<T>();
	}

	template<class T> void splitVerticalView()
	{
		Qt5DynamicView* view = reinterpret_cast<Qt5DynamicView*>(getCurrentWindow(Qt5ViewType_Dynamic));
		if (view == nullptr)
			return;

		view->splitView(Qt::Horizontal);

		assignView<T>();
	}

signals:
	void signalSettings();
	void signalBuildLayout();
	void signalApplyLayout(Qt5Layout* layout);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

