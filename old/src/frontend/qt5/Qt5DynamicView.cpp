#include "Qt5DynamicView.h"
#include "Qt5MainWindow.h"
#include "Qt5BaseView.h"
#include "Qt5DockWidget.h"
#include <QSplitter>
#include <QLabel>

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DynamicViewContextMenu::Qt5DynamicViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent)
    : Qt5ContextMenu(mainWindow, parent)
{
    addSeparator();
    addAction(m_mainWindow->m_dynamicWindowAssignViewMenu->menuAction());
    addAction(m_mainWindow->m_dynamicWindowSplitHorizontalViewMenu->menuAction());
    addAction(m_mainWindow->m_dynamicWindowSplitVerticalViewMenu->menuAction());
    addAction(m_mainWindow->m_windowFillMainWindowAction);
    addAction(m_mainWindow->m_windowUnfillMainWindowAction);
    addAction(m_mainWindow->m_windowDeleteViewAction);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DynamicViewContextMenu::~Qt5DynamicViewContextMenu()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DynamicView::Qt5DynamicView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5BaseView* parent)
    : Qt5BaseView(mainWindow, dock, parent)
    , m_splitter(nullptr)
{
    m_type = Qt5ViewType_Dynamic;

    m_children[0] = nullptr;
    m_children[1] = nullptr;

    focusInEvent(nullptr);

    resize(512, 448);
    setFocusPolicy(Qt::WheelFocus);
    setContextMenuPolicy(Qt::CustomContextMenu);

    m_statusLabel = new QLabel;
    m_statusLabel->setFocusPolicy(Qt::WheelFocus);
    m_statusLabel->setContextMenuPolicy(Qt::CustomContextMenu);
    m_statusLabel->move(2, 2);
    m_statusLabel->resize(510, 444);
    m_statusLabel->setAutoFillBackground(true);
    m_statusLabel->setText(tr("Unassigned View"));
    m_statusLabel->setAlignment(Qt::AlignCenter);

    setCentralWidget(m_statusLabel);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuProxy(const QPoint &)));
    connect(m_statusLabel, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(contextMenuProxy(const QPoint &)));
    m_statusLabel->setFocusProxy(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DynamicView::~Qt5DynamicView()
{
    disconnect();

    // Reset focus tracking
    m_mainWindow->setCurrentWindow(nullptr, Qt5ViewType_Reset);
    if (m_splitter != nullptr)
    {
        m_splitter->deleteLater();
        m_splitter = nullptr;
    }

    centralWidget()->deleteLater();
    emit signalDelayedSetCentralWidget(nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DynamicView::splitView(Qt::Orientation orientation)
{
    if (m_splitter == nullptr)
    {
        if (m_children[0] == nullptr && m_statusLabel != nullptr)
        {
            m_statusLabel->hide();
            m_statusLabel->deleteLater();
            m_statusLabel = nullptr;
        }

        m_splitter = new QSplitter(orientation, this);

        Qt5Setting* splitterSetting = g_settings->getSetting(Qt5SettingId_OpaqueSplitter);
        if (splitterSetting != nullptr)
        {
            Qt5SettingArgument* argument = g_settings->getArgument(splitterSetting, 0);
            m_splitter->setOpaqueResize((bool)argument->dataPointer);
        }

        Qt5DynamicView* newViewOrig = new Qt5DynamicView(m_mainWindow, nullptr, this);
        if (m_children[0] != nullptr)
        {
            newViewOrig->assignView(m_children[0]);
            newViewOrig->m_children[0] = m_children[0];
            m_children[0]->setParent(newViewOrig);
            m_children[0]->m_parent = newViewOrig;
            m_children[0]->m_parentDock = nullptr;
        }
        else
        {
            emit newViewOrig->signalDelayedSetCentralWidget(newViewOrig->m_statusLabel);
        }

        m_children[0] = newViewOrig;

        Qt5DynamicView* newView = new Qt5DynamicView(m_mainWindow, nullptr, this);
        m_children[1] = newView;

        emit newView->signalDelayedSetCentralWidget(newView->m_statusLabel);

        Q_ASSERT(m_children[0] != nullptr);
        Q_ASSERT(m_children[1] != nullptr);

        m_splitter->addWidget(m_children[0]);
        m_splitter->addWidget(m_children[1]);

        m_splitter->setStretchFactor(0, 0);
        m_splitter->setStretchFactor(1, 0);

        printf("GWDEBUG: Qt5DynamicView::splitView - Splitter count: %d\n", m_splitter->count());

        emit signalDelayedSetCentralWidget(m_splitter);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DynamicView::assignView(Qt5BaseView* view)
{
    if (m_splitter != nullptr)
    {
        printf("GWDEBUG: Qt5DynamicView::assignView - Splitter count: %d\n", m_splitter->count());
        QList<int32> it = m_splitter->sizes();
        printf("GWDEBUG: Qt5DynamicView::assignView - Splitter size test: %d\n", it.count());

        return;
    }

    if (m_statusLabel != nullptr)
    {
        m_statusLabel->hide();
        m_statusLabel->deleteLater();
        m_statusLabel = nullptr;
    }

    emit signalDelayedSetCentralWidget(view);
    view->m_parent = this;
    view->setParent(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DynamicView::applySettings()
{
    printf("Qt5DynamicView::applySettings\n");

    if (m_splitter == nullptr)
        return;

    Qt5Setting* splitterSetting = g_settings->getSetting(Qt5SettingId_OpaqueSplitter);
    if (splitterSetting != nullptr)
    {
        Qt5SettingArgument* argument = g_settings->getArgument(splitterSetting, 0);
        m_splitter->setOpaqueResize((bool)argument->dataPointer);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DynamicView::buildLayout()
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

    if (m_children[0])
        entry.child1 = m_children[0]->m_id;
    else
        entry.child1 = 0;

    if (m_children[1])
        entry.child2 = m_children[1]->m_id;
    else
        entry.child2 = 0;

    if (m_parentDock)
    {
        entry.isFloating = m_parentDock->isFloating();
        if (entry.isFloating)
        {
            entry.dockPositionX = m_parentDock->pos().x();
            entry.dockPositionY = m_parentDock->pos().y();
            entry.dockSizeX = m_parentDock->width();
            entry.dockSizeY = m_parentDock->height();
        }
        else
        {
            entry.dockPositionX = 0;
            entry.dockPositionY = 0;
            entry.dockSizeX = 0;
            entry.dockSizeY = 0;
        }
    }
    else
    {
        entry.isFloating = false;
    }

    if (m_splitter)
        entry.hasSplitter = true;
    else
        entry.hasSplitter = false;

    if (m_parent == m_mainWindow)
        entry.topLevel = true;
    else
        entry.topLevel = false;

    if (m_parentDock == nullptr && m_mainWindow->centralWidget() == this)
        entry.fillMainWindow = true;
    else
        entry.fillMainWindow = false;

    if (m_splitter)
    {
        printf("GWDEBUG: Qt5DynamicView::buildLayout - Splitter count test: %d\n", m_splitter->count());

        QList<int32> it = m_splitter->sizes();

        printf("GWDEBUG: Qt5DynamicView::buildLayout - Splitter size test: %d\n", it.count());


        entry.splitRegion1Size = it[0];
        entry.splitRegion2Size = it[1];
        entry.splitDirection = m_splitter->orientation();

        printf("GWDEBUG: Qt5DynamicView::buildLayout - direction: %s\n",
               entry.splitDirection == Qt::Horizontal ? "Horizontal" : "Vertical");
    }

    m_mainWindow->addLayout(&entry);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DynamicView::applyLayout(Qt5Layout* layout)
{
    (void)layout;

    printf("Qt5DynamicView::applyLayout\n");

    Qt5LayoutEntry* entry = &layout->entries[m_entry];

    if (m_parentDock)
    {
        if (entry->isFloating)
        {
            m_parentDock->setGeometry(entry->dockPositionX, entry->dockPositionY, entry->dockSizeX, entry->dockSizeY);
            m_parentDock->setFloating(true);
        }
        else
        {
            m_parentDock->setFloating(false);
        }
    }

    QPoint mpos = mapFromGlobal(QPoint(entry->positionX, entry->positionY));
    setGeometry(mpos.x(), mpos.y(), entry->sizeX, entry->sizeY);

    if (!m_splitter && m_children[0] && m_children[1])
    {
        if (m_statusLabel)
        {
            m_statusLabel->deleteLater();
            m_statusLabel = nullptr;
        }

        printf("GWDEBUG: Qt5DynamicView::applyLayout - direction: %s\n",
               entry->splitDirection == Qt::Horizontal ? "Horizontal" : "Vertical");

        m_splitter = new QSplitter(entry->splitDirection, this);

        Qt5Setting* splitterSetting = g_settings->getSetting(Qt5SettingId_OpaqueSplitter);
        if (splitterSetting != nullptr)
        {
            Qt5SettingArgument* argument = g_settings->getArgument(splitterSetting, 0);
            m_splitter->setOpaqueResize((bool)argument->dataPointer);
        }

        m_splitter->addWidget(m_children[0]);
        m_splitter->addWidget(m_children[1]);

        m_splitter->setStretchFactor(0, 0);
        m_splitter->setStretchFactor(1, 0);

        printf("GWDEBUG: Qt5DynamicView::applyLayout - Splitter count: %d\n", m_splitter->count());

        emit signalDelayedSetCentralWidget(m_splitter);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
