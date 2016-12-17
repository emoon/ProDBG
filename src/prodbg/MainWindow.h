#pragma once

#include "ui_MainWindow.h"
#include <QMainWindow>

class QStatusBar;
class QThread;

namespace prodbg {

class Session;
class CodeView;
class MemoryView;
class AmigaUAE;
class BackendSession;
class RegisterView;
class BackendRequests;
class BreakpointModel;
class CodeViews;
class RecentExecutables;
class ViewHandler;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);

private:
    Q_SLOT void openSourceFile();
    Q_SLOT void reloadCurrentFile();
    Q_SLOT void breakContDebug();
    Q_SLOT void startDebug();
    Q_SLOT void start();
    Q_SLOT void stop();
    Q_SLOT void stepIn();
    Q_SLOT void stepOver();
    Q_SLOT void toggleBreakpoint();
    Q_SLOT void amigaUAEConfig();
    Q_SLOT void debugAmigaExe();
    Q_SLOT void openRecentExe();

    Q_SLOT void newMemoryView();
    Q_SLOT void newRegisterView();

    Q_SIGNAL void breakContBackend();
    Q_SIGNAL void startBackend();
    Q_SIGNAL void stopBackend();
    Q_SIGNAL void stepInBackend();
    Q_SIGNAL void stepOverBackend();

private:
    // Current supported backends (hard-coded for now)
    enum Backend
    {
        Dummy,
        Amiga,
        Custom,
    };

    void internalStartAmigaExe();
    void initActions();
    void writeSettings();
    void readSettings();
    void startDummyBackend();
    void closeCurrentBackend();
    void startAmigaUAEBackend();
    void setupBackendConnections();
    void setupBackend(BackendSession* backend);

    void initRecentFileActions();
    void stopInternal();

    Q_SLOT void uaeStarted();
    Q_SLOT void statusUpdate(const QString& status);
    Q_SLOT void processEnded(int exitCode);

    QVector<QAction*> m_recentFileActions;

    ViewHandler* m_viewHandler = nullptr;

    // This is somewhat temporary but convinient to have
    QString m_lastAmigaExe;

    // Hard-coded Amiga support. Would be nice to have this more modular
    AmigaUAE* m_amigaUae = nullptr;

    // List of recent executables
    RecentExecutables* m_recentExecutables = nullptr;

    // Hardcoded views for now.
    MemoryView* m_memoryView = nullptr;
    RegisterView* m_registerView = nullptr;
    QStatusBar* m_statusbar = nullptr;
    BreakpointModel* m_breakpoints = nullptr;

    Ui_MainWindow m_ui;
    Backend m_currentBackend = Dummy;

    CodeViews* m_codeViews = nullptr;

    BackendRequests* m_backendRequests = nullptr;
    BackendSession* m_backend = nullptr;
    QThread* m_backendThread = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
