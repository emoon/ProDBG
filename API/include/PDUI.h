#ifndef _PDUI_H_
#define _PDUI_H_

#include "PDCommon.h"

#ifdef _cplusplus
extern "C" {
#endif

typedef void* PDUIHandle;
typedef void* PDUIListView;
typedef void* PDUICustomView;

struct PDUIPainter;
struct PDRect;

typedef void (*PDCustomDrawCallback)(void* userData, PDRect* rect, struct PDUIPainter* painter);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUI
{
	PDUIListView (*listview_create)(void* privateData, const char** name, int id);

	int (*listview_clear)(void* privateData, PDUIListView handle);
	int (*listview_item_add)(void* privateData, PDUIListView handle, const char** item);

	// Custom view

	PDUICustomView (*customview_create)(void* privateData, void* userData, PDCustomDrawCallback callback);

	void* privateData;

} PDUI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDRect
{
	int x, y, width, height;

} PDRect;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUIPainter
{
	void (*fillRect)(void* privateData, PDRect* rect, unsigned int color);
	void (*fontMetrics)(void* privateData, int* width, int* height); 
	void (*setPen)(void* privateData, unsigned int color);
	void (*drawText)(void* privateData, int x, int y, const char* text);

	void* privateData;

} PDUIPainter;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUICustomView_create(uiFuncs, userData, callback) uiFuncs->customview_create(uiFuncs->privateData, userData, callback)
#define PDUIListView_create(uiFuncs, names, id) uiFuncs->listview_create(uiFuncs->privateData, names, id)
#define PDUIListView_clear(uiFuncs, handle) uiFuncs->listview_clear(uiFuncs->privateData, handle)
#define PDUIListView_itemAdd(uiFuncs, handle, names) uiFuncs->listview_item_add(uiFuncs->privateData, handle, names)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUIPaint_fillRect(pf, rect, color) pf->fillRect(pf->privateData, rect, color)
#define PDUIPaint_setPen(pf, color) pf->setPen(pf->privateData, color)
#define PDUIPaint_drawText(pf, x, y, text) pf->drawText(pf->privateData, x, y, text)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _cplusplus
}
#endif

#endif

