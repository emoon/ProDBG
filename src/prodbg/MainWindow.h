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
    CodeView* m_codeView = nullptr;
    MemoryView* m_memoryView = nullptr;
    RegisterView* m_registerView = nullptr;
    QStatusBar* m_statusbar = nullptr;
    Ui_MainWindow m_ui;

    BackendRequests* m_backendRequests = nullptr;
    BackendSession* m_backend = nullptr;
    QThread* m_backendThread = nullptr;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
