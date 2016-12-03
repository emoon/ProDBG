#pragma once

#include "ui_mainwindow.h"
#include <QMainWindow>

class QStatusBar;
class QTimer;

namespace prodbg {

class Session;
class CodeView;
class MemoryView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

protected:
    void closeEvent(QCloseEvent* event);

private:
    Q_SLOT void start();
    Q_SLOT void stop();
    Q_SLOT void stepIn();
    Q_SLOT void stepOver();
    Q_SLOT void toggleBreakpoint();
    Q_SLOT void amigaUAEConfig();
    Q_SLOT void timedUpdate();

private:
    void initActions();
    void writeSettings();
    void readSettings();

    // Hardcoded views for now.
    CodeView* m_codeView;
    MemoryView* m_memoryView;
    QStatusBar* m_statusbar;
    Ui_MainWindow m_ui;

    QTimer* m_timer;

    // This is the active debugging session that will communicate with the backend and supply data to the frontend
    Session* m_currentSession;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
