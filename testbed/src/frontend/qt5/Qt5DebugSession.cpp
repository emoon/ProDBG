#include "Qt5DebugSession.h"
#include "Qt5DebuggerThread.h"
#include "Qt5CodeEditor.h"
#include "Qt5CallStack.h"
#include "Qt5Locals.h"
#include "core/BinarySerializer.h"
#include <QThread>
#ifndef _WIN32
#include <unistd.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

Qt5DebugSession* g_debugSession = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DebugSession::Qt5DebugSession()
{
    m_debuggerThread = 0;
    m_threadRunner = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::createSession()
{
    printf("Qt5DebugSession::createSession\n");
    g_debugSession = new Qt5DebugSession;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::addCodeEditor(Qt5CodeEditor* codeEditor)
{
    m_codeEditors.push_back(codeEditor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::addLocals(Qt5Locals* locals)
{
    m_locals.push_back(locals);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::addCallStack(Qt5CallStack* callStack)
{
    m_callStacks.push_back(callStack);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::delCodeEditor(Qt5CodeEditor* codeEditor)
{
    m_codeEditors.removeOne(codeEditor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::delLocals(Qt5Locals* locals)
{
    m_locals.removeOne(locals);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::delCallStack(Qt5CallStack* callStack)
{
    m_callStacks.removeOne(callStack);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::begin(const char* executable)
{
	PDSerializeWrite writer_;
	PDSerializeWrite* writer = &writer_;

	m_threadRunner = new QThread;
	m_debuggerThread = new Qt5DebuggerThread;
	m_debuggerThread->moveToThread(m_threadRunner);

	// TODO: Not sure if we should set this up here, would be better to move it so we don't need to actually
	//       start a executable/etc when doing a begin

	connect(m_threadRunner , SIGNAL(started()), m_debuggerThread, SLOT(start()));
	connect(m_debuggerThread, SIGNAL(finished()), m_threadRunner , SLOT(quit()));
	connect(this, &Qt5DebugSession::tryStartDebugging, m_debuggerThread, &Qt5DebuggerThread::tryStartDebugging); 
	connect(this, &Qt5DebugSession::tryStep, m_debuggerThread, &Qt5DebuggerThread::tryStep); 

	connect(m_debuggerThread, &Qt5DebuggerThread::sendData, this, &Qt5DebugSession::getData); 
	connect(this, &Qt5DebugSession::sendData, m_debuggerThread, &Qt5DebuggerThread::getData); 

	printf("beginDebug %s %d\n", executable, (uint32_t)(uint64_t)QThread::currentThreadId());

	BinarySerializer_initWriter(writer);

	// Write executable

	BinarySerialize_beginEvent(writer, PDEventType_setExecutable, 0);
	PDWRITE_STRING(writer, executable);
	BinarySerialize_endEvent(writer);

	// TODO: Write breakpoints here

	// Write start

	BinarySerialize_beginEvent(writer, PDEventType_start, 0);
	BinarySerialize_endEvent(writer);

	// TODO: Write executable to debugger plugin

	emit sendData(writer->writeData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5DebugSession::getFilenameLine(const char** filename, int* line)
{
    (void)filename;
    (void)line;
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Rewrite, sort by name and such

bool Qt5DebugSession::hasLineBreakpoint(const char* filename, int line)
{
	for (int i = 0, e = m_breakpointCount; i < e; ++i)
    {
    	const BreakpointFileLine* breakpoint = &m_breakpoints[i];
    	
    	if (!strstr(breakpoint->filename, filename))
    		continue;
    		
    	if (breakpoint->line == line)
    		return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::step()
{
    emit tryStep();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::getData(void* readerData)
{
	PDSerializeRead reader;
	PDSerializeRead* readerPtr = &reader;

	BinarySerializer_initReader(readerPtr, readerData);

	while (PDREAD_BYTES_LEFT(readerPtr) > 0)
	{
		int size = PDREAD_INT(readerPtr);
		PDEventType type = (PDEventType)PDREAD_INT(readerPtr);
		int eventId = PDREAD_INT(readerPtr);
		(void)eventId;

		switch (type)
		{
			case PDEventType_getLocals:
			{
				for (auto i = m_locals.begin(); i != m_locals.end(); i++) 
					(*i)->update(readerPtr);
				break;
			}

			case PDEventType_getCallStack:
			{
				for (auto i = m_callStacks.begin(); i != m_callStacks.end(); i++) 
					(*i)->update(readerPtr);
				break;
			}

			//case PDEventType_watch:
			//case PDEventType_registers:
			//case PDEventType_memory:
			//case PDEventType_tty:
			default:
			{
				// If we don't know the type we just skip all data

				printf("Unknown readerType %d\n", type);
				PDREAD_SKIP_BYTES(readerPtr, size);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5DebugSession::addBreakpointUI(const char* file, int line)
{
    // if we don't have a thread the session debugging session hasn't started yet so it's fine to add the bp directly

    if (!m_debuggerThread)
    {
        printf("Qt5DebugSession::addBreakpointUI %s %d\n", file, line);
        addBreakpoint(file, line, -2);
        return true;
    }
    else
    {
        //emit tryAddBreakpoint(file, line);
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5DebugSession::addBreakpoint(const char* file, int line, int id)
{
    if (m_breakpointCount + 1 >= m_breakpointMaxCount)
        return false;

    BreakpointFileLine* bp = &m_breakpoints[m_breakpointCount++];
        
    *bp = { file, line, id };

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::delBreakpoint(int id)
{
	BreakpointFileLine* breakpoints = m_breakpoints;
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
