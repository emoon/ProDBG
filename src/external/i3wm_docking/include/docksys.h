#pragma once

#include <stdbool.h>

struct Con;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DockSysCallbacks
{
	void (*updateWindowSize)(void *userData, int x, int y, int width, int height);
	void (*setCursorStyle)(int style);
} DockSysCallbacks;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_set_callbacks(DockSysCallbacks* callbacks);

void docksys_horizontal_split(struct Con *con, void *user_data);
void docksys_vertical_split(struct Con *con, void *user_data);

struct Con *docksys_create_workspace(const char *name);

struct Con *docksys_con_by_user_data(void* user_data);

void docksys_close_con(void* user_data);

void docksys_create(int x, int y, int width, int height);
void docksys_set_mouse(int x, int y, bool leftDown); 

void docksys_update();
