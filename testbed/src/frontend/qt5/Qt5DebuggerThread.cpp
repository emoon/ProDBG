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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
void Qt5DebuggerThread::start()
{
	int count;

	printf("start Qt5DebuggerThread\n");

	Plugin* plugin = PluginHandler_getPlugins(&count);

	if (count != 1)
	{
		emit finished();
		return;
	}

	// try to start debugging session of a plugin

	m_debuggerPlugin = (PDBackendPlugin*)plugin->data;
	m_pluginData = m_debuggerPlugin->createInstance(0);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));

	printf("end start Qt5DebuggerThread\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::tryStartDebugging()
{
	printf("Try starting debugging\n");

	// Start the debugging and if we manage start the update that will be called each 10 ms

	if (m_debuggerPlugin->action(m_pluginData, PDAction_run))
	{
		m_timer.start(10);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::update()
{
	if (PDDebugState_paused == m_debuggerPlugin->update(m_pluginData))
	{
		/*
		// TODO: fix ugly line hack here (temproray not to resend the data all the time if not needed)
	
		if (m_oldLine != m_debugDataState.line)
		{
			emit sendDebugDataState(&m_debugDataState);
			m_oldLine = m_debugDataState.line;
		}
		*/
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::tryStep()
{
	m_debuggerPlugin->action(m_pluginData, PDAction_step);
}

}

