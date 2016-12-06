#include "BackendRequests.h"
#include "BackendSession.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackendRequests::BackendRequests(BackendSession* session)
{
    connect(this, &BackendRequests::requestMem, session, &BackendSession::beginReadMemory);
    connect(this, &BackendRequests::requestDisassembly, session, &BackendSession::beginDisassembly);

    connect(session, &BackendSession::endReadMemory, this, &BackendRequests::endReadMemory);
    connect(session, &BackendSession::endDisassembly, this, &BackendRequests::endDisassembly);

    connect(session, &BackendSession::programCounterChanged, this, &BackendRequests::programCounterChanged);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate expressions such ass 0x120+12 (useful for memory view)

void BackendRequests::beginResolveAddress(const QString& expression, uint64_t* out)
{
    (void)expression;
    (void)out;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginDisassembly(uint64_t address, uint32_t count, QVector<IBackendRequests::AssemblyInstruction>* instructions)
{
    requestDisassembly(address, count, instructions);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Requests a block of memory from the target. The return vector has
// is uint16_t with the real data in lower 8-bit and upper is reserved for status flags
// Readable/Writeable/etc

bool BackendRequests::beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target)
{
    requestMem(lo, hi, target);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
