#include "Qt5ChildWindow.h"
#include <QtWidgets>

namespace prodbg
{

Qt5ChildWindow::Qt5ChildWindow()
{
	setAttribute(Qt::WA_DeleteOnClose);

	// TEMP:
	resize(QSize(400, 400));
	readSourceFile("examples/Fake6502/Fake6502Main.c");
}

void Qt5ChildWindow::closeEvent(QCloseEvent* event)
{
    //if (maybeSave())
  //  {
        event->accept();
    //}
    //else
   // {
        //event->ignore();
    //}
}

}
