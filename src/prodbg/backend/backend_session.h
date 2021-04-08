#pragma once

#include <pd_backend.h>
#include <QtCore/QObject>
#include "backend_requests_interface.h"
#include "../core/messages_api.h"

class QString;
class QTimer;
struct PDReader;
struct PDWriter;
struct PDBackendPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BackendSession : public QObject {
    Q_OBJECT

public:
    BackendSession();
    ~BackendSession();

    static BackendSession* create_backend_session(const QString& backend_name);
    bool set_backend(const QString& backend_name);

    Q_SLOT void thread_finished();
    Q_SLOT void update();
    Q_SLOT void file_target_request(bool stop_at_main, const QString& path);
    Q_SLOT void start();

    Q_SLOT void request_add_file_line_breakpoint(const QString& filename, int line);
    Q_SLOT void request_remove_file_line_breakpoint(const QString& filename, int line);

    Q_SLOT void step_in();

    Q_SLOT void request_locals(const PDIBackendRequests::ExpandVars& expanded_vars, uint64_t request_id);

    Q_SLOT void request_basic(PDIBackendRequests::BasicRequest request_id);
    Q_SLOT void request_frame_index(int frame_index);

    Q_SLOT void request_memory(uint64_t lo, uint64_t hi, QVector<uint8_t>* target);
    Q_SLOT void request_custom(int custom_message_id, QVector<uint8_t>* target, QVector<uint8_t>* source);

    // Reply signals

    Q_SIGNAL void program_counter_changed(const PDIBackendRequests::ProgramCounterChange& pc);
    Q_SIGNAL void target_reply(bool status, const QString& error_message);

    Q_SIGNAL void reply_locals(const PDIBackendRequests::Variables& variables);
    Q_SIGNAL void reply_callstack(const PDIBackendRequests::Callstack& callstack);
    Q_SIGNAL void reply_source_files(const QVector<QString>& source_files);
    Q_SIGNAL void reply_memory(QVector<uint8_t>* res, uint64_t address, int address_width);
    Q_SIGNAL void reply_custom(int custom_message_id, QVector<uint8_t>* target);

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
    MessagesAPI* m_messages_api = nullptr;
    QTimer* m_timer = nullptr;

    // Current active backend plugin
    PDBackendPlugin* m_backend_plugin;

    // Data owned by current active backend plugin
    void* m_backend_plugin_data;
};

