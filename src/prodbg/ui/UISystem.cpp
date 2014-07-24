#include "UISystem.h"
#include <core/io/SharedObject.h>
#include <core/Log.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace prodbg
{

static ARFuncs* s_arFuncs;
static Handle s_handle;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UISystem_initArika(const char* basePath, const char* backend)
{
	char baseRedirect[4096];
	void* (*initFuncs)();

	// This is a hack that we always load ../Arika/.. path during development as both systems will
	// be changing and I still want to keep them close by

	sprintf(baseRedirect, "../Arika/%s", basePath);

	Handle handle = SharedObject_open(baseRedirect, backend);

	if (!handle)
	{
		log_error("Unable to load arika backend: %s - %s\n", basePath, backend);
		return false;
	}

	*(void**)(&initFuncs) = SharedObject_getSym(handle, "ar_init_funcs");

	if (!initFuncs)
	{
		log_error("Unable to find ar_init_funcs in Arika backend: %s - %s\n", basePath, backend);
		SharedObject_close(handle);
		return false;
	}

	s_arFuncs = (ARFuncs*)initFuncs();
	s_handle = handle;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ARFuncs* UISystem_getArFuncs()
{
	return s_arFuncs;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}


