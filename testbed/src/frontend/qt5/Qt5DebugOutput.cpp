#include "Qt5DebugOutput.h"

namespace prodbg
{

Qt5DebugOutput::Qt5DebugOutput(QWidget* parent)
: QPlainTextEdit(parent)
{
	setLineWrapMode(QPlainTextEdit::NoWrap);
	setPlainText(tr("This is the debug output!"));
}

Qt5DebugOutput::~Qt5DebugOutput()
{

}

void Qt5DebugOutput::appendLine(const char* line)
{
	printf("%s\n", line);
}

}