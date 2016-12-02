#include "MainWindow.h"
#include "CodeView/CodeView.h"
#include "Config/AmigaUAEConfig.h"
#include "MemoryView/MemoryViewWidget.h"
#include <QDockWidget>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
    : m_codeView(new CodeView)
    , m_memoryView(new MemoryViewWidget)
    , m_statusbar(new QStatusBar)
{
    char* dummy_data = (char*)malloc(1024 * 1024);

    for (int i = 0; i < 1024 * 1024; ++i)
        dummy_data[i] = (char)rand();

    m_memoryView->setData(QByteArray(dummy_data, 1024 * 1024));

    m_ui.setupUi(this);

    setCentralWidget(m_codeView);

    setWindowTitle(tr("ProDBG"));

    QDockWidget* dock = new QDockWidget(tr("MemoryView"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);

    dock->setObjectName(tr("MemoryViewDock"));

    dock->setWidget(m_memoryView);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_codeView->readSourceFile("src/prodbg/main.cpp");

    m_statusbar->showMessage(tr("Ready."));

    setStatusBar(m_statusbar);

    initActions();
    readSettings();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::initActions()
{
    connect(m_ui.actionStart, &QAction::triggered, this, &MainWindow::start);
    connect(m_ui.actionAmiga_UAE, &QAction::triggered, this, &MainWindow::amigaUAEConfig);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::start()
{
    printf("start\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::amigaUAEConfig()
{
    AmigaUAEConfig* cfg = new AmigaUAEConfig();
    cfg->setModal(true);
    cfg->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::stop()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::stepIn()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::stepOver()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::toggleBreakpoint()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::closeEvent(QCloseEvent* event)
{
    writeSettings();
    event->accept();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::writeSettings()
{
    QSettings settings("TBL", "ProDBG");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::readSettings()
{
    QSettings settings("TBL", "ProDBG");

    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    resize(settings.value("size", QSize(800, 600)).toSize());
    move(settings.value("pos", QPoint(100, 100)).toPoint());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
