#include "Qt5DebuggerThread.h"
#include <QThread>
#include <core/PluginHandler.h>
#include <ProDBGAPI.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DebuggerThread::Qt5DebuggerThread(const char* executable) : 
	m_executable(executable)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
void Qt5DebuggerThread::start()
{
	int count;

	printf("update debugger thread %d\n", (uint32_t)(uint64_t)QThread::currentThreadId());

	Plugin* plugin = PluginHandler_getPlugins(&count);

	if (count != 1)
	{
		emit finished();
		return;
	}

	// try to start debugging session of a plugin

	m_debuggerPlugin = (PDDebugPlugin*)plugin->data;
	m_pluginData = m_debuggerPlugin->createInstance(0);

	if (!m_debuggerPlugin->start(m_pluginData, PD_DEBUG_LAUNCH, (void*)m_executable))
	{
		emit finished();
		return;
	}

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
	m_timer.start(10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::update()
{
	m_debuggerPlugin->action(m_pluginData, PD_DEBUG_ACTION_NONE, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::getDebugStatus()
{
	void* returnData = nullptr;
	m_debuggerPlugin->getState(m_pluginData, &returnData); 
}


}

