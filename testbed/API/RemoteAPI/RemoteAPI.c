#include "../Remote.h"
#include <ProDBGAPI.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif

#if !defined(_WIN32)
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#if !defined(_WIN32)
typedef int SOCKET;
#define INVALID_SOCKET -1
#endif

#define REMOTE_CONSOLE_PORT 1340

static int s_socket = INVALID_SOCKET;
static int s_serverSocket = INVALID_SOCKET; 
static PDBackendPlugin* s_plugin;
static void* s_userData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int socketPoll(SOCKET socket)
{
	struct timeval to = { 0, 0 };
	fd_set fds;

	FD_ZERO(&fds);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
	FD_SET(socket, &fds);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

	return select(socket + 1, &fds, NULL, NULL, &to) > 0;
}

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

static int setupSocket()
{
	struct sockaddr_in sin;
	int yes = 1;

#if defined(_WIN32)
	 WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,0),&wsaData) != 0)
		return 0;
#endif

	s_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (s_serverSocket == INVALID_SOCKET)
		return 0;

	memset(&sin, 0, sizeof sin);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(REMOTE_CONSOLE_PORT);

	if (setsockopt(s_serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		return 0;
	}

	if (-1 == bind(s_serverSocket, (struct sockaddr *)&sin, sizeof(sin)))
	{
		perror("bind");
		printf("Unable to bind server socket\n");
		return 0;
	}

	while (listen(s_serverSocket, SOMAXCONN) == -1)
		;

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_create(struct PDBackendPlugin* plugin, int waitForConnection)
{
	// setup socket

	if (!setupSocket())
		return 0;

	// TODO: Verify that this plugin is ok

	s_plugin = plugin;

	// wait for connection if waitForConnecion > 0

	waitForConnection *= 1000; // count in ms

	while (waitForConnection > 0)
	{
		PDRemote_update();

		if (PDRemote_isConnected())
			break;

		sleepMs(100);
		waitForConnection -= 100;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static SOCKET clientConnect(SOCKET serverSocket, struct sockaddr_in* host)
{
	struct sockaddr_in hostTemp;
	unsigned int hostSize = sizeof(struct sockaddr_in);

	SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&hostTemp, (socklen_t*)&hostSize);

	if (INVALID_SOCKET == clientSocket) 
		return INVALID_SOCKET;

	*host = hostTemp;

	return clientSocket;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateListner()
{
	struct timeval timeout;
	struct sockaddr_in client;
	SOCKET clientSocket = INVALID_SOCKET;
	fd_set fds;

	if (PDRemote_isConnected())
		return;

	FD_ZERO(&fds);
	FD_SET(s_serverSocket, &fds);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	// look for new clients
	
	if (select(s_serverSocket + 1, &fds, NULL, NULL, &timeout) > 0)
	{
		clientSocket = clientConnect(s_serverSocket, &client);

		if (INVALID_SOCKET != clientSocket)
		{
			printf("Connected!\n");
			s_socket = clientSocket; 
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void disconnect()
{
#if defined(_WIN32)
	closesocket(s_socket);
#else
	close(s_socket);
#endif
	s_socket = INVALID_SOCKET;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int localRecv(char* buffer, size_t length, int flags)
{
	int ret;

	if (!PDRemote_isConnected())
		return 0;

	ret = (int)recv(s_socket, buffer, length, flags);

	if (ret != (int)length)
	{
		disconnect();
		return 0;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int localSend(const char* buffer, size_t length, int flags)
{
	int ret;

	if (!PDRemote_isConnected())
		return 0;

	if ((ret = (int)send(s_socket, buffer, length, flags)) != (int)length)
	{
		disconnect();
		return 0;
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int pollRead()
{
	if (!PDRemote_isConnected())
		return 0;

	return !!socketPoll(s_socket);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void processCommands()
{
	uint32_t command;

	if (!localRecv((char*)&command, 4, 0)) 
		return;

	// top 16-bits = type of command
	// lower 16-bits = action/event

	command = htonl(command);	// make sure we have right endian format

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

int PDRemote_update()
{
	updateListner();

	if (pollRead())
	{
		// process commandds here

	}

	return 1;
}

