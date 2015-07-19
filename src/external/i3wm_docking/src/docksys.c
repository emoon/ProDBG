#include "docksys.h"
#include "data.h"
#include "con.h"
#include "tree.h"
#include "move.h"
#include "log.h"
#include "output.h"
#include "render.h"
#include "workspace.h"
#include <stddef.h>

#if defined(DOCKSYS_SUPPORTS_LOAD_SAVE)
#include <jansson.h>
#endif

DockSysCallbacks* g_callbacks = 0;

static int prev_mouse_x;
static int prev_mouse_y;

int handle_button_press(int x, int y, int mxd, int myd, bool lmb_down);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_set_callbacks(DockSysCallbacks* callbacks)
{
	g_callbacks = callbacks;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_horizontal_split(Con *con, void *user_data);
void docksys_verical_split(Con *con, void *user_data);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Con* docksys_create_workspace(const char *name)
{
	return workspace_get("test_ws", NULL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_create(int x, int y, int width, int height)
{
	tree_init((I3Rect) { 0, 0, (uint32_t)width, (uint32_t)height });
	fake_outputs_init(0, 0, (uint32_t)width, (uint32_t)height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_set_mouse(void *user_data, int x, int y, bool leftDown)
{
	int mxd = x - prev_mouse_x;
	int myd = y - prev_mouse_y;

	handle_button_press(x, y, mxd, myd, leftDown);

	prev_mouse_x = x;
	prev_mouse_y = y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_update()
{
	tree_render();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Con* docksys_con_by_user_data(void* user_data)
{
	return con_by_user_data(user_data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void split(Con *con, void* user_data, orientation_t orientation)
{
	if (con)
	{
		tree_split(con, orientation);
		tree_open_con(con->parent, user_data);
	}
	else
	{
		tree_open_con(NULL, user_data);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_horizontal_split(Con *con, void *user_data)
{
	split(con, user_data, HORIZ);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_vertical_split(Con *con, void *user_data)
{
	split(con, user_data, VERT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_close_con(void* user_data)
{
	con_focus(con_by_user_data(user_data));
	tree_close_con(DONT_KILL_WINDOW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_update_size(int width, int height)
{
	(void)width;
	(void)height;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(DOCKSYS_SUPPORTS_LOAD_SAVE)

void printTree(Con* con, int level)
{
	Con* child = 0;

	for (int i = 0; i < level; ++i)
		printf(" ");

	printf("%p - %s\n", con, con->name);

    TAILQ_FOREACH(child, &(con->nodes_head), nodes) 
        printTree(child, level + 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void saveTree(Con* con, json_t* item, json_t* parentArray)
{
	Con* child = 0;

	json_object_set_new(item, "name", json_string(con->name ? con->name : "(null)"));
	json_array_append_new(parentArray, item);

	if (con_num_children(con) == 0)
		return;

    json_t* children = json_array();
    json_object_set_new(item, "children", children);

    TAILQ_FOREACH(child, &(con->nodes_head), nodes) 
	{
		json_t* newItem = json_object();
		saveTree(child, newItem, children); 
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_save_layout(const char* filename)
{
	printTree(croot, 0);

	Con* con = workspace_get("1", NULL);

    json_t* root = json_object();
    json_t* children = json_array();

	saveTree(con, root, children); 

    //json_array_append_new(root, children);
    //json_object_set_new(root, "children", children);

    if (json_dump_file(root, filename, JSON_INDENT(4) | JSON_PRESERVE_ORDER) != 0)
    {
        printf("JSON: Unable to open %s for write\n", filename);
        return;
    }

	printf("Saved layout\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_load_layout(const char* filename)
{
	

}

#endif




