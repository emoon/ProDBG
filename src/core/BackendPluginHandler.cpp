#include "BackendPluginHandler.h"
#include <vector>
#include <pd_common.h>
#include "Logging.h"

#ifndef _WIN32
#include <libgen.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg {

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

bool BackendPluginHandler::add_plugin(const char* filename) {
    char cwd[PATH_MAX] = {0};
    char fullname[PATH_MAX * 2] = {0};

    ssize_t size = readlink("/proc/self/exe", cwd, sizeof(cwd));
    if (size == -1) {
        log_error("Unable to get executable path: error %s\n", strerror(errno));
        return false;
    }

    char* path = dirname(cwd);

    sprintf(fullname, "%s/lib%s.so", path, filename);
    void* handle = dlopen(fullname, RTLD_LAZY);

    if (!handle) {
        log_error("Unable to open %s\n (error %s)\n", fullname, dlerror());
        return false;
    }

    // reset errors;
    dlerror();

    InitPlugin init_plugin = (InitPlugin)dlsym(handle, "pd_init_plugin");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        log_error("Unable to open find \"pd_init_plugin\" in %s (error %s)\n", fullname, dlsym_error);
        dlclose(handle);
        return false;
    }

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

};  // namespace prodbg
