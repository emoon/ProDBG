#include "Qt5LocalsView.h"
#include "Qt5MainWindow.h"

namespace prodbg
{

Qt5LocalsViewContextMenu::Qt5LocalsViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent)
: Qt5DynamicViewContextMenu(mainWindow, parent)
{
}

Qt5LocalsViewContextMenu::~Qt5LocalsViewContextMenu()
{
}

Qt5LocalsView::Qt5LocalsView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent)
: Qt5BaseView(mainWindow, dock, parent)
{
	m_type = Qt5ViewType_Locals;

	focusInEvent(nullptr);
	
	m_locals = new Qt5Locals(parent);
    setCentralWidget(m_locals);

    connect(parent, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
    connect(m_locals, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));

    m_locals->setFocusProxy(this);
}

Qt5LocalsView::~Qt5LocalsView()
{
	disconnect();
	
	// Reset Focus Tracking (for safety)
	m_mainWindow->setCurrentWindow(nullptr, Qt5ViewType_Reset);

	centralWidget()->deleteLater();
    emit signalDelayedSetCentralWidget(nullptr);
}

void Qt5LocalsView::buildLayout()
{

}

void Qt5LocalsView::applyLayout(Qt5Layout* layout)
{
	(void)layout;
}

}