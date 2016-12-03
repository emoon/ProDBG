#include "Session.h"
#include "Core/PluginHandler.h"
#include "api/src/remote/pd_readwrite_private.h"
#include <QDebug>
#include <QString>
#include <QStatusBar>
#include <pd_backend.h>
#include <pd_readwrite.h>

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session::Session(QStatusBar* statusbar)
    : m_status(statusbar)
    , m_writer0(new PDWriter)
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

Session::~Session()
{
    destroyPluginData();

    pd_binary_writer_destroy(m_writer0);
    pd_binary_writer_destroy(m_writer1);
    pd_binary_reader_destroy(m_reader);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session::createSession(const QString& backendName, QStatusBar* statusbar)
{
    Session* session = new Session(statusbar);

    if (!session->setBackend(backendName)) {
        delete session;
        return nullptr;
    }

    return session;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Session::setBackend(const QString& backendName)
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

void Session::destroyPluginData()
{
    if (m_backendPlugin && m_backendPluginData) {
        m_backendPlugin->destroy_instance(m_backendPluginData);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session::update()
{
    if (!m_backendPlugin) {
        return;
    }

    // Swap the write buffers
    PDWriter* temp = m_currentWriter;
    m_currentWriter = m_prevWriter;
    m_prevWriter = temp;

    unsigned int reqDataSize = pd_binary_writer_get_size(m_prevWriter);
    pd_binary_reader_reset(m_reader);

    pd_binary_reader_init_stream(m_reader, pd_binary_writer_get_data(m_prevWriter), reqDataSize);
    pd_binary_writer_reset(m_currentWriter);

    int state = m_backendPlugin->update(m_backendPluginData, PDAction_None, m_reader, m_currentWriter);

    if (m_status) {
        QString result;
        QTextStream(&result) << m_backendPlugin->name << " : " << getStateName(state);
        m_status->showMessage(result);
    }

    pd_binary_reader_init_stream(m_reader, pd_binary_writer_get_data(m_prevWriter), pd_binary_writer_get_size(m_prevWriter));
    pd_binary_reader_reset(m_reader);

    // update interfaces with data

    /*
    for (int i = 0; i < len; ++i) {
        // do stuff here

        PDBinaryReader_reset(m_reader);
    }
    */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session::start()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session::stop()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session::stepIn()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session::stepOver()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
