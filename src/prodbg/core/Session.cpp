#include "Session.h"
#include "core/Alloc.h"
#include "api/PluginInstance.h"
#include "api/src/remote/PDReadWrite_private.h"
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

struct Session
{
	Session() : backend(nullptr)
	{
	}

    PDReader reader;
	PDWriter backendWriter;
	PDWriter viewPluginsWriter;
	PDBackendInstance* backend;
	std::vector<ViewPluginInstance*> viewPlugins;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Session* Session_create()
{
	Session* s = new Session; 
	PDBinaryWriter_init(&s->backendWriter);
	PDBinaryWriter_init(&s->viewPluginsWriter);
    PDBinaryReader_init(&s->reader);

	return s;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_destroy(Session* session)
{
	delete session;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Session_update(Session* s, PDAction action)
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

