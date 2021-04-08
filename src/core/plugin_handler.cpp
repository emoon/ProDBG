#include "plugin_handler.h"
#include "pd_common.h"
#include "pd_backend.h"
#include "pd_ui_register_plugin.h"
#include "pd_memory_view.h"
#include "pd_view.h"
#include <vector>
#include "logging.h"
#include "shared_object.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::vector<PDBackendPlugin*> s_backend_plugins;
static std::vector<PDMemoryView*> s_memory_view_plugins;
static std::vector<PDView*> s_view_plugins;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class RegPlugins : public PDRegisterViewPlugin {
    void register_memory_view(PDMemoryView* view) {
        log_info("added plugin: %s\n", view->name());
        s_memory_view_plugins.push_back(view);
    }

    void register_view(PDView* view) {
        log_info("added view plugin: %s\n", view->name());
        s_view_plugins.push_back(view);
    }
} s_reg_plugins;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void register_backend_plugin(const char* type, void* data, void* private_data) {
    (void)type;
    (void)private_data;
    s_backend_plugins.push_back((PDBackendPlugin*)data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void* (*InitBackendPlugin)(PDRegisterBackendPlugin* register_plugin, void* private_data);
typedef void (*InitView)(PDRegisterViewPlugin* reg);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PluginHandler::add_plugin(const char* path) {
    void* handle = shared_object_open_fullpath(path);
    bool found_plugin = false;

    if (!handle) {
        log_warn("Unable to open plugin: %s error: %s", path, shared_object_error());
        return;
    }

    InitBackendPlugin backend_plugin = (InitBackendPlugin)shared_object_symbol(handle, "pd_register_backend");

    if (backend_plugin) {
        backend_plugin(register_backend_plugin, nullptr);
        log_info("Register backend plugin: %s\n", path);
        found_plugin = true;
    }

    InitView init_view_plugin = (InitView)shared_object_symbol(handle, "pd_register_view");

    if (init_view_plugin) {
        init_view_plugin(&s_reg_plugins);
        found_plugin = true;
    }

    if (!found_plugin) {
        log_info("No plugins were found in %s (searched for \"pd_register_view\" and \"pd_register_backend\"\n", path);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin* PluginHandler::find_backend_plugin(const char* name) {
    for (const auto& t : s_backend_plugins) {
        if (!strcmp(t->name, name)) {
            return t;
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDView* PluginHandler::find_view_plugin(const char* name) {
    for (const auto& t : s_view_plugins) {
        if (!strcmp(t->name(), name)) {
            return t;
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDMemoryView* PluginHandler::find_memory_view_plugin(const char* name) {
    for (const auto& t : s_memory_view_plugins) {
        if (!strcmp(t->name(), name)) {
            return t;
        }
    }

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class T>
inline T* const* get_type_array(const std::vector<T*>& plugins, int* count) {
    *count = int(plugins.size());
    return plugins.data();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDBackendPlugin* const* PluginHandler::get_backend_plugins(int* count) {
    return get_type_array<PDBackendPlugin>(s_backend_plugins, count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDView* const* PluginHandler::get_view_plugins(int* count) {
    return get_type_array<PDView>(s_view_plugins, count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PDMemoryView* const* PluginHandler::get_memory_view_plugins(int* count) {
    return get_type_array<PDMemoryView>(s_memory_view_plugins, count);
}

