#pragma once

#include <pd_docking.h>
#include <stdint.h>

struct Con;
struct json_t;
typedef void* DockHandle;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_set_callbacks(void* instance, PDDockingCallbacks* callbacks);

//void docksys_horizontal_split(void* instance, void* user_data, PDDockHandle handle);
//void docksys_vertical_split(void* instance, void* user_data, PDDockHandle handle);

//struct Con* docksys_create_workspace(const char *name);
//struct Con* docksys_con_by_user_data(void* user_data);

void docksys_close_con(void* instance, void* user_data);

//int docksys_is_hovering_border();

void docksys_create(int x, int y, int width, int height);
void docksys_set_mouse(void* instance, void* user_data, int x, int y, uint8_t left_down); 
void docksys_update_size(void* instance, int width, int height);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_save_layout(void* instance, const char* filename);
int docksys_load_layout(void* instance, const char* filename);

void docksys_update(void* instance);

