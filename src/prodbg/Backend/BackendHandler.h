#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <stdint.h>

namespace progbg {

class Session;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BackendHandler : public QObject
{
    Q_OBJECT

public:
	// Slots
    Q_SLOT void stepOver();
    Q_SLOT void stepIn();
    Q_SLOT void stop();
    Q_SLOT void requestMemory(uint64_t lo, uint64_t hi, QVector<uint16_t>* target);

    // Signals
    Q_SIGNAL void responseMemory(QVector<uint16_t>* res, uint16_t address);
    Q_SIGNAL void statusUpdate(QString update);

private:
    Session* m_session;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
