#pragma once

#include <stdint.h>
#include <QtCore/QObject>
#include <QtCore/QVector>

class QString;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class IBackendRequests : public QObject {
    Q_OBJECT;

public:
    // Used for basic requests that doesn't need input args
    enum class BasicRequest {
        // All source files in the debug target as known by the backend
        SourceFiles,
        // Callstack of the current debugging thread (if applicable)
        Callstack,
    };

    //
    // Description of a hardware register commonly found in most CPUs
    //
    struct Register {
        // Name of the register
        QString name;
        // Data for the register (stored in big endian order)
        QVector<uint8_t> data;
        // If the register is read only (hw registers such as Program Counter)
        bool read_only;
    };

    //
    // Callstack Entry
    //
    struct CallstackEntry {
        // Address (may be ~0 if not applicable)
        uint64_t address;
        // Description line of the callstack. Up to backend but module/file/line/etc may be included
        QString desc;
        // Language type
        QString lang;
        // Source file if supported
        QString file;
        // Line matching with source file
        int line;
    };

    struct Callstack {
        QVector<CallstackEntry> entries;
        uint64_t request_id;
    };

    //
    // Data for a variable
    //
    struct VariableData {
        // Adress of the data (may be null if not native code)
        uint64_t address;
        QString name;
        QString value;
        QString type;
        // This is is true if the type may have children
        // such as structs, arrays, etc
        bool may_have_children;
    };

    //
    //
    struct Variables {
        QVector<VariableData> variables;
        uint64_t request_id;
    };

    //
    // Send from the backend when the position of the program counter has
    // changed. filename and line is optionally set if this information is
    // avalibile in the backend and line is set to -1 if no such info is there
    // to be used
    //
    struct ProgramCounterChange {
        QString filename;
        uint64_t pc;
        int line;
    };

    //
    // Description of assembly instruction that is provided when requesting
    // disassemble of code fro the target
    //
    struct AssemblyInstruction {
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
    enum MemoryAddressFlags {
        // Set if memory is read-able
        Readable = 1 << 8,
        // Set if memory is write-able
        Writable = 1 << 9
    };

public:
    // Send a custom event to the backend. The id should be registers using the
    // IdService_register This can be done in the same way using the id service
    // on the backend side. This allows the front-end to send custom commands to
    // the backend that doesn't fit any general backend
    // virtual void sendCustomString(uint16_t id, const QString& text) = 0;

    // A debug target can contains a list of source files used for the modules.
    // This will request that the backend sends back a list of them
    virtual void request_basic(BasicRequest request_type) = 0;

    // Add a breakpoint on a specific file and line number
    virtual void request_add_file_line_breakpoint(const QString& filename, int line) = 0;

    // Remove a breakpoint at a specific address
    virtual void request_remove_file_line_breakpoint(const QString& filename, int line) = 0;

    // Request locals variables
    virtual uint64_t request_locals(const QString& locals_entry) = 0;

    // Request frame index
    virtual uint64_t request_frame_index(int frame_index) = 0;

    // TODO: Reply breakpoints back

    // Add a breakpoint at a specific address
    // virtual void beginAddAddressBreakpoint(uint64_t address) = 0;

    // Add a breakpoint on a specific file and line number
    // virtual void beginAddFileLineBreakpoint(const QString& filename, int line) = 0;

    // Remove a breakpoint at a specific address
    // virtual void beginRemoveAddressBreakpoint(uint64_t address) = 0;

    // Remove a breakpoint on a specific file and line number
    // virtual void beginRemoveFileLineBreakpoint(const QString& filename, int line) = 0;

    // Get hw registers from the backend
    // registers = array of registers
    // virtual void beginReadRegisters(QVector<Register>* registers) = 0;

    // Get disassembly from the backend
    // lo = starting memory range
    // hi = ending memory range
    // instruction = a series of instructions from the backend
    // virtual void beginDisassembly(uint64_t address,
    //                              uint32_t instructionCount,
    //                              QVector<AssemblyInstruction>* instructions) = 0;

    // Evaluate expressions such ass 0x120+12 (useful for memory view)
    // expression = expression to evaluate (for example 0x124 + 2)
    // out = result of the operation
    // virtual void beginResolveAddress(const QString& expression, uint64_t* out) = 0;

    // Read a block of memory from the target.
    // lo = starting memory range
    // hi = ending memory range
    // target = output of memory. Each byte is stored as uint16_t with the upper
    // 8 bits are set as combination
    //          of MemoryAddressFlags
    // virtual bool beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target) = 0;

public:
    // reply from request of source files
    Q_SIGNAL void reply_callstack(const Callstack& callstack);

    // reply from request of source files
    Q_SIGNAL void reply_source_files(const QVector<QString>& files);

    // Get hw registers from the backend
    // registers = array of registers
    // Q_SIGNAL void endReadRegisters(QVector<Register>* registers);

    // The result from the beginDisassembly operation
    // Q_SIGNAL void endDisassembly(QVector<AssemblyInstruction>* instructions, int addressWidth);

    // Response signal for expression evaluation
    // errorMessage = "" if no issue, otherwise error message
    // dest = output of the evalutation
    // Q_SIGNAL void endResolveAddress(uint64_t* dest);

    // Response signal for a memory request. If target size is 0 the operation
    // failed. TODO: Better way target = filled with requested memory (if
    // successful) address = starting address addressWidth = number of bytes an
    // address uses. E.g. 4 for a 32-bit target.
    // Q_SIGNAL void endReadMemory(QVector<uint16_t>* target, uint64_t address, int addressWidth);

    // This signal is being sent when the program counter of the debugged
    // application has changed This can be used to figure out if it's needed to
    // re-request data. For example a Memory view may want to use this as the
    // program may have altered the same memory that is currently being
    // displayed
    Q_SIGNAL void program_counter_changed(const ProgramCounterChange& pc);

    // Reply of locals
    Q_SIGNAL void reply_locals(const Variables& variables);

    // This signal is being sent when the the current debugging session has ended
    Q_SIGNAL void session_ended();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
