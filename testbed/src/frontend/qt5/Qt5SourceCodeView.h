#pragma once

#include "Qt5CodeEditor.h"
#include "Qt5BaseView.h"
#include "Qt5DynamicView.h"

namespace prodbg
{

class Qt5SourceCodeViewContextMenu : public Qt5DynamicViewContextMenu
{
	Q_OBJECT;

public:
	Qt5SourceCodeViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent = nullptr);
	virtual ~Qt5SourceCodeViewContextMenu();
};

class Qt5SourceCodeView : public Qt5BaseView
{
	Q_OBJECT;

public:
	Qt5SourceCodeView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent = nullptr);
	virtual ~Qt5SourceCodeView();

	virtual Qt5ViewType getViewType() const
	{
		return Qt5ViewType_SourceCode;
	}

protected:
	virtual Qt5ContextMenu* createContextMenu()
	{
		return new Qt5SourceCodeViewContextMenu(m_mainWindow, this);
	}

public slots:
	void buildLayout();
	void applyLayout(Qt5Layout* layout);

private:
	Qt5CodeEditor* m_sourceCode;
};

}
