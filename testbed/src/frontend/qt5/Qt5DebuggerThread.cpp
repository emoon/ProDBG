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

Qt5DebuggerThread::Qt5DebuggerThread(Qt5DebuggerThread::TargetType type) : 
	m_debugState(PDDebugState_noTarget),
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
	PDBinaryReader_initStream(&m_reader, serializeData, serSize);

	// \todo This will alloc memory all the time but we may not use it. better to use some prealloced chunks?
	
	PDBinaryWriter_init(&m_writer);

	if (m_debuggerPlugin->update(m_pluginData, (PDAction)action, &m_reader, &m_writer) == PDDebugState_running)
	{
		// if timer isn't active at this point we should start it
		if (!m_timer.isActive())
			m_timer.start(10);
	}

	// if we have some data written in the update lets send it back

	uint32_t size = PDBinaryWriter_getSize(&m_writer);
	void* data = PDBinaryWriter_getData(&m_writer);

	if (size > 0)
		emit sendData(data, size);
	else
		free(data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This gets called from the DebugSession (which is on the UI thread) and when the data gets here this thread has
// owner ship of the data and will release it when done with it

void Qt5DebuggerThread::setState(void* serializeData, int serSize)
{
	if (m_targetType == Local)
	{
		updateLocal(serializeData, serSize, 0);
	}
	else
	{
		if (RemoteConnection_isConnected(m_connection))
		{
			uint32_t tempBuffer[1024];

			if (serSize > (int)(sizeof(tempBuffer) - 4))
			{
				printf("Unable to send data as size is bigger than supported buffer (%d %d)!\n", (int)sizeof(tempBuffer), serSize);
				return;
			}

			tempBuffer[0] = htonl(1 << 16);
			memcpy((char*)&tempBuffer[1], serializeData, serSize);

			printf("Sending request to target (size %d)\n", serSize + (int)sizeof(uint32_t));
			RemoteConnection_send(m_connection, tempBuffer, serSize + sizeof(uint32_t), 0);

			// \todo: Merge into 1 buffer? Not sure if this is really save
			//uint32_t command = htonl(1 << 16);
			//RemoteConnection_send(m_connection, &command, sizeof(uint32_t), 0);
			//RemoteConnection_send(m_connection, serializeData, serSize, 0);
		}
	}

	// After we finished reading the data we free it up
	free(serializeData);
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
		int retSize;

		// \todo Growing buffer for large transfers
		uint8_t* tempBuffer = (uint8_t*)malloc(1024 * 1024); 	// temprory

		if (!RemoteConnection_pollRead(m_connection))
			return;

		while (1)
		{
			if ((retSize = RemoteConnection_recv(m_connection, (char*)&tempBuffer[totalSize], 1024, 0)) != 1024)
				break;

			totalSize += 1024;
		}

		totalSize += retSize;

		emit sendData(tempBuffer, totalSize);
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
			uint32_t command = htonl(0 << 16 | action);
			RemoteConnection_send(m_connection, &command, sizeof(uint32_t), 0);
		}
	}
}

}

