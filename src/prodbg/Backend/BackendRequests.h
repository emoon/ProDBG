#pragma once

#include <QtCore/QObject>
#include "IBackendRequests.h"

namespace prodbg {

class BackendSession;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BackendRequests : public IBackendRequests {
    Q_OBJECT
public:
    BackendRequests(BackendSession* session);

public:
    // Request to start a file for debugging
    void file_target_request(bool stop_at_main, const QString& path);

    // Request source files from debug target
    void request_basic(IBackendRequests::BasicRequest request_type);

    // Add a breakpoint at a specific address
    //void beginAddAddressBreakpoint(uint64_t address);

    // Add a breakpoint on a specific file and line number
    void request_add_file_line_breakpoint(const QString& filename, int line);

    // Remove a breakpoint at a specific address
    void request_remove_file_line_breakpoint(const QString& filename, int line);

    // Request locals variables. This function returns an ID that should be used
    // to match the reply. This is because several users may request data and
    // it's up to the user to make sure the use the data that match the request
    uint64_t request_locals(const ExpandVars& expanded_vars);

   // Request frame selection
    uint64_t request_frame_index(int frame_index);

    // Send a custom event to the backend. The id should be registers using the
    // IdService_register This can be done in the same way using the id service
    // on the backend side. This allows the front-end to send custom commands to
    // the backend that doesn't fit any general backend
    /*
    void sendCustomString(uint16_t id, const QString& text);


    // Add a breakpoint on a specific file and line number
    void beginAddFileLineBreakpoint(const QString& filename, int line);

    // Remove a breakpoint at a specific address
    void beginRemoveAddressBreakpoint(uint64_t address);

    // Remove a breakpoint on a specific file and line number
    void beginRemoveFileLineBreakpoint(const QString& filename, int line);

    // Get hw registers from the backend
    // registers = array of registers
    void beginReadRegisters(QVector<Register>* registers);

    // Get disassembly from the backend
    // address = starting memory range
    // instructionCount = number of instructions to recive
    // instruction = a series of instructions from the backend
    void beginDisassembly(uint64_t address, uint32_t instructionCount,
                          QVector<IBackendRequests::AssemblyInstruction>* instructions);

    // Evaluate expressions such as 0x120+12 (useful for memory view)
    void beginResolveAddress(const QString& expression, uint64_t* out);

    // Requests a block of memory from the target. The return vector has
    // is uint16_t with the real data in lower 8-bit and upper is reserved for
    // status flags Readable/Writeable/etc
    bool beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);
    */

private:
    Q_SIGNAL void request_basic_signal(IBackendRequests::BasicRequest request_type);
    Q_SIGNAL void request_file_target_signal(bool stop_at_main, const QString& filename);
    Q_SIGNAL void request_add_file_line_breakpoint_signal(const QString& filename, int line);
    Q_SIGNAL void request_remove_file_line_breakpoint_signal(const QString& filename, int line);
    Q_SIGNAL void request_locals_signal(const IBackendRequests::ExpandVars& expanded_vars, uint64_t request_id);
    Q_SIGNAL void request_frame_index_signal(int frame_index);

    /*
    Q_SIGNAL void evalExpression(const QString& expr, uint64_t* out);
    Q_SIGNAL void sendCustomStr(uint16_t id, const QString& text);

    Q_SIGNAL void toggleFileLineBreakpoint(const QString& filename, int line, bool add);
    Q_SIGNAL void toggleAddressBreakpoint(uint64_t address, bool add);

    Q_SIGNAL void readRegisters(QVector<Register>* registers);
    Q_SIGNAL void requestMem(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);
    Q_SIGNAL void requestDisassembly(uint64_t address, uint32_t count,
                                     QVector<IBackendRequests::AssemblyInstruction>* instructions);
    */

    uint64_t m_request_id;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
