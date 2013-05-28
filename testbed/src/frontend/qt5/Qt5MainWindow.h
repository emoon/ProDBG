#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QMainWindow>
#include <QObject>
#include "Qt5BaseView.h"
#include "Qt5Layout.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QMenu;

class QProgressBar;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

class Qt5DockWidget;
class Qt5SettingsWindow;

#ifndef QT5_MAX_VIEWS
#define QT5_MAX_VIEWS 65535
#endif

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

public slots:
	void fileSettings();
	void fileSaveLayout();
	void fileResetLayout();
	void fileExit();

	void debugStart();

	// Dynamic Views
	void windowSplitVertically();
	void windowSplitHorizontally();
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
	void newDynamicView();
	void newExampleView1();
	void newExampleView2();
	void newExampleView3();

	void fileSettingsFinished(int result);

protected:
	enum Qt5ViewType m_currentViewType;
	enum Qt5ViewType m_lastViewType;

	Qt5BaseView* m_currentView;
	Qt5BaseView* m_lastView;

	Qt5LayoutEntry* m_layoutEntries;

	int m_currentLayoutEntry;
	int m_viewCount;
	int m_nextView;

	bool m_viewTable[QT5_MAX_VIEWS];

	bool m_deletingDockWidget;
	bool m_shutdown;

	QWidget* m_backgroundWidget;

	QProgressBar* m_mainWindowProgressBar;
	int m_progressBarStateCount;

private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();


	QMenu* m_fileMenu;
	QMenu* m_windowMenu;
	QMenu* m_newWindowMenu;
	QMenu* m_dynamicWindowMenu;
	QMenu* m_dynamicWindowAssignViewMenu;
	QMenu* m_debugMenu;
	QMenu* m_helpMenu;

	QAction* m_fileResetLayoutAction;
	QAction* m_fileExitAction;
	QAction* m_fileSettingsAction;
	QAction* m_fileSaveLayoutAction;

	QAction* m_debugStartAction;

	QAction* m_windowSplitVerticallyAction;
	QAction* m_windowSplitHorizontallyAction;
	QAction* m_windowFillMainWindowAction;
	QAction* m_windowUnfillMainWindowAction;
	QAction* m_windowDeleteViewAction;
	QAction* m_windowCloseViewAction;

	// TODO: Encapsulate specific view logic into plugins or something
	QAction* m_windowNewDynamicViewAction;
	QAction* m_windowNewExampleView1Action;
	QAction* m_windowNewExampleView2Action;
	QAction* m_windowNewExampleView3Action;

	QAction* m_windowAssignExampleView1Action;
	QAction* m_windowAssignExampleView2Action;
	QAction* m_windowAssignExampleView3Action;

	QAction* m_helpAboutAction;
	QAction* m_helpContentsAction;
	QAction* m_helpIndexAction;



Qt5ContextMenu* m_currentWindowMenu;

	Qt5SettingsWindow* m_settingsWindow;
	

	

	bool m_centralWidgetSet;

signals:
	void signalSettings();
	void signalBuildLayout();
	void signalApplyLayout(Qt5Layout* layout);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

