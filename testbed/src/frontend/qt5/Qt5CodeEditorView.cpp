#include "Qt5CodeEditorView.h"

namespace prodbg
{

Qt5CodeEditorViewContextMenu::Qt5CodeEditorViewContextMenu(Qt5MainWindow* mainWindow, Qt5BaseView* parent)
: Qt5DynamicViewContextMenu(mainWindow, parent)
{
}

Qt5CodeEditorViewContextMenu::~Qt5CodeEditorViewContextMenu()
{
}

Qt5CodeEditorView::Qt5CodeEditorView(Qt5MainWindow* mainWindow, Qt5DockWidget* dock, Qt5BaseView* parent)
: Qt5BaseView(mainWindow, dock, parent)
{
	m_codeEditor = new CodeEditor();
	setCentralWidget(m_codeEditor);

	m_codeEditor->readSourceFile("examples/Fake6502/Fake6502Main.c");
}

Qt5CodeEditorView::~Qt5CodeEditorView()
{
	centralWidget()->deleteLater();
    emit signalDelayedSetCentralWidget(nullptr);
}

}
