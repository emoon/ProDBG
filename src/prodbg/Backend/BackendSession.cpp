#include "BackendSession.h"
#include "Core/PluginHandler.h"
#include "IBackendRequests.h"
#include "Service.h"
#include "api/src/remote/pd_readwrite_private.h"
#include <QDebug>
#include <QString>
#include <QTimer>
#include <pd_backend.h>
#include <pd_io.h>
#include <pd_readwrite.h>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackendSession::BackendSession()
    : m_writer0(new PDWriter)
    , m_writer1(new PDWriter)
    , m_currentWriter(nullptr)
    , m_prevWriter(nullptr)
    , m_reader(new PDReader)
    , m_backendPlugin(nullptr)
    , m_backendPluginData(nullptr)
{
    pd_binary_writer_init(m_writer0);
    pd_binary_writer_init(m_writer1);
    pd_binary_reader_init(m_reader);

    m_currentWriter = m_writer0;
    m_prevWriter = m_writer1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackendSession::~BackendSession()
{
    destroyPluginData();

    pd_binary_writer_destroy(m_writer0);
    pd_binary_writer_destroy(m_writer1);
    pd_binary_reader_destroy(m_reader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BackendSession* BackendSession::createBackendSession(const QString& backendName)
{
    BackendSession* session = new BackendSession();

    if (!session->setBackend(backendName)) {
        delete session;
        return nullptr;
    }

    return session;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BackendSession::setBackend(const QString& backendName)
{
    // Names of the backends are stored in utf-8 so convert them here
    QByteArray name = backendName.toUtf8();
    PDBackendPlugin* plugin = PluginHandler_findBackendPlugin(name.constData());

    if (!plugin) {
        qDebug() << "Unable to find plugin: " << backendName;
        return false;
    }

    destroyPluginData();

    m_backendPlugin = plugin;

    // Asserts here to verify that these are always set. TODO: Better user facing error?

    Q_ASSERT(m_backendPlugin->create_instance);

    m_backendPluginData = m_backendPlugin->create_instance(&Service_get);

    Q_ASSERT(m_backendPluginData);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::destroyPluginData()
{
    if (m_backendPlugin && m_backendPluginData) {
        m_backendPlugin->destroy_instance(m_backendPluginData);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static QString s_stateTable[] = {
    QStringLiteral("No target"),        QStringLiteral("Running"),          QStringLiteral("Stop (breakpoint)"),
    QStringLiteral("Stop (exception)"), QStringLiteral("Trace (stepping)"), QStringLiteral("Unknown"),
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const QString& getStateName(int state)
{
    if (state < PDDebugState_Count && state >= 0) {
        switch (state) {
            case PDDebugState_NoTarget:
                return s_stateTable[0];
            case PDDebugState_Running:
                return s_stateTable[1];
            case PDDebugState_StopBreakpoint:
                return s_stateTable[2];
            case PDDebugState_StopException:
                return s_stateTable[3];
            case PDDebugState_Trace:
                return s_stateTable[4];
        }
    }

    return s_stateTable[5];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateMemory(QVector<uint16_t>* target, PDReader* reader)
{
    uint8_t* data;
    uint64_t address = 0;
    uint64_t size = 0;

    target->resize(0);

    PDRead_find_u64(reader, &address, "address", 0);

    if (PDRead_find_data(reader, (void**)&data, &size, "data", 0) == PDReadStatus_NotFound) {
        return;
    }

    for (uint64_t i = 0; i < size; ++i) {
        uint16_t t = *data++;
        t |= IBackendRequests::MemoryAddressFlags::Readable;
        t |= IBackendRequests::MemoryAddressFlags::Writable;
        target->append(t);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t updateDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, PDReader* reader)
{
    uint32_t addressWidth = 0;

    PDReaderIterator it;

    PDRead_find_u32(reader, &addressWidth, "address_width", 0);

    if (PDRead_find_array(reader, &it, "disassembly", 0) == PDReadStatus_NotFound) {
        return addressWidth;
    }

    while (PDRead_get_next_entry(reader, &it)) {
        uint64_t address;
        const char* text;

        IBackendRequests::AssemblyInstruction inst;

        PDRead_find_u64(reader, &address, "address", it);
        PDRead_find_string(reader, &text, "line", it);

        inst.text = QString::fromUtf8(text);
        inst.address = address;

        instructions->append(inst);
    }

    return addressWidth;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateRegisters(QVector<IBackendRequests::Register>* target, PDReader* reader)
{
    PDReaderIterator it;

    if (PDRead_find_array(reader, &it, "registers", 0) == PDReadStatus_NotFound) {
        printf("Unable to find registers array\n");
        return;
    }

    while (PDRead_get_next_entry(reader, &it)) {
        const char* name = "";
        uint8_t* data = 0;
        uint64_t size = 0;
        uint8_t read_only = 0;

        IBackendRequests::Register reg;

        PDRead_find_string(reader, &name, "name", it);
        PDRead_find_u8(reader, &read_only, "read_only", it);
        PDRead_find_data(reader, (void**)&data, &size, "register", it);

        reg.name = QString::fromUtf8(name);
        reg.read_only = read_only ? true : false;

        for (int i = 0; i < size; ++i) {
            reg.data.append(data[i]);
        }

        target->append(reg);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::toggleAddressBreakpoint(uint64_t address, bool add)
{
    PDWrite_event_begin(m_currentWriter, PDEventType_SetBreakpoint);
    PDWrite_u64(m_currentWriter, "address", address);
    PDWrite_event_end(m_currentWriter);

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::toggleFileLineBreakpoint(const QString& filename, int line, bool add)
{
    PDWrite_event_begin(m_currentWriter, PDEventType_SetBreakpoint);
    PDWrite_string(m_currentWriter, "filename", filename.toUtf8().data());
    PDWrite_u32(m_currentWriter, "line", line);
    PDWrite_event_end(m_currentWriter);

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::beginReadRegisters(QVector<IBackendRequests::Register>* target)
{
    uint32_t event = 0;

    PDWrite_event_begin(m_currentWriter, PDEventType_GetRegisters);
    PDWrite_event_end(m_currentWriter);

    update();

    target->resize(0);

    while ((event = PDRead_get_event(m_reader))) {
        if (event != PDEventType_SetRegisters) {
            continue;
        }

        updateRegisters(target, m_reader);
    }

    endReadRegisters(target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target)
{
    uint32_t event;

    qDebug() << "Got memory request";

    uint64_t size = hi - lo;

    // Write request and update

    PDWrite_event_begin(m_currentWriter, PDEventType_GetMemory);
    PDWrite_u64(m_currentWriter, "address_start", lo);
    PDWrite_u64(m_currentWriter, "size", size);
    PDWrite_event_end(m_currentWriter);

    update();

    while ((event = PDRead_get_event(m_reader))) {
        switch (event) {
            case PDEventType_SetMemory: {
                updateMemory(target, m_reader);
                break;
            }
        }
    }

    endReadMemory(target, lo, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::beginDisassembly(uint64_t address, uint32_t count,
                                      QVector<IBackendRequests::AssemblyInstruction>* target)
{
    uint32_t event = 0;
    uint32_t addressWidth = 0;

    target->resize(0);

    PDWrite_event_begin(m_currentWriter, PDEventType_GetDisassembly);
    PDWrite_u64(m_currentWriter, "address_start", address);
    PDWrite_u64(m_currentWriter, "instruction_count", count);
    PDWrite_event_end(m_currentWriter);

    update();

    while ((event = PDRead_get_event(m_reader))) {
        switch (event) {
            case PDEventType_SetDisassembly: {
                addressWidth = updateDisassembly(target, m_reader);
                break;
            }
        }
    }

    endDisassembly(target, addressWidth);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::sendCustomString(uint16_t id, const QString& text)
{
    PDWrite_event_begin(m_currentWriter, id);
    PDWrite_string(m_currentWriter, "text", text.toUtf8().data());
    PDWrite_event_end(m_currentWriter);

    update();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::updateCurrentPc()
{
    uint32_t event = 0;

    pd_binary_reader_reset(m_reader);

    while ((event = PDRead_get_event(m_reader))) {
        if (event != PDEventType_SetExceptionLocation) {
            continue;
        }

        IBackendRequests::ProgramCounterChange pcChange;

        uint64_t pc = 0;
        bool fileLineChaged = false;
        const char* filename = nullptr;

        if (PDRead_find_string(m_reader, &filename, "filename", 0) != PDReadStatus_NotFound) {
            uint32_t line = 0;
            PDRead_find_u32(m_reader, &line, "line", 0);

            QString file = QString::fromUtf8(filename);

            pcChange.filename = file;
            pcChange.line = (int)line;;

            if (line != m_currentLine || file != m_currentFile) {
                m_currentFile = file;
                m_currentLine = line;
                fileLineChaged = true;
            }
        } else {
            pcChange.line = -1;
        }

        PDRead_find_u64(m_reader, &pc, "address", 0);

        pcChange.programCounter = pc;

        if (pc != m_currentPc || fileLineChaged) {
            m_currentPc = pc;
            programCounterChanged(pcChange);
        }
    }

    pd_binary_reader_reset(m_reader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDDebugState BackendSession::internalUpdate(PDAction action)
{
    if (!m_backendPlugin) {
        return PDDebugState_NoTarget;
    }

    pd_binary_writer_finalize(m_currentWriter);

    unsigned int reqDataSize = pd_binary_writer_get_size(m_currentWriter);
    pd_binary_reader_reset(m_reader);

    pd_binary_reader_init_stream(m_reader, pd_binary_writer_get_data(m_currentWriter), reqDataSize);
    pd_binary_writer_reset(m_prevWriter);

    PDDebugState state = m_backendPlugin->update(m_backendPluginData, action, m_reader, m_prevWriter);

    pd_binary_writer_finalize(m_prevWriter);

    pd_binary_reader_init_stream(m_reader, pd_binary_writer_get_data(m_prevWriter),
                                 pd_binary_writer_get_size(m_prevWriter));
    pd_binary_reader_reset(m_reader);
    pd_binary_writer_reset(m_currentWriter);

    // Send state change if state is different from the last time

    if (state != m_debugState) {
        statusUpdate(getStateName(state));
        m_debugState = state;
        updateCurrentPc();
    }

    return state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::update()
{
    internalUpdate(PDAction_None);
    updateCurrentPc();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::start()
{
    if (m_debugState == PDDebugState_Running) {
        return;
    }

    internalUpdate(PDAction_Run);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &BackendSession::update);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::stop()
{
    internalUpdate(PDAction_Break);
    updateCurrentPc();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::breakDebug()
{
    internalUpdate(PDAction_Break);
    updateCurrentPc();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::stepIn()
{
    // printf("stepIn\n");
    internalUpdate(PDAction_Step);
    updateCurrentPc();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::stepOver()
{
    internalUpdate(PDAction_StepOver);
    updateCurrentPc();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
