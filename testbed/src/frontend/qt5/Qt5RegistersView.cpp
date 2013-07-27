#include "Qt5RegistersView.h"
#include "Qt5MainWindow.h"

namespace prodbg
{

Qt5RegistersViewContextMenu::Qt5RegistersViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent)
: Qt5DynamicViewContextMenu(mainWindow, parent)
{
}

Qt5RegistersViewContextMenu::~Qt5RegistersViewContextMenu()
{
}

Qt5RegistersView::Qt5RegistersView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent)
: Qt5BaseView(mainWindow, dock, parent)
{
	m_type = Qt5ViewType_Registers;

	focusInEvent(nullptr);
	
	m_locals = new Qt5Registers(parent);
    setCentralWidget(m_locals);

    //connect(parent, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
    //connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
    //connect(m_locals, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
    //m_locals->setFocusProxy(this);
}

Qt5RegistersView::~Qt5RegistersView()
{
	disconnect();
	
	// Reset Focus Tracking (for safety)
	m_mainWindow->setCurrentWindow(nullptr, Qt5ViewType_Reset);

	centralWidget()->deleteLater();
    emit signalDelayedSetCentralWidget(nullptr);
}

void Qt5RegistersView::buildLayout()
{

}

void Qt5RegistersView::applyLayout(Qt5Layout* layout)
{
	(void)layout;
}

}
