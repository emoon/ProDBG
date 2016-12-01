#pragma once

#include <QMainWindow>

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
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
