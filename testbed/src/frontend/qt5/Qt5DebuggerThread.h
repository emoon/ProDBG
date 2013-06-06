#pragma once

#include <QObject>
#include <QTimer>
#include <QStringList>
#include "ProDBGAPI.h"
#include "Core/DataPacket.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Qt5DebuggerThread : public QObject
{
	Q_OBJECT

public:
	Qt5DebuggerThread();

public slots:
    void start();
	void update();
	void tryStep();
	void tryStartDebugging();
	void getData(void* serializeData);
 
signals:
    void finished();

    // data is here to be set to the serializer. Not very elgent so we should maybe wrap it in something better
    void sendData(void* serializedata);

private:
	
	void sendState();

	PDDebugState m_debugState;
	PDBackendPlugin* m_debuggerPlugin;
	void* m_pluginData;
	QTimer m_timer;

	PDSerializeRead m_reader;
	PDSerializeWrite m_writer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

