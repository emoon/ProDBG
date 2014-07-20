#include "Qt5BaseView.h"
#include "Qt5MainWindow.h"
#include "Qt5DynamicView.h"
#include <QtWidgets>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5BaseView::Qt5BaseView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5BaseView* parent)
    : m_mainWindow(mainWindow)
    , m_parentDock(dock)
    , m_parent(parent)
    , m_type(Qt5ViewType_Reset)
    , m_frame(nullptr)
    , m_idLabel(nullptr)
{
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

    connect(parent, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuProxy(const QPoint &)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuProxy(const QPoint &)));

    m_id = m_mainWindow->addView();
    m_entry = 0;

    QString objectName;
    objectName.setNum(m_id);
    setObjectName(objectName);

    m_mainWindow->m_viewCount++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5BaseView::~Qt5BaseView()
{
    m_mainWindow->m_viewCount--;
    m_mainWindow->deleteView(m_id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5BaseView::contextMenuEvent(QContextMenuEvent* event)
{
    printf("%s :: contextMenuEvent\n", qPrintable(getViewTypeName()));

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5BaseView::focusInEvent(QFocusEvent* event)
{
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5BaseView::closeEvent(QCloseEvent* event)
{
    m_mainWindow->setCurrentWindow(m_mainWindow, Qt5ViewType_Main);
    m_mainWindow->setWindowMenu(new Qt5ContextMenu(m_mainWindow, m_mainWindow));

    if (event != nullptr)
    {
        event->accept();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QFrame* Qt5BaseView::createFrameEmbedWidget(QWidget* widget, const QString& title)
{
    m_frame = new QFrame(this);
    m_frame->setFrameShape(QFrame::Box);
    m_frame->setFrameShadow(QFrame::Plain);
    m_frame->setContextMenuPolicy(Qt::CustomContextMenu);
    m_frame->setFocusPolicy(Qt::WheelFocus);
    m_frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout* frameLayout = new QVBoxLayout();
    frameLayout->setContentsMargins(2, 2, 2, 2);

    QFont headerFont = QGuiApplication::font();
    headerFont.setWeight(75);
    headerFont.setBold(true);

    QHBoxLayout* headerLayout = new QHBoxLayout();
    headerLayout->setContentsMargins(2, 2, 0, 0);

    QLabel* titleLabel = new QLabel();
    titleLabel->setFont(headerFont);
    titleLabel->setFocusPolicy(Qt::NoFocus);
    titleLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    titleLabel->setText(title);

    m_idLabel = new QLabel();
    m_idLabel->setFont(headerFont);
    m_idLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    m_idLabel->setText("");

    headerLayout->addWidget(titleLabel);
    headerLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    headerLayout->addWidget(m_idLabel);

    QFrame* horizontalLine = new QFrame();
    horizontalLine->setFrameShape(QFrame::HLine);
    horizontalLine->setFrameShadow(QFrame::Plain);
    horizontalLine->setContextMenuPolicy(Qt::CustomContextMenu);
    horizontalLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    frameLayout->addLayout(headerLayout);

    frameLayout->addWidget(horizontalLine);

    if (widget != nullptr)
    {
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        frameLayout->addWidget(widget);
    }

    m_frame->setLayout(frameLayout);
    setCentralWidget(m_frame);

    connect(m_frame, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuProxy(const QPoint &)));
    connect(titleLabel, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuProxy(const QPoint &)));
    connect(m_idLabel, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuProxy(const QPoint &)));

    m_frame->setFocusProxy(this);

    return m_frame;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5BaseView::contextMenuProxy(const QPoint&)
{
    printf("%s :: contextMenuProxy\n", qPrintable(getViewTypeName()));

    focusInEvent(nullptr);
    if (!hasSplitter())
    {
        m_mainWindow->getWindowMenu()->display(cursor().pos());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5BaseView::buildLayout()
{
    Qt5LayoutEntry entry;
    g_settings->resetEntry(&entry);

    entry.entryId = m_id;
    entry.viewType = m_type;
    entry.parentId = m_parent->m_id;
    entry.positionX = mapToGlobal(pos()).x();
    entry.positionY = mapToGlobal(pos()).y();
    entry.sizeX = width();
    entry.sizeY = height();

    m_mainWindow->addLayout(&entry);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5BaseView::applyLayout(Qt5Layout* layout)
{
    Qt5LayoutEntry* entry = &layout->entries[m_entry];

    QPoint mpos = mapFromGlobal(QPoint(entry->positionX, entry->positionY));
    setGeometry(mpos.x(), mpos.y(), entry->sizeX, entry->sizeY);

    QString idString;
    idString.setNum(entry->entryId);
    idString.prepend("( ID: ");
    idString.append(" )");
    m_idLabel->setText(idString);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5BaseView::delayedSetCentralWidget(QWidget* widget)
{
    setCentralWidget(widget);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5BaseView::hasSplitter() const
{
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
