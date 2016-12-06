#pragma once

#include <QObject>
#include "IBackendRequests.h"

class QString;
struct PDReader;
struct PDWriter;
struct PDBackendPlugin;

namespace prodbg {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BackendSession : public QObject
{
    Q_OBJECT

public:
    BackendSession();
    ~BackendSession();

    static BackendSession* createBackendSession(const QString& backendName);
    bool setBackend(const QString& backendName);

    // void start();
    // void stop();
    // void stepIn();
    // void stepOver();
    void update();

    Q_SLOT void beginReadMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);
    Q_SLOT void beginDisassembly(uint64_t address, uint32_t count, QVector<IBackendRequests::AssemblyInstruction>* target);

    // Signals
    Q_SIGNAL void endDisassembly(QVector<IBackendRequests::AssemblyInstruction>* instructions, int adressWidth);
    Q_SIGNAL void endReadMemory(QVector<uint16_t>* res, uint64_t address, int addressWidth);
    Q_SIGNAL void programCounterChanged(uint64_t pc);
    Q_SIGNAL void statusUpdate(QString update);

private:
    void destroyPluginData();

    // Writers/Read for communitaction between backend and UI
    PDWriter* m_writer0;
    PDWriter* m_writer1;
    PDWriter* m_currentWriter;
    PDWriter* m_prevWriter;
    PDReader* m_reader;

    // DUMMY, to be removed
    uint64_t m_currentPc;

    // Current active backend plugin
    PDBackendPlugin* m_backendPlugin;

    // Data owned by current active backend plugin
    void* m_backendPluginData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
