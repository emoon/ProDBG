#pragma once

#include "ui_mainwindow.h"
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
    Q_SLOT void start();
    Q_SLOT void stop();
    Q_SLOT void stepIn();
    Q_SLOT void stepOver();
    Q_SLOT void toggleBreakpoint();
    Q_SLOT void amigaUAEConfig();
    Q_SLOT void debugAmigaExe();

private:
    void initActions();
    void writeSettings();
    void readSettings();
    void startDummyBackend();

    // Hardcoded views for now.
    CodeView* m_codeView;
    MemoryView* m_memoryView;
    RegisterView* m_registerView;
    QStatusBar* m_statusbar;
    Ui_MainWindow m_ui;

    BackendRequests* m_backendRequests;
    BackendSession* m_backend;
    QThread* m_backendThread;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
