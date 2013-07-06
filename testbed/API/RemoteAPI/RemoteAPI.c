#include "../Remote.h"
#include "BinarySerializer.h"
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
static PDDebugState s_currentState = PDDebugState_running; 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void sleepMs(int ms)
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

static void processCommands()
{
	uint32_t command;

	if (!RemoteConnection_recv(s_conn, (char*)&command, 4, 0)) 
		return;

	// top 16-bits = type of command
	// lower 16-bits = action/event

	command = htonl(command);	// make sure we have right endian format

	printf("got command %04x\n", command);

	// \todo: Make sure to use enum/defines here

	switch (command >> 16)
	{
		// Action
		case 0 :
		{
			s_plugin->action(s_userData, (PDAction)(command & 0xffff));
			break;
		}

		// Event
		case 1:
		{
			void* data;
			int retSize, size;
			PDSerializeWrite writer;
			PDSerializeRead reader;
			PDSerializeRead* readerPtr = &reader;

			// Grab how big buffer we need to allocate
			//RemoteConnection_recv(s_conn, (char*)&size, 4, 0);
			size = 64 * 1024; 

			if (!(data = malloc((size_t)size)))
			{
				printf("Unable to allocate recv buffer (size %d)\n", size);
				return;
			}

			if ((retSize = RemoteConnection_recv(s_conn, data, size, 0)) <= 0)
			{
				printf("Unable to get data from socket (wantedSize %d got Size %d\n", size, retSize);
				return;
			}

			BinarySerializer_initReaderFromStream(readerPtr, data, retSize);
			BinarySerializer_initWriter(&writer);

			while (PDREAD_BYTES_LEFT(readerPtr) > 0)
			{
				int eventSize, type, eventId;

				BinarySerializer_saveReadOffset(readerPtr);

				eventSize = PDREAD_INT(readerPtr);
				type = PDREAD_INT(readerPtr);
				eventId = PDREAD_INT(readerPtr);

				printf("requesting eventSize %d type %d eventId %d\n", eventSize, type, eventId);	

				// getEvent
				// \todo proper enum/constant here

				if (eventId < 0x500)
				{
					BinarySerialize_beginEvent(&writer, (PDEventType)type, 0);
					s_plugin->getState(s_userData, (PDEventType)type, eventId, readerPtr, &writer);
					BinarySerialize_endEvent(&writer);
				}
				else
				{
					s_plugin->setState(s_userData, (PDEventType)type, eventId, readerPtr, &writer);
				}
		
				BinarySerializer_gotoNextOffset(readerPtr, eventSize);
			}

			// Lets send back the data!

			size = BinarySerializer_writeSize(&writer);
			data = BinarySerializer_getStartData(&writer);

			if ((retSize = RemoteConnection_send(s_conn, data, size, 0)) != size)
			{
				// \todo correct logging functions
				printf("Unable to send exception location (wanted SendSize %d got Size %d)\n", size, retSize); 
			}

			//BinarySerializer_destroyData(&writer);
			//BinarySerializer_destroyData(readerPtr);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_update(int sleepTime)
{
	PDDebugState state;

	if (sleepTime > 0)
		sleepMs(sleepTime);

	RemoteConnection_updateListner(s_conn);

	if (RemoteConnection_pollRead(s_conn))
		processCommands();

	state = s_plugin->update(s_userData);

	// take action if state has changed

	if (state != s_currentState)
	{
		printf("changeState (%d %d)\n", state, s_currentState);

		if (state == PDDebugState_stopException && RemoteConnection_isConnected(s_conn))
		{
			int size, retSize, handledException;
			void* ptr;

			PDSerializeWrite writer;
			BinarySerializer_initWriter(&writer);

			// Here we only grab the exception location and the debugger can
			// request more data as seen fit based on that info

			BinarySerialize_beginEvent(&writer, PDEventType_getExceptionLocation, 0);
			handledException = s_plugin->getState(s_userData, PDEventType_getExceptionLocation, 0, 0, &writer);
			BinarySerialize_endEvent(&writer);

			if (!handledException)
			{
				printf("Not proper handling of PDEventType_getExceptionLocation in getState. Debugging won't work correct\n");
				BinarySerializer_destroyData(&writer);
				return 1;
			}
			
			size = BinarySerializer_writeSize(&writer);
			ptr = BinarySerializer_getStartData(&writer);

			printf("sending some stuff over to the debugger (size %d)\n", size);

			if ((retSize = RemoteConnection_send(s_conn, ptr, size, 0)) != size)
			{
				// \todo correct logging functions
				printf("Unable to send exception location (wanted SendSize %d got Size %d)\n", size, retSize); 
			}

			//BinarySerializer_destroyData(&writer);
		}

		s_currentState = state;
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_isConnected()
{
	return RemoteConnection_isConnected(s_conn);
}

