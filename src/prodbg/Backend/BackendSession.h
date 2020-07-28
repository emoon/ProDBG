#pragma once

#include <pd_backend.h>
#include <QtCore/QObject>
#include "IBackendRequests.h"

class QString;
class QTimer;
struct PDReader;
struct PDWriter;
struct PDBackendPlugin;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BackendSession : public QObject {
    Q_OBJECT

public:
    BackendSession();
    ~BackendSession();

    static BackendSession* create_backend_session(const QString& backendName);
    bool set_backend(const QString& backendName);

    Q_SLOT void thread_finished();
    Q_SLOT void update();
    Q_SLOT void file_target_request(const QString& path);
    Q_SLOT void start();

    Q_SLOT void request_add_file_line_breakpoint(const QString& filename, int line);
    Q_SLOT void request_remove_file_line_breakpoint(const QString& filename, int line);

    Q_SLOT void step_in();

    Q_SLOT void request_locals(const QString& locals_entry, uint64_t request_id);

    Q_SLOT void request_basic(IBackendRequests::BasicRequest request_id);
    Q_SLOT void request_frame_index(int frame_index);

    /*
    Q_SLOT void start();
    Q_SLOT void stop();
    Q_SLOT void stepOver();
    Q_SLOT void breakContDebug();

    Q_SLOT void evalExpression(const QString& expression, uint64_t* out);

    Q_SLOT void sendCustomString(uint16_t id, const QString& text);

    Q_SLOT void toggleAddressBreakpoint(uint64_t address, bool add);
    Q_SLOT void toggleFileLineBreakpoint(const QString& filename, int line, bool add);

    Q_SLOT void beginReadRegisters(QVector<IBackendRequests::Register>* target);
    Q_SLOT void beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);
    Q_SLOT void beginDisassembly(uint64_t address,
                                 uint32_t count,
                                 QVector<IBackendRequests::AssemblyInstruction>* target);

    // Signals
    Q_SIGNAL void endResolveAddress(uint64_t* out);
    Q_SIGNAL void endReadRegisters(QVector<IBackendRequests::Register>* registers);
    Q_SIGNAL void endDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, int adressWidth);
    Q_SIGNAL void endReadMemory(QVector<uint16_t>* res, uint64_t address, int addressWidth);
    */
    // Q_SIGNAL void statusUpdate(const QString& update);
    // Q_SIGNAL void sourceFileLineChanged(const QString& filename, uint32_t line);
    Q_SIGNAL void program_counter_changed(const IBackendRequests::ProgramCounterChange& pc);
    Q_SIGNAL void target_reply(bool status, const QString& error_message);

    Q_SIGNAL void reply_locals(const IBackendRequests::Variables& variables);
    Q_SIGNAL void reply_callstack(const IBackendRequests::Callstack& callstack);
    Q_SIGNAL void reply_source_files(const QVector<QString>& source_files);

    Q_SIGNAL void session_ended();

private:
    void update_current_pc();
    void destory_plugin_data();

    PDDebugState internal_update(PDAction action);

    PDDebugState m_debugState = PDDebugState_NoTarget;

    QString m_currentFile;
    uint32_t m_currentLine = 0;
    uint64_t m_currentPc = 0;

    // Writers/Read for communitaction between backend and UI
    PDWriter* m_writer0;
    PDWriter* m_writer1;
    PDWriter* m_currentWriter;
    PDWriter* m_prevWriter;
    PDReader* m_reader;
    QTimer* m_timer = nullptr;

    // Current active backend plugin
    PDBackendPlugin* m_backend_plugin;

    // Data owned by current active backend plugin
    void* m_backend_plugin_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace prodbg
