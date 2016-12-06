#include "BackendSession.h"
#include "Core/PluginHandler.h"
#include "IBackendRequests.h"
#include "api/src/remote/pd_readwrite_private.h"
#include <QDebug>
#include <QString>
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
    , m_currentPc(0)
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

    m_backendPluginData = m_backendPlugin->create_instance(0);

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

/*
static QString s_stateTable[] =
{
    QStringLiteral("No target"),
    QStringLiteral("Running"),
    QStringLiteral("Stop (breakpoint)"),
    QStringLiteral("Stop (exception)"),
    QStringLiteral("Trace (stepping)"),
    QStringLiteral("Unknown"),
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
*/

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

static void updateDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, PDReader* reader)
{
    PDReaderIterator it;

    if (PDRead_find_array(reader, &it, "disassembly", 0) == PDReadStatus_NotFound) {
        return;
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target)
{
    uint32_t event;

    qDebug() << "Got memory request";

    // There should be a better way to return this. Right now the reciver size has to guess
    // what goes wrong. I think it would be better to wrap all of this into some Result<> (Rust style)
    // type instead that describes why something is Err or Ok.

    if (!target) {
        endReadMemory(target, 0, 0);
        return;
    }

    if (lo >= hi) {
        target->resize(0);
        endReadMemory(target, 0, 0);
        return;
    }

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

void BackendSession::beginDisassembly(uint64_t address, uint32_t count, QVector<IBackendRequests::AssemblyInstruction>* target)
{
    uint32_t event = 0;

    qDebug() << "Got disassembly request";
    
    target->resize(0);

    PDWrite_event_begin(m_currentWriter, PDEventType_GetDisassembly);
    PDWrite_u64(m_currentWriter, "address_start", address);
    PDWrite_u64(m_currentWriter, "size", count);
    PDWrite_event_end(m_currentWriter);

    update();

    while ((event = PDRead_get_event(m_reader))) {
        switch (event) {
            case PDEventType_SetDisassembly: {
                updateDisassembly(target, m_reader);
                break;
            }
        }
    }

    endDisassembly(target);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::update()
{
    if (!m_backendPlugin) {
        return;
    }

    pd_binary_writer_finalize(m_currentWriter);

    // Swap the write buffers
    // PDWriter* temp = m_currentWriter;
    // m_currentWriter = m_prevWriter;
    // m_prevWriter = temp;

    unsigned int reqDataSize = pd_binary_writer_get_size(m_currentWriter);
    pd_binary_reader_reset(m_reader);

    pd_binary_reader_init_stream(m_reader, pd_binary_writer_get_data(m_currentWriter), reqDataSize);
    pd_binary_writer_reset(m_prevWriter);

    int state = m_backendPlugin->update(m_backendPluginData, PDAction_None, m_reader, m_prevWriter);
    (void)state;

    pd_binary_writer_finalize(m_prevWriter);

    pd_binary_reader_init_stream(m_reader, pd_binary_writer_get_data(m_prevWriter),
                                 pd_binary_writer_get_size(m_prevWriter));
    pd_binary_reader_reset(m_reader);
    pd_binary_writer_reset(m_currentWriter);

    // update interfaces with data

    /*
    for (int i = 0; i < len; ++i) {
        // do stuff here

        PDBinaryReader_reset(m_reader);
    }
    */
}

/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::start()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::stop()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::stepIn()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BackendSession::stepOver()
{
}
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
