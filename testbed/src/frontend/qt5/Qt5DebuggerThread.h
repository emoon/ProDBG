#pragma once

#include <QObject>
#include <QTimer>
#include <QStringList>
#include "ProDBGAPI.h"
#include "Core/DataPacket.h"

struct RemoteConnection;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5DebuggerThread : public QObject
{
    Q_OBJECT

public:

    enum TargetType
    {
        Local,
        Remote
    };

    Qt5DebuggerThread(TargetType type);
    void setRemoteTarget(const char* hostName, int port);

public:
    QTimer* m_timer;

public slots:
    void start();
    void update();
    void doAction(int action);
    void setState(uint8_t* serializeData, int size);
 
signals:
    void finished();

    // data is here to be set to the serializer. Not very elgent so we should maybe wrap it in something better
    void sendData(uint8_t* serializedata, int size);


private:
    
    void updateLocal(void* readData, int readSize, int action);

    PDDebugState m_debugState;
    PDBackendPlugin* m_debuggerPlugin;
    void* m_pluginData;
    const char* m_targetHost;
    RemoteConnection* m_connection;
    int m_port;
    TargetType m_targetType;

    PDReader m_reader;
    PDWriter m_writer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

