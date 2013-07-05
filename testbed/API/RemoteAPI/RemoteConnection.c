#include "RemoteConnection.h"
#include <string.h>

#if defined(_MSC_VER)
#pragma warning(disable: 4496)
#include <winsock2.h>
#pragma warning(default: 4496)
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

#include <stdio.h>
#include <stdlib.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#if !defined(_WIN32)
#define closesocket close
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct RemoteConnection
{
	enum RemoteConnectionType type;

	int serverSocket; 	// used when having a listener socket
	int socket;

} RemoteConnection;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline int socketPoll(int socket)
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

static int createListner(RemoteConnection* conn, int port)
{
	struct sockaddr_in sin;
	int yes = 1;

	conn->serverSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (conn->serverSocket == INVALID_SOCKET)
		return 0;

	memset(&sin, 0, sizeof sin);

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	if (setsockopt(conn->serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1)
	{
		perror("setsockopt");
		return 0;
	}

	if (-1 == bind(conn->serverSocket, (struct sockaddr *)&sin, sizeof(sin)))
	{
		perror("bind");
		return 0;
	}

	while (listen(conn->serverSocket, SOMAXCONN) == -1)
		;

	printf("Created listener\n");

	return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RemoteConnection* RemoteConnection_create(enum RemoteConnectionType type, int port)
{
	RemoteConnection* conn = 0;

#if defined(_WIN32)
	 WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,0),&wsaData) != 0)
		return 0;
#endif

	conn = malloc(sizeof(RemoteConnection));

	conn->type = type;
	conn->serverSocket = INVALID_SOCKET;
	conn->socket = INVALID_SOCKET;

	if (type == RemoteConnectionType_Listener)
	{
		if (!createListner(conn, port))
		{
			free(conn);
			return 0;
		}
	}

	return conn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoteConnection_connect(RemoteConnection* conn, const char* address, int port)
{
	struct hostent* he;
	struct sockaddr_in sa;
	char** ap;
	int sock = INVALID_SOCKET;

	printf("Trying to connect\n");

	he = gethostbyname(address);

	if (!he)
	{
		printf("Failed to get hostname\n");
		return 0;
	}

	for (ap = he->h_addr_list; *ap; ++ap) 
	{
		sa.sin_family = (sa_family_t)he->h_addrtype;
		sa.sin_port = htons(port);

		memcpy(&sa.sin_addr, *ap, he->h_length);

		sock = socket(he->h_addrtype, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET)
			continue;

		if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) >= 0)
			break;

		closesocket(sock);
		sock = INVALID_SOCKET;
	}

	if (sock == INVALID_SOCKET)
	{
		printf("No socket to connect to\n");
		return 0;
	}

	conn->socket = sock;

	printf("Connected!\n");

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RemoteConnection_destroy(struct RemoteConnection* conn)
{
	if (conn->socket != INVALID_SOCKET)
		closesocket(conn->socket);

	if (conn->serverSocket != INVALID_SOCKET)
		closesocket(conn->serverSocket);

	free(conn);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoteConnection_connected(struct RemoteConnection* conn)
{
	return conn->socket != INVALID_SOCKET;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int clientConnect(RemoteConnection* conn, struct sockaddr_in* host)
{
	struct sockaddr_in hostTemp;
	unsigned int hostSize = sizeof(struct sockaddr_in);

	printf("Trying to accept\n");

	conn->socket = accept(conn->serverSocket, (struct sockaddr*)&hostTemp, (socklen_t*)&hostSize);

	if (INVALID_SOCKET == conn->socket) 
	{
		perror("accept");
		printf("Unable to accept connection..\n");
		return 0;
	}

	if (NULL != host) 
		*host = hostTemp;

	printf("Accept done\n");

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void RemoteConnection_updateListner(RemoteConnection* conn)
{
	struct timeval timeout;
	struct sockaddr_in client;
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(conn->serverSocket, &fds);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
	if (RemoteConnection_isConnected(conn))
		return;

	// look for new clients
	
	if (select(conn->serverSocket + 1, &fds, NULL, NULL, &timeout) > 0)
	{
		if (clientConnect(conn, &client))
			printf("Connected to %s\n", inet_ntoa(client.sin_addr));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoteConnection_disconnect(RemoteConnection* conn)
{
	printf("Disconnected\n");

	if (conn->socket != INVALID_SOCKET)
		closesocket(conn->socket);

	conn->socket = INVALID_SOCKET;

	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoteConnection_recv(RemoteConnection* conn, char* buffer, int length, int flags)
{
	int ret;

	if (!RemoteConnection_connected(conn))
		return 0;

	ret = (int)recv(conn->socket, buffer, (size_t)length, flags);

	if (ret <= 0)
	{
		printf("recv %d %d\n", ret, length);
		RemoteConnection_disconnect(conn);
		return 0;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoteConnection_send(RemoteConnection* conn, const void* buffer, int length, int flags)
{
	int ret;

	if (!RemoteConnection_connected(conn))
		return 0;

	if ((ret = (int)send(conn->socket, buffer, (size_t)length, flags)) != (int)length)
	{
		RemoteConnection_disconnect(conn);
		return 0;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoteConnection_pollRead(RemoteConnection* conn)
{
	if (!RemoteConnection_connected(conn))
		return 0;

	return !!socketPoll(conn->socket);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoteConnection_isConnected(RemoteConnection* conn)
{
	return conn->socket != INVALID_SOCKET;
}


