#pragma once

#include "Qt5DebugOutput.h"
#include "Qt5BaseView.h"
#include "Qt5DynamicView.h"

namespace prodbg
{

class Qt5DebugOutputViewContextMenu : public Qt5DynamicViewContextMenu
{
	Q_OBJECT;

public:
	Qt5DebugOutputViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent = nullptr);
	virtual ~Qt5DebugOutputViewContextMenu();
};

class Qt5DebugOutputView : public Qt5BaseView
{
	Q_OBJECT;

public:
	Qt5DebugOutputView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent = nullptr);
	virtual ~Qt5DebugOutputView();

	virtual Qt5ViewType getViewType() const
	{
		return Qt5ViewType_DebugOutput;
	}

protected:
	virtual Qt5ContextMenu* createContextMenu()
	{
		return new Qt5DebugOutputViewContextMenu(m_mainWindow, this);
	}

public slots:
	void buildLayout();
	void applyLayout(Qt5Layout* layout);

private:
	Qt5DebugOutput* m_debugOutput;
};

}