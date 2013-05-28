#include "Qt5SourceCodeView.h"
#include "Qt5MainWindow.h"

namespace prodbg
{

Qt5SourceCodeViewContextMenu::Qt5SourceCodeViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent)
: Qt5DynamicViewContextMenu(mainWindow, parent)
{
}

Qt5SourceCodeViewContextMenu::~Qt5SourceCodeViewContextMenu()
{
}

Qt5SourceCodeView::Qt5SourceCodeView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5DynamicView* parent)
: Qt5BaseView(mainWindow, dock, parent)
{
	m_type = Qt5ViewType_SourceCode;

	focusInEvent(nullptr);
	
	m_sourceCode = new Qt5CodeEditor(parent);
    setCentralWidget(m_sourceCode);

    connect(parent, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));
    connect(m_sourceCode, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenuProxy(const QPoint&)));

    m_sourceCode->setFocusProxy(this);

    m_sourceCode->readSourceFile("examples/Fake6502/Fake6502Main.c");
    //m_sourceCode->beginDebug("tundra-output/macosx-clang-debug-default/Fake6502");
}

Qt5SourceCodeView::~Qt5SourceCodeView()
{
	disconnect();
	
	// Reset Focus Tracking (for safety)
	m_mainWindow->setCurrentWindow(nullptr, Qt5ViewType_Reset);

	centralWidget()->deleteLater();
    emit signalDelayedSetCentralWidget(nullptr);
}

void Qt5SourceCodeView::buildLayout()
{

}

void Qt5SourceCodeView::applyLayout(Qt5Layout* layout)
{
	(void)layout;
}

}