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
    // Evaluate expressions such ass 0x120+12 (useful for memory view)
    void beginResolveAddress(const QString& expression, uint64_t* out);

    // Requests a block of memory from the target. The return vector has
    // is uint16_t with the real data in lower 8-bit and upper is reserved for status flags
    // Readable/Writeable/etc
    bool beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);

private:
    Q_SIGNAL void requestMem(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
