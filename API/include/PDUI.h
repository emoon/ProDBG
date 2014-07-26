#ifndef _PDUI_H_
#define _PDUI_H_

#include "PDCommon.h"

#ifdef _cplusplus
extern "C" {
#endif

typedef void* PDUIHandle;
typedef void* PDUIListView;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUI
{
	PDUIListView (*listview_create)(void* privateData, const char* name, int id);

	int (*listview_clear)(void* privateData, PDUIListView handle);
	int (*listview_item_add)(void* privateData, PDUIListView handle, const char* item);
	int (*listview_item_remove)(void* privateData, PDUIListView handle, int index);
	int (*listview_item_text_get)(void* privateData, PDUIListView handle, int index);

	void* privateData;

} PDUI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUIListView_create(uiFuncs, name, id) uiFuncs->listview_create(uiFuncs->privateData, name, id)
#define PDUIListView_itemAdd(uiFuncs, handle, name) uiFuncs->listview_item_add(uiFuncs->privateData, handle, name)

#ifdef _cplusplus
}
#endif

#endif

