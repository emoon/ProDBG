#pragma once

#include <QMainWindow>
#include "ui_mainwindow.h"

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

private:

    CodeView* m_codeView;
    MemoryViewWidget* m_memoryView;
    QStatusBar* m_statusbar;
    Ui_MainWindow m_ui;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
