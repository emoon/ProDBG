#include "Qt5DebugSession.h"
#include "Qt5DebuggerThread.h"
#include <QThread>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

Qt5DebugSession* g_debugSession = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DebugSession::Qt5DebugSession()
{
    m_breakpoints = new PDBreakpointFileLine[256];
    m_breakpointCount = 0; 
    m_breakpointMaxCount = 256;
    m_debuggerThread = 0;
    m_threadRunner = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::createSession()
{
    g_debugSession = new Qt5DebugSession;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::begin(const char* executable)
{
	m_threadRunner = new QThread;
	m_debuggerThread = new Qt5DebuggerThread();
	m_debuggerThread->moveToThread(m_threadRunner);

	connect(m_threadRunner , SIGNAL(started()), m_debuggerThread, SLOT(start()));
	connect(m_debuggerThread, SIGNAL(finished()), m_threadRunner , SLOT(quit()));
	connect(this, &Qt5DebugSession::tryStartDebugging, m_debuggerThread, &Qt5DebuggerThread::tryStartDebugging); 

	m_threadRunner->start();

	printf("beginDebug %s %d\n", executable, (uint32_t)(uint64_t)QThread::currentThreadId());

	emit tryStartDebugging(executable, m_breakpoints, (int)m_breakpointCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5DebugSession::getFilenameLine(const char** filename, int* line)
{
    (void)filename;
    (void)line;
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::connectWidget(QObject* widget)
{
	connect(this, SIGNAL(callUIthread()), widget, SLOT(sessionUpdate()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::disconnectWidget(QObject* widget)
{
	disconnect(this, SIGNAL(callUIthread()), widget, SLOT(sessionUpdate()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5DebugSession::hasLineBreakpoint(int line)
{
    (void)line;
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::step()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::addBreakpointUI(const char* file, int line)
{
    file = "Fake6502Main.c"; // temp temp

    // if we don't have a thread the session debugging session hasn't started yet so it's fine to add the bp directly

    if (!m_debuggerThread)
    {
        printf("Qt5DebugSession::addBreakpointUI %s %d\n", file, line);
        addBreakpoint(file, line, -2);
    }
    else
    {
        emit tryAddBreakpoint(file, line);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5DebugSession::addBreakpoint(const char* file, int line, int id)
{
    if (m_breakpointCount + 1 >= m_breakpointMaxCount)
        return false;

    PDBreakpointFileLine* bp = &m_breakpoints[m_breakpointCount++];
        
    *bp = { file, line, id };

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::delBreakpoint(int id)
{
   PDBreakpointFileLine* breakpoints = m_breakpoints; 
   int count = m_breakpointCount;

    for (int i = 0, e = count; i < e; ++i)
    {
        if (breakpoints[i].id == id)
        {
            count--;

            if (count != 0)
                breakpoints[i] = breakpoints[count];

            m_breakpointCount = count;
            return;
        }
    }
}


}
