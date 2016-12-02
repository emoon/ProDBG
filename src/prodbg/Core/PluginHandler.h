#pragma once

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Plugin
{
	void* data;
	const char* type;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool PluginHandler_addPlugin(const char* basePath, const char* plugin);
Plugin* PluginHandler_getPlugins(int* count);

}

