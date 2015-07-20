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

static const char* getType(int type)
{
	switch (type)
	{
		case CT_ROOT : return "root"; break;
		case CT_OUTPUT : return "output"; break;
		case CT_CON : return "con"; break;
		case CT_FLOATING_CON : return "floating_con"; break;
		case CT_WORKSPACE : return "workspace"; break;
		case CT_DOCKAREA : return "dockarea"; break;
		default : return "Unknown"; break;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* getLayout(int type)
{
	switch (type)
	{
		case L_DEFAULT  : return "default"; break;
		case L_STACKED  : return "stacked"; break;
		case L_TABBED  : return "tabbed"; break;
		case L_DOCKAREA   : return "dockarea"; break;
		case L_OUTPUT  : return "output"; break;
		case L_SPLITV  : return "splitv"; break;
		case L_SPLITH  : return "splith"; break;
		default : return "unknown"; break;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printRect(const char* name, I3Rect rect, int level)
{
	printf("%*s%s - %d %d - %d %d\n", level, "", name, rect.x, rect.y, rect.width, rect.height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printVal(const char* name, int val, int level)
{
	printf("%*s%s - %d\n", level, "", name, val);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printPointer(const char* name, void* val, int level)
{
	printf("%*s%s - %p\n", level, "", name, val);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printString(const char* name, const char* val, int level)
{
	printf("%*s%s - %s\n", level, "", name, val);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printDouble(const char* name, double val, int level)
{
	printf("%*s%s - %f\n", level, "", name, val);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define p_val(v) printVal(#v, con->v, level)
#define p_string(v) printString(#v, con->v, level)
#define p_rect(v) printRect(#v, con->v, level)
#define p_pointer(v) printPointer(#v, con->v, level)
#define p_double(v) printDouble(#v, con->v, level)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void printTree(Con* con, int level)
{
	Con* child = 0;

	printf("%*s--------------------------------------------------\n", level, "");
	p_string(name);
	p_val(mapped);
	p_val(urgent);
	p_val(ignore_unmap);
	p_val(frame);
	printString("type", getType(con->type), level);
	p_val(num);
	p_rect(rect);
	p_rect(window_rect);
	p_rect(geometry);
	p_pointer(parent);
	p_string(sticky_group);
	p_string(mark);
	p_val(mark_changed);
	p_double(percent);
	p_double(aspect_ratio);
	p_val(base_width);
	p_val(base_height);
	p_val(border_width);
	p_val(current_border_width);
	p_val(width_increment);
	p_val(height_increment);
	p_pointer(window);
	p_val(fullscreen_mode);

	printString("layout", getLayout(con->layout), level);
	printString("last_split_layout", getLayout(con->last_split_layout), level);
	printString("workspace_layout", getLayout(con->workspace_layout), level);
	p_val(border_style);
	p_val(scratchpad_state);
	p_val(old_id);
	p_val(depth);

    TAILQ_FOREACH(child, &(con->nodes_head), nodes) 
        printTree(child, level + 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void saveTree(Con* con, json_t* item, json_t* parentArray)
{
	Con* child = 0;

	json_object_set_new(item, "name", json_string(con->name ? con->name : "(null)"));
	json_object_set_new(item, "type", json_string(getType(con->type)));
	json_object_set_new(item, "layout", json_string(getLayout(con->layout)));
	json_object_set_new(item, "percent", json_real(con->percent));

	// Allows user to add data to the node

	if (con->window && g_callbacks && con->window->userData && g_callbacks->saveUserData)
		g_callbacks->saveUserData(item, con->window->userData);

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
	//printTree(croot, 0);

	Con* con = workspace_get("1", NULL);

	printTree(con, 0);

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

#if 0

static void loadData(json_t* item, Con* parent)
{
	json_t* name = json_object_get(item, "name");
	json_t* type = json_object_get(item, "type");
	json_t* layout = json_object_get(item, "layout");
	json_t* percent = json_object_get(iterm, "percent");

	json_t* hasUserData = json_object_get(iterm, "userdata");


//	Con* con = con_new_skeleton(parent,

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void docksys_load_layout(const char* filename)
{
    json_error_t error;

    json_t* root = json_load_file(filename, 0, &error);

    if (!root)
    {
        printf("JSON Error: %s:(%d:%d) - %s\n", filename, error.line, error.column, error.text);
        return;
    }
}

#endif

#endif




