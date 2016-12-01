#include "MainWindow.h"
#include "MemoryView/MemoryViewWidget.h"
#include "CodeView/CodeView.h"
#include <QDockWidget>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow() : 
	m_codeView(new CodeView),
	m_memoryView(new MemoryViewWidget)
{
    char* dummy_data = (char*)malloc(1024 * 1024);

    for (int i = 0; i < 1024 * 1024; ++i)
        dummy_data[i] = (char)rand();

    m_memoryView->setData(QByteArray(dummy_data, 1024 * 1024));

	setCentralWidget(m_codeView);

	setWindowTitle(tr("ProDBG"));

	QDockWidget *dock = new QDockWidget(tr("MemoryView"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);

    dock->setWidget(m_memoryView);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_codeView->readSourceFile("src/prodbg/main.cpp");

    resize(1024, 768);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
