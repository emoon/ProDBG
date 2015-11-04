#pragma once

struct VICEConnection;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum VICEConnectionType {
    VICEConnectionType_Listener,
    VICEConnectionType_Connect
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VICEConnection* VICEConnection_create(enum VICEConnectionType type, int port);
void VICEConnection_destroy(struct VICEConnection* connection);

void VICEConnection_updateListner(struct VICEConnection* conn);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int VICEConnection_isConnected(struct VICEConnection* connection);
int VICEConnection_connect(struct VICEConnection* connection, const char* address, int port);
int VICEConnection_disconnect(struct VICEConnection* connection);

int VICEConnection_recv(struct VICEConnection* connection, char* buffer, int length, int flags);
int VICEConnection_send(struct VICEConnection* connection, const void* buffer, int length, int flags);
int VICEConnection_pollRead(struct VICEConnection* connection);

int VICEConnection_sendStream(struct VICEConnection* connection, const unsigned char* buffer);
unsigned char* VICEConnection_recvStream(struct VICEConnection* connection, unsigned char* out, int size);

