#include "Qt5DebuggerThread.h"
#include "Qt5DebugSession.h"
#include <QThread>
#include <core/PluginHandler.h>
#include <ProDBGAPI.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DebuggerThread::Qt5DebuggerThread()
{
	m_fileLine.filename = new char[4096];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
void Qt5DebuggerThread::start()
{
	int count;

	Plugin* plugin = PluginHandler_getPlugins(&count);

	if (count != 1)
	{
		emit finished();
		return;
	}

	// try to start debugging session of a plugin

	m_debuggerPlugin = (PDDebugPlugin*)plugin->data;
	m_pluginData = m_debuggerPlugin->createInstance(0);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::tryAddBreakpoint(const char* filename, int line)
{
	PDBreakpointFileLine fileLineBP = { filename, line, 0 };

	printf("Qt5DebuggerThread::tryAddBreakpoint\n");

	int id = m_debuggerPlugin->addBreakpoint(m_pluginData, PDBreakpointType_FileLine, &fileLineBP);

	printf("id %d\n", id);

	g_debugSession->addBreakpoint(filename, line, id);

	// update the UI thread after we added a breakpoint

	emit callUIthread();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::tryStartDebugging(const char* filename, PDBreakpointFileLine* breakpoints, int bpCount)
{
	m_executable = filename;

	// Start the debugging and if we manage start the update that will be called each 10 ms

	if (m_debuggerPlugin->start(m_pluginData, PD_DEBUG_LAUNCH, (void*)m_executable, breakpoints, bpCount))
	{
		m_timer.start(10);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::update()
{
	m_debuggerPlugin->action(m_pluginData, PD_DEBUG_ACTION_NONE, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::tryStep()
{
	m_debuggerPlugin->action(m_pluginData, PD_DEBUG_ACTION_STEP, &m_fileLine);

	printf("Want to step to %s %d\n", m_fileLine.filename, m_fileLine.line); 

	emit setFileLine(m_fileLine.filename, m_fileLine.line); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDDebugState Qt5DebuggerThread::getDebugState(void** data)
{
	return m_debuggerPlugin->getState(m_pluginData, data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::getCallStack(PDCallstack* callStack, int* maxEntries)
{
	return m_debuggerPlugin->getCallStack(m_pluginData, callStack, maxEntries);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::getLocals(PDLocals* locals, int* maxEntries)
{
	return m_debuggerPlugin->getLocals(m_pluginData, locals, maxEntries);
}


}

