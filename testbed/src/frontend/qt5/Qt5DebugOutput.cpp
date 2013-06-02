#include "Qt5DebugOutput.h"
#include "Qt5DebugSession.h"
#include "ProDBGAPI.h"

namespace prodbg
{

Qt5DebugOutput::Qt5DebugOutput(QWidget* parent)
: QPlainTextEdit(parent)
{
	setLineWrapMode(QPlainTextEdit::NoWrap);

	g_debugSession->addDebugOutput(this);
}

Qt5DebugOutput::~Qt5DebugOutput()
{
	g_debugSession->delDebugOutput(this);
}

void Qt5DebugOutput::updateDebugOutput(PDDebugOutput* debugOutput)
{
	if (debugOutput->output[0] == 0x0)
		return;
	
	appendPlainText(debugOutput->output);
}

}