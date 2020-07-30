#include "BackendRequests.h"
#include "BackendSession.h"

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackendRequests::BackendRequests(BackendSession* session) {
    connect(this, &BackendRequests::request_file_target_signal, session, &BackendSession::file_target_request);
    connect(this, &BackendRequests::request_add_file_line_breakpoint_signal, session,
            &BackendSession::request_add_file_line_breakpoint);
    connect(this, &BackendRequests::request_remove_file_line_breakpoint_signal, session,
            &BackendSession::request_remove_file_line_breakpoint);

    connect(this, &BackendRequests::request_locals_signal, session, &BackendSession::request_locals);
    connect(this, &BackendRequests::request_basic_signal, session, &BackendSession::request_basic);
    connect(this, &BackendRequests::request_frame_index_signal, session, &BackendSession::request_frame_index);


    /*
    connect(this, &BackendRequests::sendCustomStr, session, &BackendSession::sendCustomString);

    connect(this, &BackendRequests::requestMem, session, &BackendSession::beginReadMemory);
    connect(this, &BackendRequests::requestDisassembly, session, &BackendSession::beginDisassembly);
    connect(this, &BackendRequests::readRegisters, session, &BackendSession::beginReadRegisters);

    connect(this, &BackendRequests::toggleAddressBreakpoint, session, &BackendSession::toggleAddressBreakpoint);
    connect(this, &BackendRequests::toggleFileLineBreakpoint, session, &BackendSession::toggleFileLineBreakpoint);

    connect(this, &BackendRequests::evalExpression, session, &BackendSession::evalExpression);
    */

    /*
    connect(session, &BackendSession::endReadMemory, this, &BackendRequests::endReadMemory);
    connect(session, &BackendSession::endDisassembly, this, &BackendRequests::endDisassembly);
    connect(session, &BackendSession::endReadRegisters, this, &BackendRequests::endReadRegisters);
    connect(session, &BackendSession::endResolveAddress, this, &BackendRequests::endResolveAddress);
    */

    connect(session, &BackendSession::reply_callstack, this, &BackendRequests::reply_callstack);
    connect(session, &BackendSession::reply_source_files, this, &BackendRequests::reply_source_files);
    connect(session, &BackendSession::reply_locals, this, &BackendRequests::reply_locals);
    connect(session, &BackendSession::program_counter_changed, this, &BackendRequests::program_counter_changed);
    connect(session, &BackendSession::session_ended, this, &BackendRequests::session_ended);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::file_target_request(const QString& path) {
    request_file_target_signal(path);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::request_basic(IBackendRequests::BasicRequest request_id) {
    request_basic_signal(request_id);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::request_add_file_line_breakpoint(const QString& filename, int line) {
    request_add_file_line_breakpoint_signal(filename, line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::request_remove_file_line_breakpoint(const QString& filename, int line) {
    request_remove_file_line_breakpoint_signal(filename, line);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint64_t BackendRequests::request_locals(const ExpandVars& expanded_vars) {
    uint64_t request_id = m_request_id++;
    printf("BackendRequests::request_locals\n");
    request_locals_signal(expanded_vars, request_id);
    return request_id;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint64_t BackendRequests::request_frame_index(int frame_index) {
    request_frame_index_signal(frame_index);
    return 0;
}


/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::sendCustomString(uint16_t id, const QString& text) {
    sendCustomStr(id, text);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginAddAddressBreakpoint(uint64_t address) {
    toggleAddressBreakpoint(address, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginAddFileLineBreakpoint(const QString& filename, int line) {
    toggleFileLineBreakpoint(filename, line, true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginRemoveAddressBreakpoint(uint64_t address) {
    toggleAddressBreakpoint(address, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginRemoveFileLineBreakpoint(const QString& filename, int line) {
    toggleFileLineBreakpoint(filename, line, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginReadRegisters(QVector<IBackendRequests::Register>* registers) {
    readRegisters(registers);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginResolveAddress(const QString& expression, uint64_t* out) {
    evalExpression(expression, out);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendRequests::beginDisassembly(uint64_t address,
                                       uint32_t count,
                                       QVector<IBackendRequests::AssemblyInstruction>* instructions) {
    requestDisassembly(address, count, instructions);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BackendRequests::beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target) {
    // There should be a better way to return this. Right now the reciver size
    // has to guess what goes wrong. I think it would be better to wrap all of
    // this into some Result<> (Rust style) type instead that describes why
    // something is Err or Ok.

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
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
