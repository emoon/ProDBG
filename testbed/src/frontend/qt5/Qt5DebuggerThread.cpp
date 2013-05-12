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
	
void Qt5DebuggerThread::run()
{
	int count;

	printf("update debugger thread %d\n", (uint32_t)(uint64_t)QThread::currentThreadId());

	Plugin* plugin = PluginHandler_getPlugins(&count);

	if (count != 1)
		return;

	// try to start debugging session of a plugin

	m_debuggerPlugin = (PDDebugPlugin*)plugin->data;
	m_pluginData = m_debuggerPlugin->createInstance(0);

	if (!m_debuggerPlugin->start(m_pluginData, PD_DEBUG_LAUNCH, (void*)m_executable))
		return;

	// TODO: Handle exit from thread

	while (1)
	{
		m_debuggerPlugin->action(m_pluginData, PD_DEBUG_ACTION_NONE, 0);
		msleep(10);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

