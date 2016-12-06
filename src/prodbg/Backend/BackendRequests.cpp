#include "BackendRequests.h"
#include "BackendSession.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackendRequests::BackendRequests(BackendSession* session)
{
    connect(this, &BackendRequests::requestMem, session, &BackendSession::beginReadMemory);
    connect(this, &BackendRequests::requestDisassembly, session, &BackendSession::beginDisassembly);
    connect(this, &BackendRequests::readRegisters, session, &BackendSession::beginReadRegisters);

    connect(session, &BackendSession::endReadMemory, this, &BackendRequests::endReadMemory);
    connect(session, &BackendSession::endDisassembly, this, &BackendRequests::endDisassembly);
    connect(session, &BackendSession::endReadRegisters, this, &BackendRequests::endReadRegisters);

    connect(session, &BackendSession::programCounterChanged, this, &BackendRequests::programCounterChanged);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginReadRegisters(QVector<IBackendRequests::Register>* registers)
{
    readRegisters(registers);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginResolveAddress(const QString& expression, uint64_t* out)
{
    (void)expression;
    (void)out;
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
