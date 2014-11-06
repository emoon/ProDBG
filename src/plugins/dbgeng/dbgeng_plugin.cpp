#ifdef _WIN32

#include <pd_backend.h>

struct DbgEngPlugin
{
	PDDebugState state = PDDebugState_noTarget;
	bool hasValidTarget = false;

	const char* targetName = "";

};

void* createInstance(ServiceFunc* serviceFunc)
{
	DbgEngPlugin* plugin = new DbgEngPlugin;

	return plugin;
}

void destroyInstance(void* userData)
{
	DbgEngPlugin* plugin = reinterpret_cast<DbgEngPlugin*>(userData);
	delete plugin;
}

static PDDebugState update(void* userData, PDAction action, PDReader* reader, PDWriter* writer)
{
	DbgEngPlugin* plugin = reinterpret_cast<DbgEngPlugin*>(userData);

	return plugin->state;
}

static PDBackendPlugin plugin =
{
	"Microsoft Debugger Engine",
	createInstance,
	destroyInstance,
	update,
};

extern "C" PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* privateData)
{
	registerPlugin(PD_BACKEND_API_VERSION, &plugin, privateData);
}

#endif
