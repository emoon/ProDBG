#ifndef _REMOTECONNECTION_H_
#define _REMOTECONNECTION_H_

#ifdef __cplusplus
extern "C" {
#endif

struct RemoteConnection;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum RemoteConnectionType
{
	RemoteConnectionType_Listener,
	RemoteConnectionType_Connect,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct RemoteConnection* RemoteConnection_create(enum RemoteConnectionType type, int port);
void RemoteConnection_destroy(struct RemoteConnection* connection);

void RemoteConnection_updateListner(struct RemoteConnection* conn);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int RemoteConnection_isConnected(struct RemoteConnection* connection);
int RemoteConnection_connect(struct RemoteConnection* connection, const char* address, int port);
int RemoteConnection_disconnect(struct RemoteConnection* connection);

int RemoteConnection_recv(struct RemoteConnection* connection, char* buffer, int length, int flags);
int RemoteConnection_send(struct RemoteConnection* connection, const void* buffer, int length, int flags);
int RemoteConnection_pollRead(struct RemoteConnection* connection);

#ifdef __cplusplus
}
#endif

#endif

