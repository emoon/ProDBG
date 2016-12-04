#pragma once

#include "IBackendRequests.h"
#include <QObject>

namespace prodbg {

class BackendSession;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BackendRequests : public IBackendRequests
{
    Q_OBJECT
public:
    BackendRequests(BackendSession* session);

public:
    // Return the number of bytes an address uses. E.g. 4 for a 32-bit target.
    int addressWidthBytes();

    // Evaluate expressions such ass 0x120+12 (useful for memory view)
    bool resolveAddress(const QString& expression, uint64_t* out);

    // Requests a block of memory from the target. The return vector has
    // is uint16_t with the real data in lower 8-bit and upper is reserved for status flags
    // Readable/Writeable/etc
    void requestMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);

private:
    Q_SIGNAL void requestMem(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
