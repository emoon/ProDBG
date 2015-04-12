#ifndef _PDUI_H_
#define _PDUI_H_

#include "pd_common.h"
#include "pd_keys.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
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

typedef struct PDVec4
{
    float x, y, z, w;
} PDVec4;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDRect
{
	float x, y;
	float width, height;
} PDRect;

// TODO: use uint32_t?

#define PD_COLOR_32(r, g, b, a) (((unsigned int)r << 24) | ((unsigned int)g << 16) | ((unsigned int)b << 8) | (a))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum PDStyleVar
{
    PDStyleVar_Invalid = 0,
    PDStyleVar_Alpha,             // float
    PDStyleVar_WindowPadding,     // PDVec2
    PDStyleVar_WindowRounding,    // float
    PDStyleVar_FramePadding,      // PDVec2
    PDStyleVar_FrameRounding,     // float
    PDStyleVar_ItemSpacing,       // PDVec2
    PDStyleVar_ItemInnerSpacing,  // PDVec2
    PDStyleVar_TreeNodeSpacing,   // float
    PDStyleVar_Count
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Abstract this more

enum PDWindowFlags
{
    // Default: 0
    PDWindowFlags_NoTitleBar          = 1 << 0,   // Disable title-bar
    PDWindowFlags_NoResize            = 1 << 1,   // Disable user resizing with the lower-right grip
    PDWindowFlags_NoMove              = 1 << 2,   // Disable user moving the window
    PDWindowFlags_NoScrollbar         = 1 << 3,   // Disable scroll bar (window can still scroll with mouse or programatically)
    PDWindowFlags_NoScrollWithMouse   = 1 << 4,   // Disable user scrolling with mouse wheel
    PDWindowFlags_NoCollapse          = 1 << 5,   // Disable user collapsing window by double-clicking on it
    PDWindowFlags_AlwaysAutoResize    = 1 << 6,   // Resize every window to its content every frame
    PDWindowFlags_ShowBorders         = 1 << 7,   // Show borders around windows and items
    PDWindowFlags_NoSavedSettings     = 1 << 8,   // Never load/save settings in .ini file

    // [Internal]
    PDWindowFlags_ChildWindow         = 1 << 9,   // For internal use by BeginChild()
    PDWindowFlags_ChildWindowAutoFitX = 1 << 10,  // For internal use by BeginChild()
    PDWindowFlags_ChildWindowAutoFitY = 1 << 11,  // For internal use by BeginChild()
    PDWindowFlags_ComboBox            = 1 << 12,  // For internal use by ComboBox()
    PDWindowFlags_Tooltip             = 1 << 13   // For internal use by BeginTooltip()
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: Abstract this more

enum PDInputTextFlags
{
    // Default: 0
    PDInputTextFlags_CharsDecimal        = 1 << 0,   // Allow 0123456789.+-*/
    PDInputTextFlags_CharsHexadecimal    = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    PDInputTextFlags_CharsUppercase      = 1 << 2,   // Turn a..z into A..Z
    PDInputTextFlags_CharsNoBlank        = 1 << 3,   // Filter out spaces, tabs
    PDInputTextFlags_AutoSelectAll       = 1 << 4,   // Select entire text when first taking mouse focus
    PDInputTextFlags_EnterReturnsTrue    = 1 << 5,   // Return 'true' when Enter is pressed (as opposed to when the value was modified)
    PDInputTextFlags_CallbackCompletion  = 1 << 6,   // Call user function on pressing TAB (for completion handling)
    PDInputTextFlags_CallbackHistory     = 1 << 7,   // Call user function on pressing Up/Down arrows (for history handling)
    PDInputTextFlags_CallbackAlways      = 1 << 8,   // Call user function every time
    PDInputTextFlags_CallbackCharFilter  = 1 << 9    // Call user function to filter character. Modify data->EventChar to replace/filter input, or return 1 to discard character.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDInputTextCallbackData
{
    int eventKey;            // Key pressed (Up/Down/TAB)           // Read-only   
    char* buffer;            // Current text                        // Read-write (pointed data only)
    int bufferSize;          // Read-only
    bool bufferDirty;        // Set if you modify buffer directly   // Write
    int flags;               // What user passed to inputText()     // Read-only   // PDInputTextFlags
    int cursorPos;           //                                     // Read-write
    int selectionStart;      //                                     // Read-write (== to selectionEnd when no selection)
    int selectionEnd;        //                                     // Read-write
    void* userData;          // What user passed to InputText()

	void (*deleteChars)(struct PDInputTextCallbackData* data, int pos, int byteCount);
	void (*insertChars)(struct PDInputTextCallbackData* data, int pos, const char* text, const char* textEnd);
} PDInputTextCallbackData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDSCInterface
{
	intptr_t (*sendCommand)(void* privData, unsigned int message, uintptr_t p0, intptr_t p1);
	void (*update)(void* privData);
	void (*draw)(void* privData);
	void* privateData;
} PDSCInterface;

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
    void (*setScrollHere)();
    void (*pushItemWidth)(float item_width);                     // width of items for the common item+label case. default to ~2/3 of windows width.
    void (*popItemWidth)();

	// Window
	
	PDVec2 (*getWindowSize)();
	PDVec2 (*getWindowPos)();
	float (*getFontHeight)();
	float (*getFontWidth)();

    void (*beginChild)(const char* stringId, PDVec2 size, bool border, int extraFlags /* PDWindowFlags */);
    void (*endChild)();

    // Text

    void (*text)(const char* fmt, ...);
    void (*textColored)(PDVec4 col, const char* fmt, ...);
    void (*textWrapped)(const char* fmt, ...);
    bool (*inputText)(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDInputTextCallbackData*), void* userData);

    // Scintilla Editor Widget
	
	PDSCInterface* (*scInputText)(const char* label, float xSize, float ySize, void (*callback)(void*), void* userData);

    // Widgets

    int (*checkbox)(const char* label, bool* v);

    int (*button)(const char* label);
    int (*buttonSmall)(const char* label);
    int (*buttonSize)(const char* label, int width, int height, int repeatWhenHeld);

    // Misc

	PDRect (*getCurrentClipRect)();

    // Mouse

	PDVec2 (*getMousePos)();
	PDVec2 (*getMouseScreenPos)();
	int (*isMouseClicked)(int button, int repeat);
	int (*isMouseDoubleClicked)(int button);
	int (*isMouseHoveringBox)(PDVec2 boxMin, PDVec2 boxMax);
    int (*isItemHovered)();

	// Keyboard
	
	int (*isKeyDown)(int key, int repeat);
	int (*getKeyModifier)();
    void (*setKeyboardFocusHere)(int offset);

    // Styles

    void (*pushStyleVarV)(int styleVar /* PDStyleVar */, PDVec2 value);
    void (*pushStyleVarF)(int styleVar /* PDStyleVar */, float value);
    void (*popStyleVar)(int count);
	
    // Rendering

	void (*fillRect)(PDRect rect, unsigned int color); 

	// Id

	void (*pushIdPtr)(void* id);
	void (*pushIdInt)(int id);
	void (*popId)();

	//test
    void* privateData;

} PDUI;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PDUI_collumns(uiFuncs, count, id, border) uiFuncs->columns(count, id, border)
#define PDUI_nextColumn(uiFuncs) uiFuncs->nextColumn()
#define PDUI_text(uiFuncs, format, ...) uiFuncs->text(format, __VA_ARGS__)
#define PDUI_textWrapped(uiFuncs, format, ...) uiFuncs->textWrapped(format, __VA_ARGS__)

#define PDUI_button(uiFuncs, label) uiFuncs->button(label)
#define PDUI_buttonSmall(uiFuncs, label) uiFuncs->buttonSmall(label)
#define PDUI_buttonSize(uiFuncs, label, w, h, repeat) uiFuncs->button(label, w, h, repeat)

#define PDUI_SCSendCommand(funcs, msg, p0, p1) funcs->sendCommand(funcs->privateData, msg, p0, p1)
#define PDUI_SCDraw(funcs) funcs->update(funcs->privateData)
#define PDUI_SCUpdate(funcs) funcs->draw(funcs->privateData)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif


