#include "Qt5DynamicView.h"
#include "Qt5MainWindow.h"
#include "Qt5BaseView.h"
#include "Qt5DockWidget.h"

#include <QSplitter>
#include <QLabel>
namespace prodbg
{

Qt5DynamicViewContextMenu::Qt5DynamicViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent)
: Qt5ContextMenu(mainWindow, parent)
{
	addSeparator();
	addAction(m_mainWindow->m_dynamicWindowAssignViewMenu->menuAction());
	addAction(m_mainWindow->m_windowSplitVerticallyAction);
	addAction(m_mainWindow->m_windowSplitHorizontallyAction);
	addAction(m_mainWindow->m_windowFillMainWindowAction);
	addAction(m_mainWindow->m_windowUnfillMainWindowAction);
	addAction(m_mainWindow->m_windowDeleteViewAction);
}

Qt5DynamicViewContextMenu::~Qt5DynamicViewContextMenu()
{
}

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

	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
	connect(m_statusLabel, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
	m_statusLabel->setFocusProxy(this);
}

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

		// GW-TODO:
		/*
		Qt5Setting* splitterSetting = m_currentSettings->getSetting(Qt5Setting_SplitterOpaque);
		if (splitterSetting != nullptr)
		{
			Qt5SettingArgument* argument = m_currentSettings->getArgument(splitterSetting, 0);
			m_splitter->setOpaqueResize((bool)argument->data);
		}
		*/

		Qt5DynamicView* newViewOrig = new Qt5DynamicView(m_mainWindow, nullptr, this);
		if (m_children[0] != nullptr)
		{
			newViewOrig->assignView(m_children[0]);
			newViewOrig->m_children[0] = m_children[0];
			m_children[0]->setParent(newViewOrig);
			m_children[0]->m_parent = newViewOrig; // GW-TODO: Huh?
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

		m_splitter->addWidget(m_children[0]);
		m_splitter->addWidget(m_children[1]);

		m_splitter->setStretchFactor(0, 0);
		m_splitter->setStretchFactor(1, 0);

		emit signalDelayedSetCentralWidget(m_splitter);
	}
}

void Qt5DynamicView::assignView(Qt5BaseView* view)
{
	if (m_splitter != nullptr)
		return;

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

void Qt5DynamicView::applySettings()
{

}

void Qt5DynamicView::buildLayout()
{

}

void Qt5DynamicView::applyLayout(Qt5Layout* layout)
{
	(void)layout;
}

}