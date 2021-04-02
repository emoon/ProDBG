#include "backend_plugin_handler.h"
#include <pd_common.h>
#include <vector>
#include "logging.h"
#include "shared_object.h"

#ifndef _WIN32
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#endif

#if defined(PRODBG_NIX)
#include <dlfcn.h>
#include <errno.h>
#include <libgen.h>
#include <unistd.h>
#endif

#if defined(PRODBG_LINUX)
#define SO_PREFIX "lib"
#define SO_SUFFIX ".so"
#elif defined(PRODBG_MAC)
#define SO_PREFIX "lib"
#define SO_SUFFIX ".dylib"
#elif defined(PRODBG_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define SO_PREFIX ""
#define SO_SUFFIX ".dll"
#else
#error unsupported platform
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<BackendPlugin> s_plugins;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void register_plugin(const char* type, void* data, void* private_data) {
    BackendPlugin plugin;

    plugin.data = data;
    plugin.type = type;

    s_plugins.push_back(plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void* (*InitPlugin)(RegisterPlugin* register_plugin, void* private_data);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool BackendPluginHandler::add_plugin(const char* name) {
    const char* base_path = shared_object_exe_directory();

    // TODO: Better error handling
    if (base_path == nullptr)
        base_path = "";

    void* handle = shared_object_open(base_path, name);

    if (!handle) {
        printf("Unable to open plugin: %s error: %s", name, shared_object_error());
        return false;
    }

    InitPlugin init_plugin = (InitPlugin)shared_object_symbol(handle, "pd_init_plugin");

    if (!init_plugin) {
        printf("Unable to open find \"pd_init_plugin\" in %s\n", name);
    }

    printf("Register plugin %s\n", name);

    init_plugin(register_plugin, nullptr);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin* BackendPluginHandler::find_plugin(const char* name) {
    for (const auto& plugin : s_plugins) {
        PDPluginBase* base = (PDPluginBase*)plugin.data;
        if (!strcmp(name, base->name)) {
            return (PDBackendPlugin*)plugin.data;
        }
    }

    return nullptr;
}

