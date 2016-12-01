#include "MainWindow.h"
#include "MemoryView/MemoryViewWidget.h"
#include "CodeView/CodeView.h"

MainWindow::MainWindow() : 
	m_memoryView(new MemoryViewWidget),
	m_codeView(new CodeView),
{
}
