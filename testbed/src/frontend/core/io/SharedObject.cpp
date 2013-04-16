#include "SharedObject.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__APPLE__)
#include <dlfcn.h>
#elif defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#error "Unsupported platform"
#endif

namespace prodbg
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Handle SharedObject_open(const char* filename)
{
#if defined(__APPLE__)
	return dlopen(filename, RTLD_LOCAL | RTLD_LAZY);
#elif defined(_WINN32)
	return (Handle)LoadLibrary(filename);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SharedObject_close(Handle handle)
{
#if defined(__APPLE__)
	dlclose(handle);
#elif defined(_WIN32)
	return (Handle)LoadLibrary((HMODULE)handle);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* SharedObject_getSym(Handle handle, const char* name)
{
#if defined(__APPLE__)
	return dlsym(handle, name);
#elif defined(_WIN32)
	return (void*)GetProcAddress(handle, name);
#endif
}

}

