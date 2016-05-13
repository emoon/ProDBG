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
	PDMenuHandle (*create_menu)(const char* name);
	void (*destroy_menu)(PDMenuHandle handle);
	void (*add_sub_menu)(const char* name, PDMenuHandle parent, PDMenuHandle child);

	PDMenuItem (*add_menu_item)(
			PDMenuHandle menu, 
			const char* name, 
			uint32_t id,
			uint32_t key,
			uint32_t modifier);
	void (*remove_menu_item)(PDMenuItem handle);

} PDMenuFuncs;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus

}
#endif

#endif

