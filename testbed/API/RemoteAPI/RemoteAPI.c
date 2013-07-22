#include "PDReadWrite_private.h"
#include "RemoteConnection.h"
#include <ProDBGAPI.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

static struct RemoteConnection* s_conn;
static struct PDBackendPlugin* s_plugin;
static void* s_userData;

static PDWriter s_writerData;
static PDReader s_readerData;

static PDWriter* s_writer;
static PDReader* s_reader;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void sleepMs(int ms)
{
#ifdef _MSC_VER
	Sleep(ms);
#else
	usleep((unsigned int)(ms * 1000));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_create(struct PDBackendPlugin* plugin, int waitForConnection)
{
	s_conn = RemoteConnection_create(RemoteConnectionType_Listener, 1340);

	if (!s_conn)
		return 0;

	s_writer = &s_writerData;
	s_reader = &s_readerData;

	PDBinaryReader_init(s_reader);

	// \todo Verify that this plugin is ok
	s_plugin = plugin;
	s_userData = plugin->createInstance(0);

	// wait for connection if waitForConnecion > 0

	waitForConnection *= 1000; // count in ms

	while (waitForConnection > 0)
	{
		PDRemote_update(100);

		if (RemoteConnection_isConnected(s_conn))
			break;

		waitForConnection -= 100;

	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_update(int sleepTime)
{
	PDDebugState state;
	void* recvData = 0;
	int recvSize = 0;
	int action = 0;
	void* data;
	uint32_t size;
	
	if (sleepTime > 0)
		sleepMs(sleepTime);

	RemoteConnection_updateListner(s_conn);

	// Check if we have some data on the incoming connection

	if (RemoteConnection_pollRead(s_conn))
	{
		uint32_t type;
		uint32_t command;

		if (RemoteConnection_recv(s_conn, (char*)&command, 4, 0)) 
		{
			command = htonl(command);	// make sure we have right endian format
			type = command >> 16;

			if (type == 0)
			{
				action = (command & 0xffff);
			}
			else if (type == 1)
			{
				recvData = malloc(1024 * 1024);

				if ((recvSize = RemoteConnection_recv(s_conn, recvData, 1024 * 1024, 0)) <= 0)
				{
					printf("Unable to get data from socket (wantedSize %d got Size %d\n", 1024 * 1024, recvSize);
					free(recvData);
					recvData = 0;
					recvSize = 0;
				}
			}
		}
	}

	PDBinaryWriter_init(s_writer);
	PDBinaryReader_initStream(s_reader, recvData, recvSize);

	state = s_plugin->update(s_userData, (PDAction)action, s_reader, s_writer);

	size = PDBinaryWriter_getSize(s_writer);
	data = PDBinaryWriter_getData(s_writer);

	if (size > 0 && RemoteConnection_isConnected(s_conn))
	{
		int retSize = 0;

		if ((retSize = RemoteConnection_send(s_conn, data, size, 0)) != (int)size)
		{
			printf("Unable to send exception location (wanted SendSize %d got Size %d)\n", size, retSize); 
		}
	}

	free(recvData);
	free(data);
	PDBinaryWriter_destroy(s_writer);

	return (int)state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_isConnected()
{
	return RemoteConnection_isConnected(s_conn);
}

