#include "../Remote.h"
#include "RemoteConnection.h"
#include <ProDBGAPI.h>

#include <stdint.h>
#include <stdio.h>

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
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_update(int sleepTime)
{
	if (sleepTime > 0)
		sleepMs(sleepTime);

	RemoteConnection_updateListner(s_conn);

	if (RemoteConnection_pollRead(s_conn))
		processCommands();

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_isConnected()
{
	return RemoteConnection_isConnected(s_conn);
}

