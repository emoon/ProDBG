#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Qt5HexEditWidget.h"
#include "Qt5BaseView.h"
#include "Qt5DynamicView.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5HexEditViewContextMenu : public Qt5DynamicViewContextMenu
{
	Q_OBJECT;

public:
	Qt5HexEditViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent = nullptr);
	virtual ~Qt5HexEditViewContextMenu();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5HexEditView : public Qt5BaseView
{
	Q_OBJECT;

public:
	Qt5HexEditView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent = nullptr);
	virtual ~Qt5HexEditView();

	virtual Qt5ViewType getViewType() const
	{
		return Qt5ViewType_HexEdit;
	}

	virtual QString getViewTypeName() const
	{
		return "Memory";
	}

protected:
	virtual Qt5ContextMenu* createContextMenu()
	{
		return new Qt5HexEditViewContextMenu(m_mainWindow, this);
	}

private slots:
	void setAddress(int address);
	void setSize(int size);

private:
	void testEditor();

private:
	Qt5HexEditWidget* m_hexEdit;

	QStatusBar* m_statusBar;
	QLabel* m_addressLabel;
	QLabel* m_addressNameLabel;
    QLabel* m_overwriteModeLabel;
    QLabel* m_overwriteModeNameLabel;
    QLabel* m_sizeLabel;
    QLabel* m_sizeNameLabel;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
