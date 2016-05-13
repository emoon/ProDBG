#ifndef _PRODBGAPI_MENU_SERVICE_H_
#define _PRODBGAPI_MENU_SERVICE_H_ 

#include "pd_keys.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PDMENUFUNCS_GLOBAL "Menu Service 1"

typedef uint64_t PDMenuHandle;
typedef uint64_t PDMenuItem;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum {
	PDMENU_FLAGS_ENABLED = 1 << 1,
	PDMENU_FLAGS_DISABLED = 1 << 2,
	PDMENU_FLAGS_SEPARATOR = 1 << 3,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDMenuFuncs {
	void* private_data;

	PDMenuHandle (*create_menu)(void* private_data, const char* name);
	void (*destroy_menu)(void* private_data, PDMenuHandle handle);
	void (*add_sub_menu)(void* private_data, const char* name, 
						 PDMenuHandle parent, PDMenuHandle child);

	PDMenuItem (*add_menu_item)(
			void* private_data,
			PDMenuHandle menu, 
			const char* name, 
			uint32_t id,
			uint32_t key,
			uint32_t modifier);

	void (*remove_menu_item)(void* private_data, PDMenuItem handle);

} PDMenuFuncs;

#define PDMenu_create_menu(funcs, name) funcs->create_menu(funcs->private_data, name) 
#define PDMenu_add_menu_item(funcs, handle, name, id, key, modifier) \
	funcs->add_menu_item(funcs->private_data, handle, name, id, key, modifier) 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

}
#endif

#endif

