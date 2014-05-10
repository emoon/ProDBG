#include "Qt5DebugSession.h"
#include "Qt5DebuggerThread.h"
#include "Qt5CodeEditor.h"
#include "Qt5CallStack.h"
#include "Qt5Locals.h"
#include "Qt5Registers.h"
#include "Qt5DebugOutput.h"
#include "core/Log.h"
#include "core/AssemblyRegister.h"
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
    m_breakpointMaxCount = 0;
    m_assemblyRegisters = 0;
    m_assemblyRegistersCount = 0;

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

void Qt5DebugSession::addRegisters(Qt5Registers* registers)
{
    m_registers.push_back(registers);
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

void Qt5DebugSession::delRegisters(Qt5Registers* registers)
{
    m_registers.removeOne(registers);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::begin(const char* executable, bool run)
{
    m_threadRunner = new QThread;
    m_debuggerThread = new Qt5DebuggerThread(Qt5DebuggerThread::Local);
    m_debuggerThread->moveToThread(m_threadRunner);

    m_debuggerThread->m_timer = new QTimer(0); //parent must be null
    m_debuggerThread->m_timer->moveToThread(m_threadRunner);

    PDWriter writerData;
    PDWriter* writer = &writerData;

    // TODO: Not sure if we should set this up here, would be better to move it so we don't need to actually
    //       start a executable/etc when doing a begin

    connect(m_threadRunner, SIGNAL(started()), m_debuggerThread, SLOT(start()));
    connect(m_debuggerThread, SIGNAL(finished()), m_threadRunner, SLOT(quit()));
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
        PDWrite_u32(writer, "line", (uint32_t)m_breakpoints[i].line);
        PDWrite_eventEnd(writer);
    }

    if (run)
    {
        PDWrite_eventBegin(writer, PDEventType_action);
        PDWrite_u32(writer, "action", 3);
        PDWrite_eventEnd(writer);
    }

    PDBinaryWriter_finalize(writer);

    emit sendData((uint8_t*)PDBinaryWriter_getData(writer), (int)PDBinaryWriter_getSize(writer));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is to start a remote session

void Qt5DebugSession::beginRemote(const char* address, int port)
{
    m_threadRunner = new QThread;
    m_debuggerThread = new Qt5DebuggerThread(Qt5DebuggerThread::Remote);
    m_debuggerThread->moveToThread(m_threadRunner);
    m_debuggerThread->setRemoteTarget(address, port);

    m_debuggerThread->m_timer = new QTimer(0); //parent must be null
    m_debuggerThread->m_timer->moveToThread(m_threadRunner);

    PDWriter writerData;
    PDWriter* writer = &writerData;

    // TODO: Not sure if we should set this up here, would be better to move it so we don't need to actually
    //       start a executable/etc when doing a begin

    connect(m_threadRunner, SIGNAL(started()), m_debuggerThread, SLOT(start()));
    connect(m_debuggerThread, SIGNAL(finished()), m_threadRunner, SLOT(quit()));
    connect(this, &Qt5DebugSession::tryAction, m_debuggerThread, &Qt5DebuggerThread::doAction);
    connect(m_debuggerThread, &Qt5DebuggerThread::sendData, this, &Qt5DebugSession::setState);
    connect(this, &Qt5DebugSession::sendData, m_debuggerThread, &Qt5DebuggerThread::setState);

    printf("beginDebug %s:%d %d\n", address, port, (uint32_t)(uint64_t)QThread::currentThreadId());


    m_threadRunner->start();

    PDBinaryWriter_init(writer);

    for (int i = 0, count = m_breakpointCount; i != count; ++i)
    {
        PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
        PDWrite_string(writer, "filename", m_breakpoints[i].filename);
        PDWrite_u32(writer, "line", (uint32_t)m_breakpoints[i].line);
        PDWrite_eventEnd(writer);
    }

    PDBinaryWriter_finalize(writer);

    emit sendData((uint8_t*)PDBinaryWriter_getData(writer), (int)PDBinaryWriter_getSize(writer));
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
    PDWriter writerData;
    PDWriter* writer = &writerData;

    PDBinaryWriter_init(writer);

    PDWrite_eventBegin(writer, PDEventType_getDisassembly);
    PDWrite_u64(writer, "address_start", startAddress);
    PDWrite_u32(writer, "instruction_count", (uint32_t)instructionCount);
    PDWrite_eventEnd(writer);

    emit sendData(PDBinaryWriter_getData(writer), (int)PDBinaryWriter_getSize(writer));

    PDBinaryWriter_destroy(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::sendBreakpoint(const char* filename, int line)
{
    PDWriter writerData;
    PDWriter* writer = &writerData;

    PDBinaryWriter_init(writer);

    PDWrite_eventBegin(writer, PDEventType_setBreakpoint);
    PDWrite_string(writer, "filename", filename);
    PDWrite_u32(writer, "line", (uint32_t)line);
    PDWrite_eventEnd(writer);

    PDBinaryWriter_finalize(writer);

    emit sendData(PDBinaryWriter_getData(writer), (int)PDBinaryWriter_getSize(writer));

    PDBinaryWriter_destroy(writer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::setState(uint8_t* readerData, int serSize)
{
    int event;
    PDReader readerD;
    PDReader* reader = &readerD;

    PDBinaryReader_init(reader);
    PDBinaryReader_initStream(reader, readerData, (uint32_t)serSize);

    while ((event = (int)PDRead_getEvent(reader)))
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
                        m_codeEditors[0]->setFileLine(filename, (int)line);
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
                int64_t start = -1;
                int32_t instructionCount = -1;
                const char* stringBuffer = 0;

                PDRead_findString(reader, &stringBuffer, "string_buffer", 0);
                PDRead_findS32(reader, &instructionCount, "instruction_count", 0);
                PDRead_findS64(reader, &start, "start_address", 0);

                // Only update 1 for now
                if (m_codeEditors.size() > 0)
                {
                    // \todo Specific editor/decided mode if disassembly or not?
                    m_codeEditors[0]->setMode(Qt5CodeEditor::Disassembly);
                    m_codeEditors[0]->setDisassembly(stringBuffer, start, instructionCount);
                }

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

            case PDEventType_setRegisters:
            {
                m_assemblyRegisters = AssemblyRegister_buildFromReader(reader, m_assemblyRegisters, &m_assemblyRegistersCount);

                if (m_registers.size() > 0)
                    m_registers[0]->update(m_assemblyRegisters, m_assemblyRegistersCount);

                if (m_codeEditors.size() > 0)
                    m_codeEditors[0]->setAssemblyRegisters(m_assemblyRegisters, m_assemblyRegistersCount);

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
        addBreakpoint(file, line, -2);
        return true;
    }
    else
    {
        addBreakpoint(file, line, -2);
        sendBreakpoint(file, line);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Qt5DebugSession::addBreakpoint(const char* file, int line, int id)
{
    printf("%s:%d\n", __FILE__, __LINE__);

    if (m_breakpointCount + 1 >= m_breakpointMaxCount)
        return false;

    BreakpointFileLine* bp = &m_breakpoints[m_breakpointCount++];

    bp->filename = file;
    bp->line = line;
    bp->id = id;

    printf("Adding breakpoint at %s:%d\n", file, line);

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebugSession::readSourceFile(const char* file)
{
    if (m_codeEditors.size() > 0)
        m_codeEditors[0]->readSourceFile(file);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
