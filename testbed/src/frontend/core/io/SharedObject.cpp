#include "SharedObject.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__APPLE__)
#include <dlfcn.h>
#include <stdio.h>
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
	Handle handle;
#if defined(__APPLE__)
	if (!(handle = dlopen(filename, RTLD_LOCAL | RTLD_LAZY)))
		printf("Unable to dlload %s (error %s)\n", filename, dlerror());
	return handle;
#elif defined(_WINN32)
	return (Handle)LoadLibrary(filename);
#else
	#error "Unsupported target"
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SharedObject_close(Handle handle)
{
#if defined(__APPLE__)
	dlclose(handle);
#elif defined(_WIN32)
	return (Handle)LoadLibrary((HMODULE)handle);
#else
	#error "Unsupported target"
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* SharedObject_getSym(Handle handle, const char* name)
{
#if defined(__APPLE__)
	return dlsym(handle, name);
#elif defined(_WIN32)
	return (void*)GetProcAddress(handle, name);
#else
	#error "Unsupported target"
#endif
}

}

