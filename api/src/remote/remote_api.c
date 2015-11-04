#include "pd_readwrite_private.h"
#include "remote_connection.h"
#include <pd_backend.h>
#include <pd_remote.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#include <arpa/inet.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Winsock2.h>
#endif

static struct RemoteConnection* s_conn;
static struct PDBackendPlugin* s_plugin;
static void* s_userData;

static PDWriter s_writerData;
static PDReader s_readerData;

static PDWriter* s_writer;
static PDReader* s_reader;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
    BlockSize = 1024,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void sleepMs(int ms) {
#ifdef _MSC_VER
    Sleep(ms);
#else
    usleep((unsigned int)(ms * 1000));
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_create(struct PDBackendPlugin* plugin, int waitForConnection) {
    s_conn = RemoteConnection_create(RemoteConnectionType_Listener, 1340);

    if (!s_conn)
        return 0;

    s_writer = &s_writerData;
    s_reader = &s_readerData;

    PDBinaryReader_init(s_reader);

    // \todo Verify that this plugin is ok
    s_plugin = plugin;
    s_userData = plugin->create_instance(0);

    // wait for connection if waitForConnecion > 0

    waitForConnection *= 1000; // count in ms

    while (waitForConnection > 0) {
        PDRemote_update(100);

        if (RemoteConnection_isConnected(s_conn))
            break;

        waitForConnection -= 100;

    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_update(int sleepTime) {
    PDDebugState state;
    uint8_t* recvData = 0;
    int recvSize = 0;
    int action = 0;
    void* data;
    uint32_t size;

    if (sleepTime > 0)
        sleepMs(sleepTime);

    RemoteConnection_updateListner(s_conn);

    // Check if we have some data on the incoming connection

    if (RemoteConnection_pollRead(s_conn)) {
        uint8_t cmd[4];

        if (RemoteConnection_recv(s_conn, (char*)&cmd, 4, 0)) {
            if (cmd[0] & (1 << 7)) {
                action = (cmd[2] << 8) | cmd[3];
            }else {
                recvSize  = ((cmd[0] & 0x3f) << 24) | (cmd[1] << 16) | (cmd[2] << 8) | cmd[3];

                if ((recvData = RemoteConnection_recvStream(s_conn, 0, recvSize)) == 0) {
                    printf("Unable to get data from stream\n");
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

    //PDWrite_event_begin(s_writer, PDEventType_setStatus);
    //PDWrite_u32(s_writer, "state", (uint32_t)state);
    //PDWrite_event_end(s_writer);

    PDBinaryWriter_finalize(s_writer);

    size = PDBinaryWriter_getSize(s_writer);
    data = PDBinaryWriter_getData(s_writer);

    // make sure to only send data if we have something to send (4 is only the size with no data)

    if (size > 4 && RemoteConnection_isConnected(s_conn)) {
        RemoteConnection_sendStream(s_conn, data);
    }

    free(recvData);
    free(data);

    PDBinaryWriter_destroy(s_writer);

    return PDRemote_isConnected();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int PDRemote_isConnected() {
    return RemoteConnection_isConnected(s_conn);
}

