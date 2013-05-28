#include "Qt5BaseView.h"
#include "Qt5MainWindow.h"
#include "Qt5DynamicView.h"

#include <QtWidgets>

namespace prodbg
{

Qt5BaseView::Qt5BaseView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5BaseView* parent)
: m_mainWindow(mainWindow)
, m_parentDock(dock)
, m_parent(parent)
, m_type(Qt5ViewType_Reset)
{
    printf("Event: %s - %s\n", __FILE__, __FUNCTION__);

	setParent(m_parent);

    if (m_mainWindow == this)
    {
        m_mainWindow->m_nextView = -1;
        m_mainWindow->m_viewCount = 0;
    }
    else
    {
        connect(m_mainWindow, SIGNAL(signalSettings()), this, SLOT(applySettings()), Qt::DirectConnection);
        connect(m_mainWindow, SIGNAL(signalBuildLayout()), this, SLOT(buildLayout()), Qt::DirectConnection);
        connect(m_mainWindow, SIGNAL(signalApplyLayout(Qt5Layout*)), this, SLOT(applyLayout(Qt5Layout*)), Qt::DirectConnection);
    }

    connect(this, SIGNAL(signalDelayedSetCentralWidget(QWidget*)), this, SLOT(delayedSetCentralWidget(QWidget*)), Qt::QueuedConnection);

    m_id = m_mainWindow->addView();
    m_entry = 0;

    QString objectName;
    objectName.setNum(m_id);
    setObjectName(objectName);

    m_mainWindow->m_viewCount++;
}

Qt5BaseView::~Qt5BaseView()
{
    printf("Event: %s - %s\n", __FILE__, __FUNCTION__);
    m_mainWindow->m_viewCount--;
    m_mainWindow->deleteView(m_id);
}

void Qt5BaseView::contextMenuEvent(QContextMenuEvent* event)
{
    printf("Event: %s - %s\n", __FILE__, __FUNCTION__);

    focusInEvent(nullptr);

    if (!hasSplitter())
    {
        m_mainWindow->getWindowMenu()->display(cursor().pos());
    }

    if (event != nullptr)
    {
        event->accept();
    }
}

void Qt5BaseView::focusInEvent(QFocusEvent* event)
{
    printf("Event: %s - %s\n", __FILE__, __FUNCTION__);

    Qt5BaseView* baseView = m_mainWindow->getCurrentWindow(Qt5ViewType_Dock);
    if (baseView != nullptr)
    {
        if (baseView->m_parentDock == nullptr)
        {
            m_mainWindow->m_windowCloseViewAction->setEnabled(false);
        }
        else
        {
            m_mainWindow->m_windowCloseViewAction->setEnabled(true);
        }
    }
    
    if (m_mainWindow->m_currentView == this)
        return;

    m_mainWindow->setCurrentWindow(this, getViewType());

    if (!hasSplitter())
    {
        Qt5ContextMenu* contextMenu = createContextMenu();
        if (contextMenu != nullptr)
        {
            m_mainWindow->setWindowMenu(contextMenu);
        }

        QString wndMessage;
        wndMessage.setNum(m_id);
        wndMessage.prepend("Active Window ID: ");
        m_mainWindow->statusBar()->clearMessage();
        m_mainWindow->statusBar()->showMessage(wndMessage);
    }

    if (event != nullptr)
    {
        event->accept();
    }
}

void Qt5BaseView::closeEvent(QCloseEvent* event)
{
    printf("Event: %s - %s\n", __FILE__, __FUNCTION__);


    m_mainWindow->setCurrentWindow(m_mainWindow, Qt5ViewType_Main);
    m_mainWindow->setWindowMenu(new Qt5ContextMenu(m_mainWindow, m_mainWindow));

    if (event != nullptr)
    {
        event->accept();
    }
}

void Qt5BaseView::contextMenuProxy(const QPoint&)
{
    printf("Event: %s - %s\n", __FILE__, __FUNCTION__);

    focusInEvent(nullptr);
    if (!hasSplitter())
    {
        m_mainWindow->getWindowMenu()->display(cursor().pos());
    }
}

void Qt5BaseView::delayedSetCentralWidget(QWidget* widget)
{
    printf("Event: %s - %s\n", __FILE__, __FUNCTION__);

	setCentralWidget(widget);
}

bool Qt5BaseView::hasSplitter() const
{
    printf("Event: %s - %s\n", __FILE__, __FUNCTION__);

	if (getViewType() == Qt5ViewType_Dynamic)
    {
        return ((Qt5DynamicView*)this)->getSplitter() != nullptr;
    }

    if (m_parent && (m_parent->getViewType() == Qt5ViewType_Dynamic))
    {
        return ((Qt5DynamicView*)m_parent)->getSplitter() != nullptr;
    }

    return false;
}

}
