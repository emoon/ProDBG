#pragma once

#include "ui_mainwindow.h"
#include <QMainWindow>

class QStatusBar;

namespace prodbg {

class CodeView;
class MemoryViewWidget;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void start();
    void stop();
    void stepIn();
    void stepOver();
    void toggleBreakpoint();
    void amigaUAEConfig();

private:
    void initActions();
    void writeSettings();
    void readSettings();

    CodeView* m_codeView;
    MemoryViewWidget* m_memoryView;
    QStatusBar* m_statusbar;
    Ui_MainWindow m_ui;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
