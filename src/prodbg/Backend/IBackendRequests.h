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
    //
    // Description of a hardware register commonly found in most CPUs
    //
    struct Register
    {
        // Name of the register
        QString name;
        // Data for the register (stored in big endian order)
        QVector<uint8_t> data;
        // If the register is read only (hw registers such as Program Counter)
        bool read_only;
    };

    //
    // Send from the backend when the position of the program counter has changed.
    // filename and line is optionally set if this information is avalibile in the backend
    // and line is set to -1 if no such info is there to be used
    //
    struct ProgramCounterChange
    {
        QString filename;
        uint64_t programCounter;
        int line;
    };

    //
    // Description of assembly instruction that is provided when requesting
    // disassemble of code fro the target
    //
    struct AssemblyInstruction
    {
        // Text of the instruction (like move.w d0,d1 or mov rax,rbx)
        QString text;
        // Address on where the instruction was stored
        uint64_t address;
        // List of hw registers that this instruction reads
        // This may be empty if the backend doesn't support to fill this info in
        QVector<QString> read_registers;
        // List of hw registers that this instrution reads
        // This may be empty if the backend doesn't support to fill this info in
        QVector<QString> write_registers;
    };

    //
    // Describes the memory as sent back from beginReadMemory. These flags
    // indicates what kind of memory it is (read/write/unmapped/etc)
    //
    enum MemoryAddressFlags
    {
        // Set if memory is read-able
        Readable = 1 << 8,
        // Set if memory is write-able
        Writable = 1 << 9
    };

public:
    // Send a custom event to the backend. The id should be registers using the IdService_register
    // This can be done in the same way using the id service on the backend side. This allows the front-end to send custom commands
    // to the backend that doesn't fit any general backend
    virtual void sendCustomString(uint16_t id, const QString& text) = 0;

    // Add a breakpoint at a specific address
    virtual void beginAddAddressBreakpoint(uint64_t address) = 0;

    // Add a breakpoint on a specific file and line number
    virtual void beginAddFileLineBreakpoint(const QString& filename, int line) = 0;

    // Remove a breakpoint at a specific address
    virtual void beginRemoveAddressBreakpoint(uint64_t address) = 0;

    // Remove a breakpoint on a specific file and line number
    virtual void beginRemoveFileLineBreakpoint(const QString& filename, int line) = 0;

    // Get hw registers from the backend
    // registers = array of registers
    virtual void beginReadRegisters(QVector<Register>* registers) = 0;

    // Get disassembly from the backend
    // lo = starting memory range
    // hi = ending memory range
    // instruction = a series of instructions from the backend
    virtual void beginDisassembly(uint64_t address, uint32_t instructionCount,
                                  QVector<AssemblyInstruction>* instructions) = 0;

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
    // Get hw registers from the backend
    // registers = array of registers
    Q_SIGNAL void endReadRegisters(QVector<Register>* registers);

    // The result from the beginDisassembly operation
    Q_SIGNAL void endDisassembly(QVector<AssemblyInstruction>* instructions, int addressWidth);

    // Response signal for expression evaluation
    // errorMessage = "" if no issue, otherwise error message
    // dest = output of the evalutation
    Q_SIGNAL void endResolveAddress(uint64_t* dest);

    // Response signal for a memory request. If target size is 0 the operation failed. TODO: Better way
    // target = filled with requested memory (if successful)
    // address = starting address
    // addressWidth = number of bytes an address uses. E.g. 4 for a 32-bit target.
    Q_SIGNAL void endReadMemory(QVector<uint16_t>* target, uint64_t address, int addressWidth);

    // This signal is being sent when the program counter of the debugged application has changed
    // This can be used to figure out if it's needed to re-request data. For example a Memory view may want to use
    // this as the program may have altered the same memory that is currently being displayed
    Q_SIGNAL void programCounterChanged(const ProgramCounterChange& pc);

    // This signal is being sent when the the current debugging session has ended 
    Q_SIGNAL void sessionEnded();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
