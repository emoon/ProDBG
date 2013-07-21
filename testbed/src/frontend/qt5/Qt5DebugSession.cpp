#include "Qt5DebugSession.h"
#include "Qt5DebuggerThread.h"
#include "Qt5CodeEditor.h"
#include "Qt5CallStack.h"
#include "Qt5Locals.h"
#include "Qt5DebugOutput.h"
#include "core/Log.h"
#include <QThread>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "../../../API/RemoteAPI/PDReadWrite_private.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

Qt5DebugSession* g_debugSession = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DebugSession::Qt5DebugSession()
{
    m_debuggerThread = 0;
    m_threadRunner = 0;
    m_breakpointCount = 0;
    m_breakpointMaxCount = 0;;

    // TODO: Dynamic Array
	m_breakpoints = new BreakpointFileLine[256];

    m_breakpointCount = 0;
    m_breakpointMaxCount = 256;
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

void Qt5DebugSession::addTty(Qt5DebugOutput* debugOutput)
{
    m_debugOutputs.push_back(debugOutput);
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

void Qt5DebugSession::delTty(Qt5DebugOutput* debugOutput)
{
    m_debugOutputs.removeOne(debugOutput);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::begin(const char* executable, bool run)
{
	m_threadRunner = new QThread;
	m_debuggerThread = new Qt5DebuggerThread(Qt5DebuggerThread::Local);
	m_debuggerThread->moveToThread(m_threadRunner);

	PDWriter writerData;
	PDWriter* writer = &writerData;

	// TODO: Not sure if we should set this up here, would be better to move it so we don't need to actually
	//       start a executable/etc when doing a begin

	connect(m_threadRunner , SIGNAL(started()), m_debuggerThread, SLOT(start()));
	connect(m_debuggerThread, SIGNAL(finished()), m_threadRunner , SLOT(quit()));
	connect(this, &Qt5DebugSession::tryAction, m_debuggerThread, &Qt5DebuggerThread::doAction); 
	connect(m_debuggerThread, &Qt5DebuggerThread::sendData, this, &Qt5DebugSession::setState);
	connect(this, &Qt5DebugSession::sendData, m_debuggerThread, &Qt5DebuggerThread::setState); 

	printf("beginDebug %s %d\n", executable, (uint32_t)(uint64_t)QThread::currentThreadId());

	m_threadRunner->start();

	PDBinaryWriter_init(writer);

	// Write executable
	
	printf("Writing down filename %s to executable\n", executable);

	PDWrite_eventBegin(writer, PDEventType_setExecutable);
	PDWrite_string(writer, "filename", executable);
	PDWrite_eventEnd(writer);

	for (int i = 0, count = m_breakpointCount; i != count; ++i)
	{
		PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
		PDWrite_string(writer, "filename", m_breakpoints[i].filename);
		PDWrite_u32(writer, "line", m_breakpoints[i].line);
		PDWrite_eventEnd(writer);
	}

	if (run)
	{
		PDWrite_eventBegin(writer, PDEventType_action);
		PDWrite_u32(writer, "action", 3);
		PDWrite_eventEnd(writer);
	}

	emit sendData(PDBinaryWriter_getData(writer), PDBinaryWriter_getSize(writer));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is to start a remote session

void Qt5DebugSession::beginRemote(const char* address, int port)
{
	m_threadRunner = new QThread;
	m_debuggerThread = new Qt5DebuggerThread(Qt5DebuggerThread::Remote);
	m_debuggerThread->moveToThread(m_threadRunner);
	m_debuggerThread->setRemoteTarget(address, port);

	// TODO: Not sure if we should set this up here, would be better to move it so we don't need to actually
	//       start a executable/etc when doing a begin

	connect(m_threadRunner , SIGNAL(started()), m_debuggerThread, SLOT(start()));
	connect(m_debuggerThread, SIGNAL(finished()), m_threadRunner , SLOT(quit()));
	connect(this, &Qt5DebugSession::tryAction, m_debuggerThread, &Qt5DebuggerThread::doAction); 
	connect(m_debuggerThread, &Qt5DebuggerThread::sendData, this, &Qt5DebugSession::setState); 
	connect(this, &Qt5DebugSession::sendData, m_debuggerThread, &Qt5DebuggerThread::setState); 

	printf("beginDebug %s:%d %d\n", address, port, (uint32_t)(uint64_t)QThread::currentThreadId());

	m_threadRunner->start();
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

void Qt5DebugSession::callAction(int action)
{
    emit tryAction(action);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::requestDisassembly(uint64_t startAddress, int instructionCount)
{
	(void)startAddress;
	(void)instructionCount;
	/*
	PDSerializeWrite writer_;
	PDSerializeWrite* writer = &writer_;

	BinarySerializer_initWriter(writer);

	// \todo Should we always write down a 64-bit address here or not?

	BinarySerialize_beginEvent(writer, PDEventType_getDisassembly, 0);
	PDWRITE_INT(writer, (int)startAddress);
	PDWRITE_INT(writer, instructionCount); 
	BinarySerialize_endEvent(writer);

	emit sendData(BinarySerializer_getStartData(writer), BinarySerializer_writeSize(writer));
	*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::setState(void* readerData, int serSize)
{
	int event;
	PDReader readerD;
	PDReader* reader = &readerD;

	PDBinaryReader_init(reader);
	PDBinaryReader_initStream(reader, readerData, serSize);

	while ((event = PDRead_getEvent(reader)))
	{
		switch (event)
		{
			case PDEventType_setLocals:
			{
				// Only update 1 for now
				if (m_locals.size() > 0)
					m_locals[0]->update(reader);

				break;
			}

			case PDEventType_setExceptionLocation:
			{
				const char* filename = 0;
				uint64_t address = 0;
				uint32_t line = 0;

				PDRead_findString(reader, &filename, "filename", 0);
				PDRead_findU32(reader, &line, "line", 0);
				PDRead_findU64(reader, &address, "address", 0);

				if (filename)
				{
					if (m_codeEditors.size() > 0)
						m_codeEditors[0]->setFileLine(filename, line);
				}
				else
				{
					m_codeEditors[0]->setMode(Qt5CodeEditor::Disassembly);
					m_codeEditors[0]->setAddress(address);
				}

				break;
			}

			case PDEventType_setDisassembly:
			{
				/*
				const char* disassembly = PDREAD_STRING(readerPtr);

				// Only update 1 for now
				if (m_codeEditors.size() > 0)
				{
					// \todo Specific editor/decided mode if disassembly or not?
					m_codeEditors[0]->setMode(Qt5CodeEditor::Disassembly);
					m_codeEditors[0]->setDisassembly(disassembly);
				}
				*/

				break;
			}

			case PDEventType_setTty:
			{
				const char* tty;

				PDRead_findString(reader, &tty, "tty", 0);

				if (tty && m_debugOutputs.size() > 0)
					m_debugOutputs[0]->appendText(tty);

				break;
			}
		}
	}

	free(readerData);
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
