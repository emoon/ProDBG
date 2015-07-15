#include "callbacks.h"

DockSysCallbacks* g_callbacks = 0;

void docksys_set_callbacks(DockSysCallbacks* callbacks)
{
	g_callbacks = callbacks;
}

