#pragma once

#include "Qt5DynamicView.h"

namespace prodbg
{

class CodeEditor;

class Qt5CodeEditorViewContextMenu : public Qt5DynamicViewContextMenu
{
	Q_OBJECT

public:
	Qt5CodeEditorViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent = nullptr);
	virtual ~Qt5CodeEditorViewContextMenu();
};

class Qt5CodeEditorView : public Qt5BaseView
{
	Q_OBJECT

	friend class Qt5MainWindow;
	friend class Qt5CodeEditorViewContextMenu;

public:
	Qt5CodeEditorView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5BaseView* parent = nullptr);
	virtual ~Qt5CodeEditorView();

	virtual Qt5ViewType getViewType() const
	{
		return Qt5ViewType_PluginStart;
	}

protected:
	virtual Qt5ContextMenu* createContextMenu()
	{
		return new Qt5CodeEditorViewContextMenu(m_mainWindow, this);
	}

private:
	CodeEditor* m_codeEditor;
};

}