#pragma once

#include "shared_object.h"

struct PDBackendPlugin;
class PDView;
class PDMemoryView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct BackendPlugin {
    void* data;
    const char* type;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Pass in log handler here

class PluginHandler {
public:
    static void add_plugin(const char* filename);

    static PDBackendPlugin* find_backend_plugin(const char* name);
    static PDView* find_view_plugin(const char* name);
    static PDMemoryView* find_memory_view_plugin(const char* name);

    static PDBackendPlugin* get_backend_plugins(int* count);
    static PDView* get_view_plugins(int* count);
    static PDMemoryView* get_memory_view_plugins(int* count);
};

