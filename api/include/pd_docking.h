#ifndef _PD_DOCKING_H_
#define _PD_DOCKING_H_ 

#include <stdint.h>

#define PD_DOCKING_API_VERSION "ProDBG Docking 1"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This is the API used for docking of views within ProDBG. It's possible to use this API to replace the
// existing one

typedef void* PDDockHandle;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDockingCursor
{
    PDDockingCursor_Default = 0,
    PDDockingCursor_SizeHorizontal,
    PDDockingCursor_SizeVertical,
} PDDockingCursor;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum PDDockingSplitDir {
	PDDockingSplitDir_Horizontal,
	PDDockingSplitDir_Vertical,
} PDDockingSplitDir;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDDockingCallbacks {
	void (*update_window_size)(void* user_data, int x, int y, int width, int height);
	void (*set_cursor_style)(void* user_data, PDDockingCursor cursor);
	void (*save_user_data)(void* item, void* user_data);
	void* (*load_user_data)(void* item);
} PDDockingCallbacks;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDDocking {
	const char* name;

	void* (*create_instance)(int x, int y, int width, int height);
	void (*destroy_instance)(void* instance);

	PDDockHandle (*get_handle_at)(int x, int y);

	void (*set_callbacks)(void* instance, PDDockingCallbacks* callbacks);

	void (*split)(void* instance, void* user_data, PDDockingSplitDir dir, PDDockHandle handle);
	void (*close_dock)(void* instance, PDDockHandle handle);

	void (*update_size)(void* instance, void* user_data, int width, int height);
	void (*set_mouse)(void* instance, void* user_data, int x, int y, uint8_t left_down);

	void (*save_state)(void* instance, const char* filename);
	void (*load_state)(void* instance, const char* filename);

	void (*update)(void* instance);

} PDDocking;

#endif
