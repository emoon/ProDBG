#pragma once

#include <QtWidgets/QMainWindow>
#include "api/include/pd_ui.h"
#include "ui_MainWindow.h"

class QStatusBar;
class QThread;
class QPluginLoader;

namespace prodbg {

class Session;
class CodeView;
class MemoryView;
class AmigaUAE;
class BackendSession;
// class RegisterView;
class BackendRequests;
class BreakpointModel;
class CodeViews;
class RecentExecutables;
class ViewHandler;
class SourceCodeWidget;

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
    Q_SLOT void reload_current_file();
    Q_SLOT void break_cont_debug();
    Q_SLOT void start_debug();
    Q_SLOT void start();
    Q_SLOT void stop();
    Q_SLOT void step_in();
    Q_SLOT void step_over();
    Q_SLOT void toggle_breakpoint();
    Q_SLOT void open_recent_exe();

    Q_SLOT void new_memory_view();
    Q_SLOT void new_register_view();

    Q_SIGNAL void break_cont_backend();
    Q_SIGNAL void start_backend();
    Q_SIGNAL void stop_backend();
    Q_SIGNAL void step_in_backend();
    Q_SIGNAL void step_over_backend();

    Q_SLOT void show_prefs();

private:
    // Current supported backends (hard-coded for now)
    enum Backend {
        Dummy,
        Amiga,
        Custom,
    };

    void init_actions();
    void write_settings();
    void read_settings();
    void start_dummy_backend();
    void close_current_backend();
    void start_amiga_uae_backend();
    void setup_backend_connections();
    void setup_backend(BackendSession* backend);

    void init_recent_file_actions();
    void stop_internal();

    Q_SLOT void uae_started();
    Q_SLOT void status_update(const QString& status);
    Q_SLOT void process_ended(int exitCode);

    QVector<QAction*> m_recent_file_actions;

    SourceCodeWidget* m_source_view = nullptr;
    ViewHandler* m_view_handler = nullptr;

    // This is somewhat temporary but convinient to have
    // QString m_lastAmigaExe;

    // Hard-coded Amiga support. Would be nice to have this more modular
    // AmigaUAE* m_amigaUae = nullptr;

    // List of recent executables
    RecentExecutables* m_recent_executables = nullptr;
    // test
    QPluginLoader* m_reg_plugin_loader;
    PDUIInterface* m_register_view = nullptr;

    // Hardcoded views for now.
    MemoryView* m_memory_view = nullptr;
    QStatusBar* m_statusbar = nullptr;
    BreakpointModel* m_breakpoints = nullptr;

    Ui_MainWindow m_ui;
    Backend m_current_backend = Dummy;

    CodeViews* m_code_views = nullptr;

    BackendRequests* m_backend_requests = nullptr;
    BackendSession* m_backend = nullptr;
    QThread* m_backend_thread = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
