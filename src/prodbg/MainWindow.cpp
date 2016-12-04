#include "MainWindow.h"
#include "CodeView/CodeView.h"

#include "AmigaUAE/AmigaUAE.h"
#include "Backend/BackendHandler.h"
#include "Backend/BackendRequests.h"
#include "Config/AmigaUAEConfig.h"
#include "MemoryView/MemoryView.h"
#include "RegisterView/RegisterView.h"
#include "Session/Session.h"

#include <QMainWindow>
#include <QThread>
#include <QtCore/QSettings>
#include <QtWidgets/QDockWidget>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
    : m_codeView(new CodeView(this))
    , m_memoryView(new MemoryView(this))
    , m_registerView(new RegisterView(this))
    , m_statusbar(new QStatusBar(this))
    , m_backend(nullptr)
//, m_currentSession(nullptr)
//, m_amigaUae(nullptr)
{
    qRegisterMetaType<uint64_t>("uint64_t");

    m_ui.setupUi(this);

    setCentralWidget(m_codeView);

    setWindowTitle(QStringLiteral("ProDBG"));

    // Setup docking for MemoryView

    {
        QDockWidget* dock = new QDockWidget(QStringLiteral("MemoryView"), this);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        dock->setObjectName(QStringLiteral("MemoryViewDock"));
        dock->setWidget(m_memoryView);
        addDockWidget(Qt::RightDockWidgetArea, dock);
    }

    // Setup docking for RegisterView

    {
        QDockWidget* dock = new QDockWidget(QStringLiteral("RegisterView"), this);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        dock->setObjectName(QStringLiteral("RegisterViewDock"));
        dock->setWidget(m_registerView);
        addDockWidget(Qt::BottomDockWidgetArea, dock);
    }

    m_codeView->readSourceFile(QStringLiteral("src/prodbg/main.cpp"));

    m_statusbar->showMessage(tr("Ready."));

    setStatusBar(m_statusbar);

    startDummyBackend();

    initActions();
    readSettings();

    (void)m_backend;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
    if (m_backendThread) {
        m_backendThread->quit();
        m_backendThread->wait();
    }

    delete m_backendThread;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::initActions()
{
    connect(m_ui.actionStart, &QAction::triggered, this, &MainWindow::start);
    connect(m_ui.actionAmiga_UAE, &QAction::triggered, this, &MainWindow::amigaUAEConfig);
    connect(m_ui.actionDebugAmigaExe, &QAction::triggered, this, &MainWindow::debugAmigaExe);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::start()
{
    printf("start\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::amigaUAEConfig()
{
    AmigaUAEConfig cfg(this);
    cfg.setModal(true);
    cfg.exec();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::debugAmigaExe()
{
    AmigaUAE amigaUae;

    if (amigaUae.validateSettings()) {
        return;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::startDummyBackend()
{
    // TODO: Create sessino here also

    m_backendThread = new QThread;
    m_backend = new BackendHandler(0);

    m_backend->moveToThread(m_backendThread);
    m_backendRequests = new BackendRequests(m_backend);

    m_registerView->setBackendInterface(m_backendRequests);

    printf("Debugger thread started\n");

    m_backendThread->start();
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
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    settings.beginGroup(QStringLiteral("MainWindow"));
    settings.setValue(QStringLiteral("size"), size());
    settings.setValue(QStringLiteral("pos"), pos());
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
    settings.setValue(QStringLiteral("windowState"), saveState());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::readSettings()
{
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    settings.beginGroup(QStringLiteral("MainWindow"));
    restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
    restoreState(settings.value(QStringLiteral("windowState")).toByteArray());
    resize(settings.value(QStringLiteral("size"), QSize(800, 600)).toSize());
    move(settings.value(QStringLiteral("pos"), QPoint(100, 100)).toPoint());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
