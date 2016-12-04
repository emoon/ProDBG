#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <stdint.h>

namespace prodbg {

class Session;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BackendHandler : public QObject
{
    Q_OBJECT

public:
    BackendHandler(Session* session);

public:
    // Slots
    // Q_SLOT void stepOver();
    // Q_SLOT void stepIn();
    // Q_SLOT void stop();
    Q_SLOT void requestMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);

    // Signals
    Q_SIGNAL void responseMemory(QVector<uint16_t>* res, uint64_t address);
    Q_SIGNAL void programCounterChanged(uint64_t pc);
    Q_SIGNAL void statusUpdate(QString update);

private:
    Session* m_session;
    uint64_t m_currentPc; /// dummy
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
