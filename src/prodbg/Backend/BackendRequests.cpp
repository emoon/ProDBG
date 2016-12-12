#include "BackendRequests.h"
#include "BackendSession.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackendRequests::BackendRequests(BackendSession* session)
{
    connect(this, &BackendRequests::sendCustomStr, session, &BackendSession::sendCustomString);

    connect(this, &BackendRequests::requestMem, session, &BackendSession::beginReadMemory);
    connect(this, &BackendRequests::requestDisassembly, session, &BackendSession::beginDisassembly);
    connect(this, &BackendRequests::readRegisters, session, &BackendSession::beginReadRegisters);

    connect(this, &BackendRequests::toggleAddressBreakpoint, session, &BackendSession::toggleAddressBreakpoint);
    connect(this, &BackendRequests::toggleFileLineBreakpoint, session, &BackendSession::toggleFileLineBreakpoint);

    connect(this, &BackendRequests::evalExpression, session, &BackendSession::evalExpression);

    connect(session, &BackendSession::endReadMemory, this, &BackendRequests::endReadMemory);
    connect(session, &BackendSession::endDisassembly, this, &BackendRequests::endDisassembly);
    connect(session, &BackendSession::endReadRegisters, this, &BackendRequests::endReadRegisters);
    connect(session, &BackendSession::endResolveAddress, this, &BackendRequests::endResolveAddress);

    connect(session, &BackendSession::programCounterChanged, this, &BackendRequests::programCounterChanged);
    connect(session, &BackendSession::sessionEnded, this, &BackendRequests::sessionEnded);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::sendCustomString(uint16_t id, const QString& text)
{
    sendCustomStr(id, text);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginAddAddressBreakpoint(uint64_t address)
{
    toggleAddressBreakpoint(address, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginAddFileLineBreakpoint(const QString& filename, int line)
{
    toggleFileLineBreakpoint(filename, line, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginRemoveAddressBreakpoint(uint64_t address)
{
    toggleAddressBreakpoint(address, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginRemoveFileLineBreakpoint(const QString& filename, int line)
{
    toggleFileLineBreakpoint(filename, line, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginReadRegisters(QVector<IBackendRequests::Register>* registers)
{
    readRegisters(registers);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginResolveAddress(const QString& expression, uint64_t* out)
{
    evalExpression(expression, out);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginDisassembly(uint64_t address, uint32_t count,
                                       QVector<IBackendRequests::AssemblyInstruction>* instructions)
{
    requestDisassembly(address, count, instructions);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BackendRequests::beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target)
{
    // There should be a better way to return this. Right now the reciver size has to guess
    // what goes wrong. I think it would be better to wrap all of this into some Result<> (Rust style)
    // type instead that describes why something is Err or Ok.

    if (!target) {
        return false;
    }

    if (lo >= hi) {
        target->resize(0);
        return false;
    }

    requestMem(lo, hi, target);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
