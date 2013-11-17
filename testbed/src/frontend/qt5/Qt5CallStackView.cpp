#include "Qt5CallStackView.h"
#include "Qt5MainWindow.h"

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStackViewContextMenu::Qt5CallStackViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent)
: Qt5DynamicViewContextMenu(mainWindow, parent)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStackViewContextMenu::~Qt5CallStackViewContextMenu()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStackView::Qt5CallStackView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent)
: Qt5BaseView(mainWindow, dock, parent)
{
	m_type = Qt5ViewType_CallStack;

	focusInEvent(nullptr);

	m_callStack = new Qt5CallStack(this);
	m_callStack->setFocusProxy(this);

	createFrameEmbedWidget(m_callStack, "Call Stack");

    connect(m_callStack, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5CallStackView::~Qt5CallStackView()
{
	disconnect();
	
	// Reset Focus Tracking (for safety)
	m_mainWindow->setCurrentWindow(nullptr, Qt5ViewType_Reset);

	centralWidget()->deleteLater();
    emit signalDelayedSetCentralWidget(nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
