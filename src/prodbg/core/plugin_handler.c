#include "plugin_handler.h"
#include "log.h"
#include "core.h"
#include <pd_common.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <uv.h>
#include <stb.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Plugin* s_plugins;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void registerPlugin(const char* type, void* data)
{
    Plugin plugin;

    plugin.data = data;
    plugin.type = type;

    log_debug("Register plugin (type %s data %p)\n", type, data);

	stb_arr_push(s_plugins, plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int openPlugin(const char* basePath, const char* plugin, uv_lib_t* lib)
{
	char filename[8192];

#ifdef PRODBG_MAC
	sprintf(filename, "%s/lib%s.dylib", basePath, plugin);
#elif PRODBG_WIN
	sprintf(filename, "%s\\%s.dll", basePath, plugin);
#else
	sprintf(filename, "%s/%s.so", basePath, plugin);
#endif

	return uv_dlopen(filename, lib);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin)
{
	uv_lib_t lib;
	void* function;

    void* (*initPlugin)(int version, ServiceFunc* serviceFunc, RegisterPlugin* registerPlugin);

	if (openPlugin(basePath, plugin, &lib) == -1)
	{
		log_error("Unable to open %s error:", uv_dlerror(&lib))
        return false;
	}

	if (uv_dlsym(&lib, "InitPlugin", &function) == -1)
	{
        log_error("Unable to find InitPlugin function in plugin %s\n", plugin);
		uv_dlclose(&lib);
		return false;
	}

    *(void**)(&initPlugin) = function;

    initPlugin(1, 0, registerPlugin);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Plugin* PluginHandler_getPlugins(int* count)
{
	*count = stb_arr_len(s_plugins);
    return s_plugins;
}

