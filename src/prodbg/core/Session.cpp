#include "Session.h"
#include "core/Alloc.h"
#include "core/Log.h"
#include "api/PluginInstance.h"
#include "api/src/remote/PDReadWrite_private.h"
#include "api/src/remote/RemoteConnection.h"
#include <vector>
#include <PDView.h>
#include <PDBackend.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum
{
	ReadWriteBufferSize = 2 * 1024 * 1024,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum SessionType
{
	Session_Local,
	Session_Remote,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Session
{
	Session() : 
		backend(nullptr), 
		connection(nullptr)
	{
	}

	SessionType type;
    PDReader reader;
	PDWriter backendWriter;
	PDWriter viewPluginsWriter;
	PDBackendInstance* backend;
	RemoteConnection* connection;
	std::vector<ViewPluginInstance*> viewPlugins;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_create(const char* target, int port)
{
	Session* s = new Session; 
	PDBinaryWriter_init(&s->backendWriter);
	PDBinaryWriter_init(&s->viewPluginsWriter);
    PDBinaryReader_init(&s->reader);

	s->type = Session_Local;

    // TODO: DebuggerThread

	if (strcmp(target, ""))
	{
		s->type = Session_Remote;

        RemoteConnection* conn = RemoteConnection_create(RemoteConnectionType_Connect, port);

        if (!RemoteConnection_connect(conn, target, port))
        {
        	// TODO: use statusbar here
            log_error("Unable to connect to %s:%d\n", target, port);
            RemoteConnection_destroy(conn);
        }
		else
		{
			s->connection = conn;
		}

		log_debug("[SESSION] connected to %s:%d\n", target, port);
	}
	else
	{
		log_debug("[SESSION] Setting up local\n");
	}

	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_destroy(Session* session)
{
	delete session;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateLocal(Session* s, PDAction action)
{
	PDBackendInstance* backend = s->backend;

    PDBinaryReader_initStream(
    	&s->reader, 
    	PDBinaryWriter_getData(&s->viewPluginsWriter), 
    	PDBinaryWriter_getSize(&s->viewPluginsWriter));

	if (backend)
	{
		PDBinaryWriter_reset(&s->backendWriter);
		backend->plugin->update(backend->userData, action, &s->reader, &s->backendWriter);
    	PDBinaryWriter_finalize(&s->backendWriter);
	}

    PDBinaryReader_initStream(
    	&s->reader, 
    	PDBinaryWriter_getData(&s->backendWriter), 
    	PDBinaryWriter_getSize(&s->backendWriter));

	PDBinaryWriter_reset(&s->viewPluginsWriter);

	for (auto p : s->viewPlugins)
	{
		p->plugin->update(p->userData, &p->ui, &s->reader, &s->viewPluginsWriter);
	}

    PDBinaryWriter_finalize(&s->viewPluginsWriter);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void updateRemote(Session* s, PDAction action)
{
	PDBackendInstance* backend = s->backend;

	if (!s->connection)
		return;

	PDBinaryReader_reset(&s->reader);

	if (RemoteConnection_pollRead(s->connection))
	{
        int totalSize = 0;
        uint8_t cmd[4];
        uint8_t* outputBuffer;

        // TODO: Make this a bit less hardcoded (cmd decode)

        if (RemoteConnection_recv(s->connection, (char*)&cmd, 4, 0))
        {
            totalSize = (cmd[0] << 24) | (cmd[1] << 16) | (cmd[2] << 8) | cmd[3];

            outputBuffer = RemoteConnection_recvStream(s->connection, 0, totalSize);

			PDBinaryReader_initStream(&s->reader, outputBuffer, totalSize);
        }
	}

	PDBinaryWriter_reset(&s->viewPluginsWriter);

	for (auto p : s->viewPlugins)
	{
		p->plugin->update(p->userData, &p->ui, &s->reader, &s->viewPluginsWriter);
	}

    PDBinaryWriter_finalize(&s->viewPluginsWriter);

    if (PDBinaryWriter_getSize(&s->viewPluginsWriter) > 4)
	{
		if (PDBinaryWriter_getSize(&s->viewPluginsWriter) > 4 && 
			RemoteConnection_isConnected(s->connection))
		{
			RemoteConnection_sendStream(s->connection, PDBinaryWriter_getData(&s->viewPluginsWriter));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_update(Session* s)
{
	if (s->type == Session_Local)
		updateLocal(s, PDAction_none);
	else
		updateRemote(s, PDAction_none);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_action(Session* s, PDAction action)
{
	if (!s)
		return;

	if (s->type == Session_Local)
	{
		updateLocal(s, action);
	}
	else
	{
        if (RemoteConnection_isConnected(s->connection))
        {
            uint8_t command[4];
            command[0] = 1 << 7; // action tag
            command[1] = 0;
            command[2] = (action >> 8) & 0xff;
            command[3] = (action >> 0) & 0xff;
            RemoteConnection_send(s->connection, &command, sizeof(uint32_t), 0);
        }
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_addViewPlugin(Session* session, ViewPluginInstance* plugin)
{
	session->viewPlugins.push_back(plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_removeViewPlugin(Session* session, ViewPluginInstance* plugin)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

