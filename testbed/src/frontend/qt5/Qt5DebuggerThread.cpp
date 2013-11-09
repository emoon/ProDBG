#include "Qt5DebuggerThread.h"
#include "Qt5DebugSession.h"
#include <QThread>
#include <core/PluginHandler.h>
#include <core/Log.h>
#include <ProDBGAPI.h>
#include "../../../API/RemoteAPI/RemoteConnection.h"
#include "../../../API/RemoteAPI/PDReadWrite_private.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int32_t getS32(const uint8_t* ptr)
{
	int32_t v = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
	return v; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Qt5DebuggerThread::Qt5DebuggerThread(Qt5DebuggerThread::TargetType type) : 
	m_debugState(PDDebugState_noTarget),
	m_debuggerPlugin(nullptr),
	m_pluginData(nullptr),
	m_targetHost(nullptr),
	m_connection(nullptr),
	m_port(0),
	m_targetType(type)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
void Qt5DebuggerThread::start()
{
	int count;

	PDBinaryReader_init(&m_reader);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));

	if (m_targetType == Local)
	{
		printf("start Qt5DebuggerThread\n");

		// \todo assume the first plugin is our plugin for now

		Plugin* plugin = PluginHandler_getPlugins(&count);

		// try to start debugging session of a plugin

		m_debuggerPlugin = (PDBackendPlugin*)plugin->data;
		m_pluginData = m_debuggerPlugin->createInstance(0);

		printf("end start Qt5DebuggerThread\n");
	}
	else
	{
		RemoteConnection* conn = RemoteConnection_create(RemoteConnectionType_Connect, m_port);

		if (!RemoteConnection_connect(conn, m_targetHost, m_port))
		{
			printf("Unable to connect to %s:%d\n", m_targetHost, m_port);
			RemoteConnection_destroy(conn);
			emit finished();
			return;
		}

		m_connection = conn;

		printf("Connected to %s:%d\n", m_targetHost, m_port);

		m_timer.start(20);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::updateLocal(void* serializeData, int serSize, int action)
{
	PDBinaryReader_initStream(&m_reader, (uint8_t*)serializeData, serSize);

	// \todo This will alloc memory all the time but we may not use it. better to use some prealloced chunks?
	
	PDBinaryWriter_init(&m_writer);

	if (m_debuggerPlugin->update(m_pluginData, (PDAction)action, &m_reader, &m_writer) == PDDebugState_running)
	{
		// if timer isn't active at this point we should start it
		if (!m_timer.isActive())
			m_timer.start(10);
	}

	PDBinaryWriter_finalize(&m_writer);

	// if we have some data written in the update lets send it back

	uint32_t size = PDBinaryWriter_getSize(&m_writer);
	void* data = PDBinaryWriter_getData(&m_writer);

	if (size > 0)
		emit sendData((uint8_t*)data, size);
	else
		free(data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This gets called from the DebugSession (which is on the UI thread) and when the data gets here this thread has
// owner ship of the data and will release it when done with it

void Qt5DebuggerThread::setState(uint8_t* serializeData, int serSize)
{
	if (m_targetType == Local)
	{
		updateLocal(serializeData, serSize, 0);
	}
	else
	{
		if (RemoteConnection_isConnected(m_connection))
		{
			if (serSize > 0)
				RemoteConnection_sendStream(m_connection, serializeData);
		}
	}

	// After we finished reading the data we free it up
	//free(serializeData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::setRemoteTarget(const char* hostName, int port)
{
	m_targetHost = strdup(hostName);
	m_port = port;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::update()
{
	if (m_targetType == Local)
	{
		updateLocal(0, 0, 0);
	}
	else
	{
		int totalSize = 0;
		uint8_t cmd[4];
		uint8_t* outputBuffer;

		if (!RemoteConnection_pollRead(m_connection))
			return;
		
		if (RemoteConnection_recv(m_connection, (char*)&cmd, 4, 0)) 
		{
			totalSize = (cmd[0] << 24) | (cmd[1] << 16) | (cmd[2] << 8) | cmd[3];
			
			outputBuffer = RemoteConnection_recvStream(m_connection, 0, totalSize); 

			if (outputBuffer)
				emit sendData(outputBuffer, totalSize);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Qt5DebuggerThread::doAction(int action)
{
	printf("Qt5DebuggerThread::doAction\n");

	if (m_targetType == Local)
	{
		updateLocal(0, 0, action);
	}
	else
	{
		if (RemoteConnection_isConnected(m_connection))
		{
			uint8_t command[4];
			command[0] = 1 << 7; // action tag
			command[1] = 0;
			command[2] = (action >> 8) & 0xff; 
			command[3] = (action >> 0) & 0xff; 
			RemoteConnection_send(m_connection, &command, sizeof(uint32_t), 0);
		}
	}
}

}

