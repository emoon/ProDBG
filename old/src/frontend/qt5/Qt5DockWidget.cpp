#include "Qt5DockWidget.h"
#include "Qt5MainWindow.h"
#include "Qt5BaseView.h"

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DockWidget::Qt5DockWidget(const QString&  title,
                             Qt5MainWindow*  mainWindow,
                             Qt5BaseView*    parent,
                             int             childId,
                             Qt::WindowFlags flags)
    : QDockWidget(title, parent, flags)
    , m_mainWindow(mainWindow)
    , m_parent(parent)
    , m_width(0)
    , m_height(0)
{
    QString objectName;
    objectName.setNum(childId);
    objectName.prepend("dock");
    setObjectName(objectName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DockWidget::~Qt5DockWidget()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DockWidget::closeEvent(QCloseEvent* event)
{
    Qt5BaseView* view = reinterpret_cast<Qt5BaseView*>(widget());
    if (view != nullptr)
    {
        view->m_mainWindow->m_deletingDockWidget = true;
        view->closeEvent(event);
        view->deleteLater();
        setWidget(nullptr);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
