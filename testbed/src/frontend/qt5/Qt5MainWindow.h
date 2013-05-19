#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <QMainWindow>
#include <QObject>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QT_BEGIN_NAMESPACE
class QMenu;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

class CodeEditor;
class Qt5ChildWindow;

class Qt5MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	Qt5MainWindow();
	void readSourceFile(const char* filename);

protected:
	void closeEvent(QCloseEvent* event);

private slots:
	Qt5ChildWindow* createChildWindow();

	void newFile();
	void openHexEditor();
	void startDebuggingTest();
	void getDebugStatus();

	void updateMenus();
	void updateWindowMenu();

	void setActiveSubWindow(QWidget* window);

private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();

	void readSettings();
	void writeSettings();

	Qt5ChildWindow* activeChildWindow();

	QMenu* m_fileMenu;
	QMenu* m_experimentsMenu;
	QMenu* m_helpMenu;

	QMdiArea* m_mdiArea;
	QSignalMapper* m_windowMapper;

	QMenu* m_windowMenu;

	QAction* m_closeAction;
	QAction* m_closeAllAction;
	QAction* m_tileAction;
	QAction* m_cascadeAction;
	QAction* m_nextAction;
	QAction* m_previousAction;
	QAction* m_separatorAction;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

