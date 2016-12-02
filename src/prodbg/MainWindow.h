#pragma once

#include "ui_mainwindow.h"
#include <QMainWindow>

class QStatusBar;

namespace prodbg {

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

private:
    void initActions();
    void writeSettings();
    void readSettings();

    CodeView* m_codeView;
    MemoryView* m_memoryView;
    QStatusBar* m_statusbar;
    Ui_MainWindow m_ui;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
