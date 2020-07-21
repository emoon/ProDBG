#include "MainWindow.h"
#include "CodeView/CodeView.h"
#include "RecentExecutables.h"

#include "AmigaUAE/AmigaUAE.h"
#include "Backend/BackendRequests.h"
#include "Backend/BackendSession.h"
#include "BreakpointModel.h"
#include "CodeViews.h"
#include "Config/AmigaUAEConfig.h"
#include "Core/PluginHandler.h"
#include "MemoryView/MemoryView.h"
#include "PluginUI/PluginUI_internal.h"
//#include "RegisterView/RegisterView.h"
#include "ViewHandler.h"
#include "toolwindowmanager/ToolWindowManager.h"

// Dialogs
#include "dialogs/PrefsDialog.h"

#include <QtCore/QDebug>
#include <QtCore/QIODevice>
#include <QtCore/QPluginLoader>
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMainWindow>
#include "SourceCodeWidget.h"
#include "edbee/texteditorwidget.h"
/*
#include "edbee/edbee.h"
#include "edbee/io/textdocumentserializer.h"
#include "edbee/models/textdocument.h"
#include "edbee/models/texteditorconfig.h"
#include "edbee/models/textgrammar.h"
#include "edbee/views/texteditorscrollarea.h"
 */

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
    : m_memory_view(new MemoryView(this)),
      // m_registerView(new RegisterView(this)),
      m_statusbar(new QStatusBar(this)),
      m_backend(nullptr)
//, m_currentSession(nullptr)
//, m_amigaUae(nullptr)
{
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<IBackendRequests::ProgramCounterChange>("IBackendRequests::ProgramCounterChange");

    m_view_handler = new ViewHandler(this);

    // TODO: Test load registers view plugin
    m_reg_plugin_loader = new QPluginLoader(QStringLiteral("registers_view"), this);
    m_register_view = qobject_cast<PDUIInterface*>(m_reg_plugin_loader->instance());

    // m_viewHandler->addView(new MemoryView(this));
    // m_viewHandler->addView(new RegisterView(this));

    m_recent_executables = new RecentExecutables;
    //m_amigaUae = new AmigaUAE(this);

    m_ui.setupUi(this);

    m_breakpoints = new BreakpointModel;

    // m_codeViews = new CodeViews(m_breakpoints, this);
    // m_codeViews->openFile(QStringLiteral("src/prodbg/main.cpp"), true);
    // m_codeViews->openFile(QStringLiteral("src/prodbg/main.cpp"),
    // m_breakpoints);
    // m_codeViews->openFile(QStringLiteral("src/prodbg/main.cpp"),
    // m_breakpoints);

    setWindowTitle(QStringLiteral("ProDBG"));

#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    // m_ui.toolWindowManager->setRubberBandLineWidth(50);

    // PluginInstance* inst = PluginUI_createTestPlugin(this);

    /*
       edbee::TextEditorWidget* editor = new edbee::TextEditorWidget(this);
       edbee::TextDocumentSerializer serializer(editor->textDocument());
    //QString filename = QStringLiteral("/home/emoon/code/projects/hippo_player/src/hippo_core/core/src/lib.rs");
    QString filename = QStringLiteral("src/prodbg/Config/Config.cpp");
    QFile file(filename);
    if (!serializer.load(&file)) {
    qDebug() << "failed to load file";
    }

    auto grammar_manager = edbee::Edbee::instance()->grammarManager();
    auto grammar = grammar_manager->detectGrammarWithFilename(filename);
    qDebug() << "grammar detected " << grammar;
    editor->textDocument()->setLanguageGrammar(grammar);
    editor->textScrollArea()->enableShadowWidget(false);
    editor->textDocument()->config()->setFont(font);
     */

    m_source_view = new SourceCodeWidget(m_breakpoints, this);
    m_source_view->load_file(QStringLiteral("src/prodbg/Config/Config.cpp"));

    setCentralWidget(m_source_view->m_editor);

    // QWidget* plugin_parent = new QWidget(this);
    // setCentralWidget(m_memoryView);

    // PluginHandler_tempLoadUIPlugin(plugin_parent, QStringLiteral("memory_view_2"));

    // Setup docking for MemoryView

    {
        // m_ui.toolWindowManager->addToolWindow(m_codeViews, ToolWindowManager::EmptySpace);
        // m_ui.toolWindowManager->addToolWindow(m_memoryView, ToolWindowManager::LastUsedArea);
        // m_ui.toolWindowManager->addToolWindow(m_registerView, ToolWindowManager::LastUsedArea);
        // m_ui.toolWindowManager->addToolWindow(plugin_parent, ToolWindowManager::LastUsedArea);

        // QDockWidget* dock = new QDockWidget(QStringLiteral("MemoryView"),
        // this); dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        // dock->setObjectName(QStringLiteral("MemoryViewDock"));
        // dock->setWidget(m_memoryView);
        // addDockWidget(Qt::RightDockWidgetArea, dock);
    }

    // Setup docking for RegisterView

    {
        // QDockWidget* dock = new QDockWidget(QStringLiteral("RegisterView"),
        // this); dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        // dock->setObjectName(QStringLiteral("RegisterViewDock"));
        // dock->setWidget(m_registerView);
        // addDockWidget(Qt::BottomDockWidgetArea, dock);
    }

    m_statusbar->showMessage(tr("Ready."));

    setStatusBar(m_statusbar);

    start_dummy_backend();

    init_actions();
    read_settings();

    init_recent_file_actions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow() {
    delete m_recent_executables;

    close_current_backend();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::init_actions() {
    /*
       connect(m_ui.actionStart, &QAction::triggered, this, &MainWindow::startDebug);
       connect(m_ui.actionReloadCurrentFile, &QAction::triggered, this, &MainWindow::reloadCurrentFile);
       connect(m_ui.actionStep_Over, &QAction::triggered, this, &MainWindow::stepOver);
       connect(m_ui.actionStep_In, &QAction::triggered, this, &MainWindow::stepIn);
       connect(m_ui.actionAmiga_UAE, &QAction::triggered, this, &MainWindow::amigaUAEConfig);
       connect(m_ui.actionDebugAmigaExe, &QAction::triggered, this, &MainWindow::debugAmigaExe);
       connect(m_ui.actionToggleBreakpoint, &QAction::triggered, this, &MainWindow::toggleBreakpoint);
       connect(m_ui.actionOpen, &QAction::triggered, this, &MainWindow::openSourceFile);
       connect(m_ui.actionBreak, &QAction::triggered, this, &MainWindow::breakContDebug);
       connect(m_ui.actionStop, &QAction::triggered, this, &MainWindow::stop);
       connect(m_ui.actionToggleSourceAsm, &QAction::triggered, m_codeViews, &CodeViews::toggleSourceAsm);
       connect(m_ui.actionMemoryView, &QAction::triggered, this, &MainWindow::newMemoryView);
       connect(m_ui.actionPreferences, &QAction::triggered, this, &MainWindow::show_prefs);
     */

    // connect(m_ui.actionRegisterView, &QAction::triggered, this, &MainWindow::newRegisterView);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::init_recent_file_actions() {
    QMenu* menu = m_ui.menuRecentExecutables;

    // QAction* recentFiles = m_ui.menuFile->actionRecentExecutables;

    for (int i = 0; i < RecentExecutables::MaxFiles_Count; ++i) {
        QAction* recentFileAction = new QAction(this);
        recentFileAction->setVisible(false);
        connect(recentFileAction, &QAction::triggered, this, &MainWindow::open_recent_exe);
        m_recent_file_actions.append(recentFileAction);
        menu->addAction(recentFileAction);
    }

    m_recent_executables->update_action_list(m_recent_file_actions);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::open_source_file() {
    QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open Source File"));

    if (path.isEmpty()) {
        return;
    }

    // m_codeViews->openFile(path, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::show_prefs() {
    auto dialog = new PrefsDialog;
    dialog->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::start() {
    printf("MainWindow::start\n");

    // const QVector<BreakpointModel::FileLineBreakpoint>& fileLineBreakpoints =
    // m_breakpoints->get_file_line_breakpoints(); const QVector<uint64_t>& addressBreakpoints =
    // m_breakpoints->get_address_breakpoints();

    // add the breakpoints before we start

    /*
       for (auto& bp : fileLineBreakpoints) {
       m_backendRequests->beginAddFileLineBreakpoint(bp.filename, bp.line);
       }

       for (auto& bp : addressBreakpoints) {
       m_backendRequests->beginAddAddressBreakpoint(bp);
       }
     */

    // startBackend();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::break_cont_debug() {
    break_cont_backend();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void MainWindow::amigaUAEConfig() {
    AmigaUAEConfig cfg(this);
    cfg.setModal(true);
    cfg.exec();
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::uae_started() {
    //startAmigaUAEBackend();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void MainWindow::debug_amiga_exe() {
    if (!m_amigaUae->openFile()) {
        return;
    }

    m_lastAmigaExe = m_amigaUae->m_localExeToRun;

    m_recentExecutables->setFile(m_recentFileActions, m_lastAmigaExe, BackendType_AmigaUAE);

    internalStartAmigaExe();
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::open_recent_exe() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString filename = action->data().toString();
        //m_lastAmigaExe = filename;
        //m_amigaUae->m_localExeToRun = filename;

        m_recent_executables->put_file_on_top(m_recent_file_actions, filename);

        //internalStartAmigaExe();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::start_debug() {
    /*
    if (m_current_backend == Amiga && !m_lastAmigaExe.isEmpty()) {
        internalStartAmigaExe();
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::reload_current_file() {
    //m_codeViews->reloadCurrentFile();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

void MainWindow::internal_start_amiga_exe() {
    m_amigaUae->m_localExeToRun = m_lastAmigaExe;

    if (!m_amigaUae->validateSettings()) {
        return;
    }

    m_lastAmigaExe = m_amigaUae->m_localExeToRun;

    /*
       if (!m_amigaUae->m_skipUAELaunch) {
       connect(m_amigaUae->m_uaeProcess, &QProcess::started, this, &MainWindow::uaeStarted);
       connect(m_amigaUae->m_uaeProcess, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this,
       &MainWindow::processEnded);

       printf("BackendSession::destroyPluginData\n");

       m_amigaUae->launchUAE();
       } else {
       startAmigaUAEBackend();
       }
     */
}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::process_ended(int) {
    stop_internal();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setup_backend_connections() {
    /*
       connect(this, &MainWindow::stepInBackend, m_backend, &BackendSession::stepIn);
       connect(this, &MainWindow::stepOverBackend, m_backend, &BackendSession::stepOver);
       connect(this, &MainWindow::startBackend, m_backend, &BackendSession::start);
       connect(this, &MainWindow::breakContBackend, m_backend, &BackendSession::breakContDebug);
       connect(this, &MainWindow::stopBackend, m_backend, &BackendSession::stop);
       connect(m_backend, &BackendSession::statusUpdate, this, &MainWindow::statusUpdate);
     */

    connect(m_backend_thread, &QThread::finished, m_backend, &BackendSession::thread_finished);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::start_dummy_backend() {
    BackendSession* backend = BackendSession::create_backend_session(QStringLiteral("Dummy Backend"));

    if (!backend) {
        qDebug() << "Unable to create Dummy Backend";
        return;
    }

    setup_backend(backend);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::close_current_backend() {
    if (m_backend_thread) {
        m_backend_thread->quit();
        m_backend_thread->wait();
    }

    delete m_backend_thread;
    delete m_backend_requests;

    if (m_register_view) {
        m_register_view->set_backend_interface(nullptr);
    }

    // m_registerView->set_backend_interface(nullptr);
    // m_codeViews->set_backend_interface(nullptr);
    // m_memoryView->set_backend_interface(nullptr);

    m_backend = nullptr;
    m_backend_thread = nullptr;
    m_backend_requests = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void MainWindow::startAmigaUAEBackend() {
    closeCurrentBackend();

    BackendSession* backend = BackendSession::create_backend_session(QStringLiteral("Amiga UAE Debugger"));

    if (!backend) {
        printf("Unable to create Amiga UAE Debugger backend\n");
        return;
    }

    setupBackend(backend);

    // m_backendRequests->sendCustomString(m_amigaUae->m_setFileId, m_amigaUae->m_fileToRun);
    // m_backendRequests->sendCustomString(m_amigaUae->m_setHddPathId, m_amigaUae->m_dh0Path);

    // TODO: This is really temporary
    // QTimer::singleShot(5000, this, &MainWindow::start);

    printf("startAmigaUAEBackend: NOT IMPLEMENTED\n");

    m_currentBackend = Amiga;
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setup_backend(BackendSession* backend) {
    m_backend = backend;

    m_backend_thread = new QThread;

    m_backend->moveToThread(m_backend_thread);
    m_backend_requests = new BackendRequests(m_backend);

    // m_registerView->set_backend_interface(m_backendRequests);
    // m_codeViews->set_backend_interface(m_backendRequests);
    //m_memory_view->set_backend_interface(m_backend_requests);

    m_backend_thread->start();

    setup_backend_connections();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::stop_internal() {
    stop_backend();
    close_current_backend();

    m_statusbar->showMessage(QStringLiteral("Ready."));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::stop() {
    /*
    if (m_currentBackend == Amiga) {
        m_amigaUae->killProcess();
    } else {
        stopInternal();
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::step_in() {
    step_in_backend();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::step_over() {
    step_over_backend();

    printf("stepOver\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::toggle_breakpoint() {
    m_source_view->toggle_breakpoint_current_line();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void MainWindow::new_memory_view() {
    MemoryView* mv = new MemoryView(this);
    mv->set_backend_interface(m_backendRequests);
    QDockWidget* dock = new QDockWidget(QStringLiteral("MemoryView"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setObjectName(QStringLiteral("MemoryViewDock"));
    dock->setWidget(mv);
    addDockWidget(Qt::TopDockWidgetArea, dock);
    dock->setFloating(true);
    m_viewHandler->addView(mv);
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Q_SLOT void MainWindow::new_register_view() {
    /*
    //RegisterView* rg = new RegisterView(this);
    rg->set_backend_interface(m_backendRequests);
    QDockWidget* dock = new QDockWidget(QStringLiteral("RegisteView"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setObjectName(QStringLiteral("RegisteViewDock"));
    dock->setWidget(rg);
    addDockWidget(Qt::TopDockWidgetArea, dock);
    dock->setFloating(true);

    m_viewHandler->addView(rg);
     */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::status_update(const QString& status) {
    m_statusbar->showMessage(status);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::closeEvent(QCloseEvent* event) {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    write_settings();

    /*
    if (m_amigaUae) {
        m_amigaUae->writeSettings();
    }
    */

    event->accept();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::write_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    settings.beginGroup(QStringLiteral("MainWindow"));

    /*
       settings.beginWriteArray(QStringLiteral("views"));

       int index = 0;

       QList<QDockWidget*> docs = findChildren<QDockWidget*>(QString());
       for (auto& doc : docs) {
       View* view = qobject_cast<View*>(doc->widget());

       if (view && view->isVisible()) {
    // TODO: Write settings here
    settings.setArrayIndex(index);
    settings.setValue(QStringLiteral("type"),
    QString::fromUtf8(view->metaObject()->className())); index += 1;
    }
    }

    settings.endArray();
     */

    settings.setValue(QStringLiteral("size"), size());
    settings.setValue(QStringLiteral("pos"), pos());
    settings.setValue(QStringLiteral("geometry"), saveGeometry());
    settings.setValue(QStringLiteral("windowState"), saveState());

    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::read_settings() {
    QSettings settings(QStringLiteral("TBL"), QStringLiteral("ProDBG"));

    settings.beginGroup(QStringLiteral("MainWindow"));

    /*
       int size = settings.beginReadArray(QStringLiteral("views"));

       for (int i = 0; i < size; ++i) {
       settings.setArrayIndex(i);
       QString viewType = settings.value(QStringLiteral("type")).toString();

       View* view = nullptr;

    // hack for now
    if (viewType == QStringLiteral("prodbg::MemoryView")) {
    view = new MemoryView(this);
    } else if (viewType == QStringLiteral("prodbg::RegisterView")) {
    view = new RegisterView(this);
    } else {
    Q_ASSERT(view);
    }

    QDockWidget* dock = new QDockWidget(viewType, this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setObjectName(viewType + QStringLiteral("Dock"));
    dock->setWidget(view);
    addDockWidget(Qt::RightDockWidgetArea, dock);
    }

    settings.endArray();
     */

    restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
    restoreState(settings.value(QStringLiteral("windowState")).toByteArray());
    resize(settings.value(QStringLiteral("size"), QSize(800, 600)).toSize());
    move(settings.value(QStringLiteral("pos"), QPoint(100, 100)).toPoint());
    settings.endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
