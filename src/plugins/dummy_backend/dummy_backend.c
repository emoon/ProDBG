#include "pd_backend.h"
#include "pd_host.h"
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DummyPlugin {
	int dummy;

} DummyPlugin;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* create_instance(ServiceFunc* serviceFunc) {
	(void)serviceFunc;

	DummyPlugin* plugin = (DummyPlugin*)malloc(sizeof(DummyPlugin));
	memset(plugin, 0, sizeof(DummyPlugin));

    return plugin;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void destroy_instance(void* user_data) {
	free(user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDDebugState update(void* user_data, 
						   PDAction action, 
						   PDReader* reader, 
						   PDWriter* writer) {
    DummyPlugin* plugin = (DummyPlugin*)user_data;
    (void)action;
    (void)reader;
    (void)writer;

    return PDDebugState_NoTarget;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PDBackendPlugin plugin =
{
    "Dummy Backend",
    create_instance,
    destroy_instance,
    0,
    update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_BACKEND_API_VERSION, &plugin, private_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
