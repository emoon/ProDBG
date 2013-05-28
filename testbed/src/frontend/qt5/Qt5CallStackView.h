#pragma once

#include "Qt5CallStack.h"
#include "Qt5BaseView.h"
#include "Qt5DynamicView.h"

namespace prodbg
{

class Qt5CallStackViewContextMenu : public Qt5DynamicViewContextMenu
{
	Q_OBJECT;

public:
	Qt5CallStackViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent = nullptr);
	virtual ~Qt5CallStackViewContextMenu();
};

class Qt5CallStackView : public Qt5BaseView
{
	Q_OBJECT;

public:
	Qt5CallStackView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent = nullptr);
	virtual ~Qt5CallStackView();

	virtual Qt5ViewType getViewType() const
	{
		return Qt5ViewType_CallStack;
	}

protected:
	virtual Qt5ContextMenu* createContextMenu()
	{
		return new Qt5CallStackViewContextMenu(m_mainWindow, this);
	}

public slots:
	void buildLayout();
	void applyLayout(Qt5Layout* layout);

private:
	Qt5CallStack* m_callStack;
};

}
