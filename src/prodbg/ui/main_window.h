#pragma once

#include <QtWidgets/QMainWindow>
//#include "api/include/pd_ui.h"
#include "ui_main_window.h"
#include "project.h"

class QStatusBar;
class QThread;
class QPluginLoader;
class FastDock;

class Session;
class CodeView;
class MemoryView;
class BackendSession;
// class RegisterView;
class BackendRequests;
class BreakpointModel;
class CodeViews;
class RecentProjects;
class ViewHandler;
class LocalsView;
class CallstackView;
class FileBrowserView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);

private:
    Q_SLOT void open_source_file();
    Q_SLOT void break_cont_debug();
    Q_SLOT void start();
    Q_SLOT void stop();
    Q_SLOT void step_in();
    Q_SLOT void step_over();
    Q_SLOT void toggle_breakpoint();
    Q_SLOT void open_recent_project();
    Q_SLOT void open_debug_exe();
    Q_SLOT void open_binary_file();
    Q_SLOT void open_debug_exe_stop_at_main();

    Q_SIGNAL void break_cont_backend();
    Q_SIGNAL void start_backend();
    Q_SIGNAL void stop_backend();
    Q_SIGNAL void step_in_backend();
    Q_SIGNAL void step_over_backend();

    Q_SLOT void target_reply(bool status, const QString& error_message);

    Q_SLOT void show_prefs();

private:
    // Current supported backends (hard-coded for now)
    enum Backend {
        Dummy,
        Custom,
    };

    void init_plugins(const QString& path);
    void create_views_menu();

    void init_actions();
    void write_settings();
    void read_settings();
    void start_dummy_backend();
    void close_current_backend();
    void setup_backend_connections();
    void setup_backend(BackendSession* backend);

    void init_recent_file_actions();
    void stop_internal();
    void open_debug(bool stop_at_main);
    void create_view_instance(int index);

    Q_SLOT void status_update(const QString& status);
    Q_SLOT void process_ended(int exitCode);

    QVector<QAction*> m_recent_project_actions;

    QStatusBar* m_statusbar = nullptr;
    FastDock* m_docking = nullptr;

    //FileBrowserView* m_file_browser = nullptr;
    //LocalsView* m_locals_view = nullptr;
    //CallstackView* m_callstack_view = nullptr;
    ViewHandler* m_view_handler = nullptr;

    // List of recent projects
    RecentProjects* m_recent_projects = nullptr;

    // Hardcoded views for now.
    //MemoryView* m_memory_view = nullptr;
    BreakpointModel* m_breakpoints = nullptr;

    Ui_MainWindow m_ui;
    Backend m_current_backend = Dummy;

    CodeViews* m_code_views = nullptr;
    BackendRequests* m_backend_requests = nullptr;
    BackendSession* m_backend = nullptr;
    QThread* m_backend_thread = nullptr;

    QString m_last_file_dir;

    QHash<QString, int> m_view_names;

    // The current active project
    Project* m_current_project;
};

