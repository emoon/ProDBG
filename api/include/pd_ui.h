#ifndef _PDUI_H_
#define _PDUI_H_

#include "pd_common.h"
#include "pd_keys.h"

#ifdef _cplusplus
extern "C" {
#endif

struct PDUIPainter;
struct PDRect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDVec2
{
	float x, y;
} PDVec2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDRect
{
	float x, y;
	float width, height;
} PDRect;

// TODO: use uint32_t?

#define PD_COLOR_32(r, g, b, a) (((unsigned int)r << 24) | ((unsigned int)g << 16) | ((unsigned int)b << 8) | (a))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDUI
{
    // Layout
	void (*separator)();                                         // horizontal line
	void (*sameLine)(int columnX, int spacingW);                 // call between widgets to layout them horizontally
	void (*spacing)();
    void (*columns)(int count, const char* id, int border);
    void (*nextColumn)();
	float (*getColumnOffset)(int column_index);
	void (*setColumnOffset)(int column_index, float offset);
	float (*getColumnWidth)(int column_index);
	PDVec2 (*getCursorPos)();                                    // cursor position relative to window position
	void (*setCursorPos)(PDVec2 pos);                            // "
	void (*setCursorPosX)(float x);                              // "
	void (*setCursorPosY)(float y);                              // "
	PDVec2 (*getCursorScreenPos)();                              // cursor position in screen space
	void (*alignFirstTextHeightToWidgets)();                     // call once if the first item on the line is a Text() item and you want to vertically lower it to match subsequent (bigger) widgets.
	float (*getTextLineSpacing)();
	float (*getTextLineHeight)();
	float (*getTextWidth)(const char* text, const char* textEnd);

	// Window
	
	PDVec2 (*getWindowSize)();
	float (*getFontHeight)();

    // Text

    void (*text)(const char* fmt, ...);

    // Widgets

    int (*button)(const char* label);
    int (*buttonSize)(const char* label, int width, int height, int repeatWhenHeld);

    // Mouse

	PDVec2 (*getMousePos)();
	PDVec2 (*getMouseScreenPos)();
	int (*isMouseClicked)(int button, int repeat);
	int (*isMouseDoubleClicked)(int button);
	int (*isMouseHoveringBox)(PDVec2 boxMin, PDVec2 boxMax);

	// Keyboard
	
	int (*getKeyDown)(int* mod);
	
    // Rendering

	void (*fillRect)(PDRect rect, unsigned int color); 

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

