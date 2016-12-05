#pragma once

#include <QObject>
#include <QVector>
#include <stdint.h>

class QString;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IBackendRequests : public QObject
{
    Q_OBJECT;

public:
    enum MemoryAddressFlags
    {
        Readable = 1 << 8,
        Writable = 1 << 9
    };

public:
    // Evaluate expressions such ass 0x120+12 (useful for memory view)
    // expression = expression to evaluate (for example 0x124 + 2)
    // out = result of the operation
    virtual void beginResolveAddress(const QString& expression, uint64_t* out) = 0;

    // Read a block of memory from the target.
    // lo = starting memory range
    // hi = ending memory range
    // target = output of memory. Each byte is stored as uint16_t with the upper 8 bits are set as combination
    //          of MemoryAddressFlags
    virtual bool beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target) = 0;

public:
    // Response signal for expression evaluation 
    // success = true if experssion was evaluated correctly, otherwise false
    // dest = output of the evalutation
    Q_SIGNAL void endResolveadderss(bool success, uint64_t* dest);

    // Response signal for a memory request. If target size is 0 the operation failed. TODO: Better way
    // target = filled with requested memory (if successful)
    // address = starting address
    // addressWidth = number of bytes an address uses. E.g. 4 for a 32-bit target.
    Q_SIGNAL void endReadMemory(QVector<uint16_t>* target, uint64_t address, int addressWidth);

    // This signal is being sent when the program counter of the debugged application has changed
    // This can be used to figure out if it's needed to re-request data. For example a Memory view may want to use
    // this as the program may have altered the same memory that is currently being displayed
    Q_SIGNAL void programCounterChanged(uint64_t pc);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
