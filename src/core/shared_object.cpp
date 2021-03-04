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

char* shared_object_error() {
#if defined(PRODBG_WINDOWS)
    static char error[512];
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error, sizeof(error), NULL);
    return error;
#elif defined(PRODBG_NIX)
    return dlerror();
#else
#error unsuppored platform
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TODO: Widechar on Windows

char* shared_object_exe_directory() {
    static char base_path[8192];

#if defined(PRODBG_WINDOWS)
    if (GetModuleFileNameA(nullptr, base_path, sizeof(base_path)) == 0) {
        printf("Unable to get executable path: %s", shared_object_error());
        return nullptr;
    }
#elif defined(PRODBG_LINUX)
    if (readlink("/proc/self/exe", base_path, sizeof(base_path)) == -1) {
        printf("Unable to get executable path: error %s\n", strerror(errno));
        return nullptr;
    }
#elif defined(PRODBG_MAC)
    uint32_t size = sizeof(base_path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        printf("Unable to get executable path, size is to small %d", size);
        return nullptr;
    }
#else
#error "Unsupported platform"
#endif
    // find the first / or \ backwards in the output path and set that as end point
    for (int i = strlen(base_path) - 1; i != 0; --i) {
        if (base_path[i] == '/' || base_path[i] == '\\') {
            base_path[i] = 0;
            break;
        }
    }

    return base_path;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* shared_object_open(const char* base_path, const char* name) {
    void* handle = NULL;
    char path[8192];

    // TODO: Support widechar on windows
    sprintf(path, "%s/%s%s%s", base_path, SO_PREFIX, name, SO_SUFFIX);

#if defined(PRODBG_WINDOWS)
    handle = (void*)LoadLibraryA(path);
#elif defined(PRODBG_NIX)
    handle = dlopen(path, RTLD_LOCAL | RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "can't dlopen '%s': %s\n", path, dlerror());
        return nullptr;
    }
#else
#error unsupported platform
#endif

    return handle;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* shared_object_symbol(void* handle, const char* symbol_name) {
#if defined(PRODBG_WINDOWS)
    return GetProcAddress((HMODULE)handle, symbol_name);
#elif defined(PRODBG_NIX)
    return dlsym(handle, symbol_name);
#else
#error unsupported platform
#endif
}

