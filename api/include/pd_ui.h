#ifndef _PDUI_H_
#define _PDUI_H_

#include "pd_common.h"

#ifdef _cplusplus
extern "C" {
#endif

struct PDUIPainter;
struct PDRect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUI
{
    // Layout

    void (*columns)(int count, const char* id, int border);
    void (*nextColumn)();

    // Text

    // void (*text)(const char* fmt, ...);

    // Widgets

    int (*button)(const char* label);
    int (*buttonSize)(const char* label, int width, int height, int repeatWhenHeld);

    void* privateData;

} PDUI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUI_collumns(uiFuncs, count, id, border) uiFuncs->columns(count, id, border)
#define PDUI_nextColumn(uiFuncs) uiFuncs->nextColumn()
#define PDUI_text(uiFuncs, format, ...) uiFuncs->text(format, __VA_ARGS__)

#define PDUI_button(uiFuncs, label) uiFuncs->button(label)
#define PDUI_buttonSize(uiFuncs, label, w, h, repeat) uiFuncs->button(label, w, h, repeat)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _cplusplus
}
#endif

#endif

