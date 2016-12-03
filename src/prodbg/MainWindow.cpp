#include "MainWindow.h"
#include "CodeView/CodeView.h"

#include "AmigaUAE/AmigaUAE.h"
#include "Config/AmigaUAEConfig.h"
#include "MemoryView/MemoryView.h"
#include "Session/Session.h"

#include <QTimer>
#include <QtCore/QSettings>
#include <QtWidgets/QDockWidget>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
    : m_codeView(new CodeView(this))
    , m_memoryView(new MemoryView(this))
    , m_statusbar(new QStatusBar(this))
    , m_timer(new QTimer(this))
    , m_currentSession(nullptr)
    , m_amigaUae(nullptr)
{
    m_ui.setupUi(this);

    setCentralWidget(m_codeView);

    setWindowTitle(QStringLiteral("ProDBG"));

    QDockWidget* dock = new QDockWidget(QStringLiteral("MemoryView"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);

    dock->setObjectName(QStringLiteral("MemoryViewDock"));

    dock->setWidget(m_memoryView);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_codeView->readSourceFile(QStringLiteral("src/prodbg/main.cpp"));

    m_statusbar->showMessage(tr("Ready."));

    setStatusBar(m_statusbar);

    m_currentSession = Session::createSession(QStringLiteral("Dummy Backend"), m_statusbar);

    initActions();
    readSettings();

    (void)m_amigaUae;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::initActions()
{
    connect(m_ui.actionStart, &QAction::triggered, this, &MainWindow::start);
    connect(m_ui.actionAmiga_UAE, &QAction::triggered, this, &MainWindow::amigaUAEConfig);
    connect(m_ui.actionDebugAmigaExe, &QAction::triggered, this, &MainWindow::debugAmigaExe);

    // TODO: We really don't need to have this running all the time but will do for now
    connect(m_timer, &QTimer::timeout, this, &MainWindow::timedUpdate);

    m_timer->start(10); // update every 10 ms
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::start()
{
    printf("start\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::timedUpdate()
{
    if (m_currentSession) {
        m_currentSession->update();
    }
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
