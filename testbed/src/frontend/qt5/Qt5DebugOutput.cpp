#include "Qt5DebugOutput.h"
#include "Qt5DebugSession.h"

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DebugOutput::Qt5DebugOutput(QWidget* parent) : QPlainTextEdit(parent)
{
	setLineWrapMode(QPlainTextEdit::NoWrap);
	setPlainText(tr("This is the debug output!"));
	g_debugSession->addTty(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DebugOutput::~Qt5DebugOutput()
{
	g_debugSession->delTty(this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugOutput::appendText(const char* line)
{
	if (line[0] == 0)
		return;

	//printf("Called with line  %s\n", line);
	//printf("Called with chars %x %x %x %x\n", line[0], line[1], line[2], line[3]);

	QString t = QString::fromLocal8Bit(line);
	t.replace(QString("\n"), QString(""));
	t.replace(QString("\r"), QString(""));

	appendPlainText(t);
}

}
