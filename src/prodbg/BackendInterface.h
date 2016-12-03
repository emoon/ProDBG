#pragma once

#include <QObject>
#include <QVector>
#include <stdint.h>

class QString;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BackendInterface : public QObject
{
    Q_OBJECT;

public:
    enum MemoryAddressFlags
    {
        Readable = 1 << 8,
        Writable = 1 << 9
    };

public:
    // Return the number of bytes an address uses. E.g. 4 for a 32-bit target.
    virtual int addressWidthBytes() = 0;

    // Evaluate expressions such ass 0x120+12 (useful for memory view)
    virtual bool resolveAddress(const QString& expression, uint64_t* out) = 0;

    // Requests a block of memory from the target. The return vector has
    // is uint16_t with the real data in lower 8-bit and upper is reserved for status flags
    // Readable/Writeable/etc
    virtual void requestMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target) = 0;

public:
    // Response signal for a memory request
    Q_SIGNAL void responseMemory(QVector<uint16_t>* target);

    // This signal is being sent when the program counter of the debugged application has changed
    // This can be used to figure out if it's needed to re-request data. For example a Memory view may want to use
    // this as the program may have altered the same memory that is currently being displayed
    Q_SIGNAL void programCounterChanged(uint64_t pc);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
