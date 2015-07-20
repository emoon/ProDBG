#pragma once

#include <stdbool.h>

struct Con;
struct json_t;

#define DOCKSYS_SUPPORTS_LOAD_SAVE 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum DockSysCursor
{
    DockSysCursor_Default = 0,
    DockSysCursor_SizeHorizontal,
    DockSysCursor_SizeVertical,
} DockSysCursor;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct DockSysCallbacks
{
	void (*updateWindowSize)(void *userData, int x, int y, int width, int height);
	void (*setCursorStyle)(DockSysCursor cursor);
	void (*saveUserData)(struct json_t* item, void* userData);
	void* (*loadUserData)(struct json_t* item);
} DockSysCallbacks;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_set_callbacks(DockSysCallbacks* callbacks);

void docksys_horizontal_split(struct Con *con, void *user_data);
void docksys_vertical_split(struct Con *con, void *user_data);

struct Con *docksys_create_workspace(const char *name);

struct Con *docksys_con_by_user_data(void* user_data);

void docksys_close_con(void* user_data);

bool docksys_is_hovering_border();

void docksys_create(int x, int y, int width, int height);
void docksys_set_mouse(void* user_data, int x, int y, bool leftDown); 
void docksys_update_size(int width, int height);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(DOCKSYS_SUPPORTS_LOAD_SAVE) 

void docksys_save_layout(const char* filename);
bool docksys_load_layout(const char* filename);

#endif


void docksys_update();
