#include "BackendRequests.h"
#include "BackendSession.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackendRequests::BackendRequests(BackendSession* session)
{
    connect(this, &BackendRequests::requestMem, session, &BackendSession::requestMemory);
    connect(session, &BackendSession::responseMemory, this, &BackendRequests::responseMemory);
    connect(session, &BackendSession::programCounterChanged, this, &BackendRequests::programCounterChanged);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Return the number of bytes an address uses. E.g. 4 for a 32-bit target.

int BackendRequests::addressWidthBytes()
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Evaluate expressions such ass 0x120+12 (useful for memory view)

bool BackendRequests::resolveAddress(const QString& expression, uint64_t* out)
{
    (void)expression;
    (void)out;
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Requests a block of memory from the target. The return vector has
// is uint16_t with the real data in lower 8-bit and upper is reserved for status flags
// Readable/Writeable/etc

void BackendRequests::requestMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target)
{
    requestMem(lo, hi, target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
