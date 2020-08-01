#include "BackendPluginHandler.h"
#include <vector>
#include <pd_common.h>

#ifndef _WIN32
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

    printf("Register plugin (type %s data %p)\n", type, data);

    s_plugins.push_back(plugin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void* (*InitPlugin)(RegisterPlugin* register_plugin, void* private_data);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Pass in log handler here

bool BackendPluginHandler::add_plugin(const char* filename) {
    char cwd[PATH_MAX] = {0};
    char fullname[PATH_MAX * 2] = {0};
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
        // TODO: Use logging API
        printf("BackendPluginHandler: Unable to get cwd\n");
        return false;
    }

    sprintf(fullname, "%s/%s", cwd, filename);
    void* handle = dlopen(fullname, RTLD_LAZY);

    if (!handle) {
        // TODO: Use logging API
        printf("BackendPluginHandler: Unable to open %s\n (error %s)", fullname, dlerror());
        return false;
    }

    // reset errors;
    dlerror();

    InitPlugin init_plugin = (InitPlugin)dlsym(handle, "pd_init_plugin");
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        // TODO: Use logging API
        printf("BackendPluginHandler: Unable to open find \"pd_init_plugin\" in %s\n (error %s)", fullname, dlsym_error);
        dlclose(handle);
        return 1;
    }

    init_plugin(register_plugin, nullptr);

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin* BackendPluginHandler::find_plugin(const char* name) {
    (void)name;
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

};  // namespace prodbg
