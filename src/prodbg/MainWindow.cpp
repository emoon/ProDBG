#include "MainWindow.h"
#include "CodeView/CodeView.h"
#include "RecentProjects.h"

#include "AmigaUAE/AmigaUAE.h"
#include "Backend/BackendRequests.h"
#include "Backend/BackendSession.h"
#include "BreakpointModel.h"
#include "CodeViews.h"
#include "Config/AmigaUAEConfig.h"
#include "Core/PluginHandler.h"
#include "PluginUI/PluginUI_internal.h"
//#include "RegisterView/RegisterView.h"
#include "ViewHandler.h"
#include "toolwindowmanager/ToolWindowManager.h"

// Dialogs
#include "FileBrowserView.h"
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

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow()
    : m_statusbar(new QStatusBar(this)),
      m_backend(nullptr)
{
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<IBackendRequests::ProgramCounterChange>("IBackendRequests::ProgramCounterChange");
    qRegisterMetaType<IBackendRequests::VariableData>("IBackendRequests::VariableData");
    qRegisterMetaType<IBackendRequests::Variables>("IBackendRequests::Variables");
    qRegisterMetaType<IBackendRequests::BasicRequest>("IBackendRequests::BasicRequest");
    qRegisterMetaType<IBackendRequests::CallstackEntry>("IBackendRequests::CallstackEntry");
    qRegisterMetaType<IBackendRequests::Callstack>("IBackendRequests::Callstack");
    qRegisterMetaType<IBackendRequests::ExpandVars>("IBackendRequests::ExpandVars");
    qRegisterMetaType<IBackendRequests::ExpandType>("IBackendRequests::ExpandType");
    qRegisterMetaType<QVector<QString>>("QVector<QString>");

    // Create the view handler and load all view plugins
    m_view_handler = new ViewHandler(this);
    m_view_handler->load_plugins(QCoreApplication::applicationDirPath());

    m_recent_projects = new RecentProjects;

    m_ui.setupUi(this);

    create_views_menu();

    m_breakpoints = new BreakpointModel;

    //m_code_views = new CodeViews(m_breakpoints, this);
    //m_code_views->open_file(QStringLiteral("/home/emoon/code/temp/debug_test/src/main.rs"), true);
    //m_code_views->open_file(QStringLiteral("src/prodbg/Config/Config.cpp"), true);

    // m_code_views->openFile(QStringLiteral("src/prodbg/main.cpp"),
    // m_breakpoints);
    // m_code_views->openFile(QStringLiteral("src/prodbg/main.cpp"),
    // m_breakpoints);

    setWindowTitle(QStringLiteral("ProDBG"));

#ifdef _WIN32
    QFont font(QStringLiteral("Courier"), 11);
#else
    QFont font(QStringLiteral("Courier"), 13);
#endif

    // m_ui.toolWindowManager->setRubberBandLineWidth(50);

    // PluginInstance* inst = PluginUI_createTestPlugin(this);

    auto temp = new QWidget(this);
    setCentralWidget(temp);
    temp->hide();

    // m_source_view = new SourceCodeWidget(m_breakpoints, this);
    // m_source_view->load_file(QStringLiteral("src/prodbg/Config/Config.cpp"));
    /*

    QDockWidget* dock = new QDockWidget(QStringLiteral("Source View"), this);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setWidget(m_code_views);
    // dock->setWidget(m_source_view->m_editor);
    //dock->setObjectName(QStringLiteral("SourceView"));

    addDockWidget(Qt::LeftDockWidgetArea, dock);

    m_file_browser = new FileBrowserView(nullptr);
    dock = new QDockWidget(QStringLiteral("File Browser"), this);
    dock->setWidget(m_file_browser);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setObjectName(QStringLiteral("FileBrowser"));
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_locals_view = new LocalsView(nullptr);
    dock = new QDockWidget(QStringLiteral("Locals"), this);
    dock->setWidget(m_locals_view);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setObjectName(QStringLiteral("Locals"));
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    m_callstack_view = new CallstackView(nullptr);
    dock = new QDockWidget(QStringLiteral("Callstack"), this);
    dock->setWidget(m_callstack_view);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);
    dock->setObjectName(QStringLiteral("Callstack"));
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    */

    // setCentralWidget(m_source_view->m_editor);

    // QWidget* plugin_parent = new QWidget(this);

    // PluginHandler_tempLoadUIPlugin(plugin_parent, QStringLiteral("memory_view_2"));

    // Setup docking for MemoryView

    {
        //m_ui.toolWindowManager->addToolWindow(m_code_views, ToolWindowManager::EmptySpace);
        //m_ui.toolWindowManager->addToolWindow(m_callstack_view, ToolWindowManager::LastUsedArea);
        //m_ui.toolWindowManager->addToolWindow(file_browser, ToolWindowManager::LastUsedArea);

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

    //start_dummy_backend();

    init_actions();
    //read_settings();

    init_recent_file_actions();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow() {
    delete m_recent_projects;

    close_current_backend();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::init_actions() {
    connect(m_ui.debug_file, &QAction::triggered, this, &MainWindow::open_debug_exe);
    connect(m_ui.actionPreferences, &QAction::triggered, this, &MainWindow::show_prefs);
    connect(m_ui.actionToggleBreakpoint, &QAction::triggered, this, &MainWindow::toggle_breakpoint);
    connect(m_ui.actionStep_In, &QAction::triggered, this, &MainWindow::step_in);

    /*
       connect(m_ui.actionStart, &QAction::triggered, this, &MainWindow::startDebug);
       connect(m_ui.actionReloadCurrentFile, &QAction::triggered, this, &MainWindow::reloadCurrentFile);
       connect(m_ui.actionStep_Over, &QAction::triggered, this, &MainWindow::stepOver);
       connect(m_ui.actionAmiga_UAE, &QAction::triggered, this, &MainWindow::amigaUAEConfig);
       connect(m_ui.actionDebugAmigaExe, &QAction::triggered, this, &MainWindow::debugAmigaExe);
       connect(m_ui.actionToggleBreakpoint, &QAction::triggered, this, &MainWindow::toggleBreakpoint);
       connect(m_ui.actionOpen, &QAction::triggered, this, &MainWindow::openSourceFile);
       connect(m_ui.actionBreak, &QAction::triggered, this, &MainWindow::breakContDebug);
       connect(m_ui.actionStop, &QAction::triggered, this, &MainWindow::stop);
       connect(m_ui.actionToggleSourceAsm, &QAction::triggered, m_code_views, &CodeViews::toggleSourceAsm);
       connect(m_ui.actionMemoryView, &QAction::triggered, this, &MainWindow::newMemoryView);
     */

    // connect(m_ui.actionRegisterView, &QAction::triggered, this, &MainWindow::newRegisterView);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::init_recent_file_actions() {
    QMenu* menu = m_ui.menu_recent_projects;

    for (int i = 0; i < RecentProjects::MaxProjects_Count; ++i) {
        QAction* recent_project_action = new QAction(this);
        recent_project_action->setVisible(false);
        connect(recent_project_action, &QAction::triggered, this, &MainWindow::open_recent_project);
        m_recent_project_actions.append(recent_project_action);
        menu->addAction(recent_project_action);
    }

    m_recent_projects->update_action_list(m_recent_project_actions);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::open_debug(bool stop_at_main) {
    QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open File for Debug..."));

    if (path.isEmpty()) {
        return;
    }

    //QString path = QStringLiteral("t2-output/linux-gcc-debug-default/crashing_native");

    close_current_backend();

    BackendSession* backend = BackendSession::create_backend_session(QStringLiteral("LLDB"));

    if (!backend) {
        printf("Unable to create LLDBBackend backend\n");
        return;
    }

    setup_backend(backend);

    // Request starting a file for debugging. target_reply will get a status back if it's possible
    // to start the file or not and the debugging can procedde after that
    m_backend_requests->file_target_request(stop_at_main, path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::open_debug_exe() {
    open_debug(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::open_debug_exe_stop_at_main() {
    open_debug(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::open_source_file() {
    QString path = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Open Source File"));

    if (path.isEmpty()) {
        return;
    }

    // m_code_views->openFile(path, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::show_prefs() {
    auto dialog = new PrefsDialog;
    dialog->show();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::start() {
    printf("MainWindow::start\n");

    auto file_line_breakpoints = m_breakpoints->get_file_line_breakpoints();

    // add the breakpoints before we start
    for (auto& bp : file_line_breakpoints) {
        m_backend_requests->request_add_file_line_breakpoint(bp.filename, bp.line);
    }

    /*
    for (auto& bp : addressBreakpoints) {
        m_backendRequests->beginAddAddressBreakpoint(bp);
    }
    */

    // signal the backend thread to start
    start_backend();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::break_cont_debug() {
    break_cont_backend();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::open_recent_project() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        QString name = action->data().toString();
        m_recent_projects->put_project_on_top(m_recent_project_actions, name);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void MainWindow::reload_current_file() {
    // m_code_views->reloadCurrentFile();
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::process_ended(int) {
    stop_internal();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setup_backend_connections() {
    connect(this, &MainWindow::start_backend, m_backend, &BackendSession::start);
    connect(this, &MainWindow::step_in_backend, m_backend, &BackendSession::step_in);

    connect(m_backend, &BackendSession::target_reply, this, &MainWindow::target_reply);
    //connect(m_file_browser, &FileBrowserView::open_file_signal, m_code_views, &CodeViews::open_file);

    /*
       connect(this, &MainWindow::stepOverBackend, m_backend, &BackendSession::stepOver);
       connect(this, &MainWindow::breakContBackend, m_backend, &BackendSession::breakContDebug);
       connect(this, &MainWindow::stopBackend, m_backend, &BackendSession::stop);
       connect(m_backend, &BackendSession::statusUpdate, this, &MainWindow::statusUpdate);
     */

    connect(m_backend_thread, &QThread::finished, m_backend, &BackendSession::thread_finished);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::target_reply(bool status, const QString& error_message) {
    // Got ok from backend! we are ready to got so start it, otherwise show the error and close the current backend
    if (status) {
        // printf starting the backend!
        start();

        // Hook-up the views to the backend
        connect(m_backend_requests, &IBackendRequests::program_counter_changed, m_code_views,
                &CodeViews::program_counter_changed);

        //m_locals_view->set_backend_interface(m_backend_requests);
        //m_callstack_view->set_backend_interface(m_backend_requests);
        //m_file_browser->set_backend_interface(m_backend_requests);

    } else {
        close_current_backend();
        qDebug() << "MainWindow::target_reply " << status << " " << error_message;
    }
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

    /*
    if (m_register_view) {
        m_register_view->set_backend_interface(nullptr);
    }
    */

    // m_registerView->set_backend_interface(nullptr);
    // m_code_views->set_backend_interface(nullptr);
    // m_memoryView->set_backend_interface(nullptr);

    m_backend = nullptr;
    m_backend_thread = nullptr;
    m_backend_requests = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setup_backend(BackendSession* backend) {
    m_backend = backend;

    m_backend_thread = new QThread;

    m_backend->moveToThread(m_backend_thread);
    m_backend_requests = new BackendRequests(m_backend);

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
    m_code_views->toggle_breakpoint();
    // m_source_view->toggle_breakpoint_current_line();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::new_memory_view() {
    /*
        MemoryView* mv = new MemoryView(this);
        mv->set_backend_interface(m_backendRequests);
        QDockWidget* dock = new QDockWidget(QStringLiteral("MemoryView"), this);
        dock->setAllowedAreas(Qt::AllDockWidgetAreas);
        dock->setObjectName(QStringLiteral("MemoryViewDock"));
        dock->setWidget(mv);
        addDockWidget(Qt::TopDockWidgetArea, dock);
        dock->setFloating(true);
        m_viewHandler->addView(mv);
    */
}

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

void MainWindow::create_view_instance(int index) {
    QWidget* view_widget = m_view_handler->create_view_by_index(index);
    auto plugin_types = m_view_handler->plugin_types();
    const QString& plugin_name = plugin_types[index].plugin_name;

    auto dock = new QDockWidget(plugin_name, this);
    dock->setWidget(view_widget);
    dock->setAllowedAreas(Qt::AllDockWidgetAreas);

    // make sure we have an unique objectname
    int count = m_view_names[plugin_name];

    if (count > 0) {
        QString result;
        QTextStream(&result) << plugin_name + QStringLiteral(" ") + count;
        dock->setObjectName(result);
    } else {
        dock->setObjectName(plugin_name);
    }

    // TODO: Better placement
    addDockWidget(Qt::BottomDockWidgetArea, dock);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::create_views_menu() {
    QMenu* plugin_menu = menuBar()->addMenu(QStringLiteral("Plugins"));
    int plugin_index = 0;
    auto plugin_types = m_view_handler->plugin_types();

    qDebug() << "menu " << plugin_menu;

    for (auto& plugin : plugin_types) {
        QAction* plugin_menu_entry = new QAction(plugin.plugin_name);
        plugin_menu_entry->setData(plugin_index);
        plugin_menu->addAction(plugin_menu_entry);

        printf("adding entry\n");

        connect(plugin_menu_entry, &QAction::triggered, this,
                [this, plugin_index]() { create_view_instance(plugin_index); });
        plugin_index++;
    }

    menuBar()->update();
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
