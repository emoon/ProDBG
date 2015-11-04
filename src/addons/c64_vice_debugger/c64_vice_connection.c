#include "c64_vice_connection.h"
#include <string.h>
#include <stdint.h>

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

#if defined(__linux__)
#include <sys/time.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#if !defined(_WIN32)
#define closesocket close
#endif

//#define DEBUG_LOG

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void debug_log(const char* fmt, ...) {
#ifdef DEBUG_LOG
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
#else
    (void)fmt;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct VICEConnection {
    enum VICEConnectionType type;

    int serverSocket;     // used when having a listener socket
    int socket;

} VICEConnection;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int socketPoll(int socket) {
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

static int createListner(VICEConnection* conn, int port) {
    struct sockaddr_in sin;
    int yes = 1;

    conn->serverSocket = (int)socket(AF_INET, SOCK_STREAM, 0);

    if (conn->serverSocket == INVALID_SOCKET)
        return 0;

    memset(&sin, 0, sizeof sin);

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons((unsigned short)port);

    if (setsockopt(conn->serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return 0;
    }

    if (-1 == bind(conn->serverSocket, (struct sockaddr*)&sin, sizeof(sin))) {
        perror("bind");
        return 0;
    }

    while (listen(conn->serverSocket, SOMAXCONN) == -1)
        ;

    debug_log("Created listener\n");

    return 1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VICEConnection* VICEConnection_create(enum VICEConnectionType type, int port) {
    VICEConnection* conn = 0;

#if defined(_WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0)
        return 0;
#endif

    conn = malloc(sizeof(VICEConnection));

    conn->type = type;
    conn->serverSocket = INVALID_SOCKET;
    conn->socket = INVALID_SOCKET;

    if (type == VICEConnectionType_Listener) {
        if (!createListner(conn, port)) {
            free(conn);
            return 0;
        }
    }

    return conn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_connect(VICEConnection* conn, const char* address, int port) {
    struct hostent* he;
    struct sockaddr_in sa = { 0 };
    char** ap;
    int sock = INVALID_SOCKET;

    debug_log("Trying to connect\n");

    he = gethostbyname(address);

    if (!he) {
        debug_log("Failed to get hostname\n");
        return 0;
    }

    for (ap = he->h_addr_list; *ap; ++ap) {
    #ifndef _WIN32
        sa.sin_family = (sa_family_t)he->h_addrtype;
    #else
        sa.sin_family = (unsigned short)he->h_addrtype;
    #endif
        sa.sin_port = htons((unsigned short)port);

        memcpy(&sa.sin_addr, *ap, he->h_length);

        sock = (int)socket(he->h_addrtype, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET)
            continue;

        if (connect(sock, (struct sockaddr*)&sa, sizeof(sa)) >= 0)
            break;

        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    if (sock == INVALID_SOCKET) {
        debug_log("No socket to connect to\n");
        return 0;
    }

    conn->socket = sock;

    debug_log("Connected!\n");

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VICEConnection_destroy(struct VICEConnection* conn) {
    if (conn->socket != INVALID_SOCKET)
        closesocket(conn->socket);

    if (conn->serverSocket != INVALID_SOCKET)
        closesocket(conn->serverSocket);

    free(conn);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_connected(struct VICEConnection* conn) {
    return conn->socket != INVALID_SOCKET;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int clientConnect(VICEConnection* conn, struct sockaddr_in* host) {
    struct sockaddr_in hostTemp;
    unsigned int hostSize = sizeof(struct sockaddr_in);

    debug_log("Trying to accept\n");

    conn->socket = (int)accept(conn->serverSocket, (struct sockaddr*)&hostTemp, (socklen_t*)&hostSize);

    if (INVALID_SOCKET == conn->socket) {
        perror("accept");
        debug_log("Unable to accept connection..\n");
        return 0;
    }

    if (NULL != host)
        *host = hostTemp;

    debug_log("Accept done\n");

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VICEConnection_updateListner(VICEConnection* conn) {
    struct timeval timeout;
    struct sockaddr_in client;
    fd_set fds;

    FD_ZERO(&fds);

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127)
#endif
    FD_SET(conn->serverSocket, &fds);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    if (VICEConnection_isConnected(conn))
        return;

    // look for new clients

    if (select(conn->serverSocket + 1, &fds, NULL, NULL, &timeout) > 0) {
        if (clientConnect(conn, &client))
            debug_log("Connected to %s\n", inet_ntoa(client.sin_addr));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_disconnect(VICEConnection* conn) {
    debug_log("Disconnected\n");

    if (conn->socket != INVALID_SOCKET)
        closesocket(conn->socket);

    conn->socket = INVALID_SOCKET;

    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_recv(VICEConnection* conn, char* buffer, int length, int flags) {
    int ret;

    if (!VICEConnection_connected(conn))
        return 0;

    ret = (int)recv(conn->socket, buffer, (size_t)length, flags);

    if (ret <= 0) {
        debug_log("recv %d %d\n", ret, length);
        VICEConnection_disconnect(conn);
        return 0;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_send(VICEConnection* conn, const void* buffer, int length, int flags) {
    int ret;

    if (!VICEConnection_connected(conn))
        return 0;

    if ((ret = (int)send(conn->socket, buffer, (size_t)length, flags)) != (int)length) {
        VICEConnection_disconnect(conn);
        return 0;
    }

    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_sendStream(VICEConnection* conn, const unsigned char* buffer) {
    int sizeCount = 0;
    // stream has the size at the very start and 2 top bits used for other things
    int32_t size = ((buffer[0] & 0x3f) << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];

    //debug_log("Going to send stream of size %d\n", size);

    while (size != 0) {
        uint32_t sizeLeft = size > 1024 ? 1024 : (uint32_t)size;

        int sent = VICEConnection_send(conn, buffer, (int)sizeLeft, 0);

        //debug_log("sent %d out of %d bytes\n", sent, size);

        if (sent == 0)
            return sizeCount;

        buffer += 1024;
        size -= sizeLeft;
        sizeCount += sizeLeft;
    }

    return sizeCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char* VICEConnection_recvStream(VICEConnection* conn, unsigned char* outputBuffer, int size) {
    uint8_t* retBuffer = outputBuffer;
    uint8_t ownBuffer = 0;

    if (!outputBuffer) {
        outputBuffer = retBuffer = malloc((size_t)size);
        memset(outputBuffer, 0xcd, size);
        ownBuffer = 1;
    }

    outputBuffer[0] = (size >> 24) & 0xff;
    outputBuffer[1] = (size >> 16) & 0xff;
    outputBuffer[2] = (size >> 8) & 0xff;
    outputBuffer[3] = (size >> 0) & 0xff;

    outputBuffer += 4;
    size -= 4;

    //debug_log("about to get data (expected size %d)\n", size);
    //debug_log("filling buffer %p\n", outputBuffer);

    while (size != 0) {
        uint32_t currSize = size > 1024 ? 1024 : (uint32_t)size;

        int ret = VICEConnection_recv(conn, (char*)outputBuffer, (int)currSize, 0);

        //debug_log("got size %d (%d)\n", ret, currSize);

        if (ret <= 0) {
            debug_log("Lost connection or error :(\n");

            if (ownBuffer)
                free(outputBuffer);

            return 0;
        }

        outputBuffer += 1024;

        size -= currSize;
    }

    return retBuffer;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_pollRead(VICEConnection* conn) {
    if (!VICEConnection_connected(conn))
        return 0;

    return !!socketPoll(conn->socket);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_isConnected(VICEConnection* conn) {
    if (conn == NULL)
        return 0;

    return conn->socket != INVALID_SOCKET;
}


