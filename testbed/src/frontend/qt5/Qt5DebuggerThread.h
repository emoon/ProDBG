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

public slots:
    void start();
	void update();
	void doAction(int action);
	void setState(void* serializeData, int size);
 
signals:
    void finished();

    // data is here to be set to the serializer. Not very elgent so we should maybe wrap it in something better
    void sendData(void* serializedata, int size);

private:
	
	void sendState();

	PDDebugState m_debugState;
	PDBackendPlugin* m_debuggerPlugin;
	void* m_pluginData;
	QTimer m_timer;
	const char* m_targetHost;
	RemoteConnection* m_connection;
	int m_port;
	TargetType m_targetType;

	PDSerializeRead m_reader;
	PDSerializeWrite m_writer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

