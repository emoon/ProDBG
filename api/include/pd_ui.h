#ifndef _PDUI_H_
#define _PDUI_H_

#include "pd_common.h"
#include "pd_keys.h"

#include <string.h>
#include <stdio.h>

#ifdef _cplusplus
extern "C" {
#endif

struct PDUIPainter;
struct PDRect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDVec2
{
    float x, y;
    PDVec2(float _x, float _y) : x(_x), y(_y) {}
} PDVec2;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct PDVec4
{
    float x, y, z, w;
    PDVec4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
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

enum PDStyleVar : int
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

enum PDWindowFlags : int
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

enum PDInputTextFlags : int
{
    // Default: 0
    PDInputTextFlags_CharsDecimal       = 1 << 0,   // Allow 0123456789.+-*/
    PDInputTextFlags_CharsHexadecimal   = 1 << 1,   // Allow 0123456789ABCDEFabcdef
    PDInputTextFlags_AutoSelectAll      = 1 << 2,   // Select entire text when first taking focus
    PDInputTextFlags_EnterReturnsTrue   = 1 << 3,   // Return 'true' when Enter is pressed (as opposed to when the value was modified)
    PDInputTextFlags_CallbackCompletion = 1 << 4,   // Call user function on pressing TAB (for completion handling)
    PDInputTextFlags_CallbackHistory    = 1 << 5,   // Call user function on pressing Up/Down arrows (for history handling)
    PDInputTextFlags_CallbackAlways     = 1 << 6    // Call user function every time
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PDInputTextCallbackData
{
    int eventKey;            // Key pressed (Up/Down/TAB)           // Read-only   
    char* buffer;            // Current text                        // Read-write (pointed data only)
    int bufferSize;          // Read-only
    bool bufferDirty;        // Set if you modify buffer directly   // Write
    PDInputTextFlags flags;  // What user passed to inputText()     // Read-only
    int cursorPos;           //                                     // Read-write
    int selectionStart;      //                                     // Read-write (== to selectionEnd when no selection)
    int selectionEnd;        //                                     // Read-write
    void* userData;          // What user passed to InputText()

    // Calling these functions loses selection.
    void deleteChars(int pos, int byteCount);
    void insertChars(int pos, const char* text, const char* textEnd = nullptr);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void PDInputTextCallbackData::deleteChars(int pos, int byteCount)
{
    char* dst = buffer + pos;
    const char* src = buffer + pos + byteCount;
    while (char c = *src++)
        *dst++ = c;
    *dst = '\0';

    bufferDirty = true;
    if (cursorPos + byteCount >= pos)
        cursorPos -= byteCount;
    else if (cursorPos >= pos)
        cursorPos = pos;
    selectionStart = selectionEnd = cursorPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline void PDInputTextCallbackData::insertChars(int pos, const char* text, const char* textEnd)
{
    const int textLen = int(strlen(buffer));
    if (!textEnd)
        textEnd = text + strlen(text);
    const int newTextLen = (int)(textEnd - text);

    if (newTextLen + textLen + 1 >= bufferSize)
        return;

    size_t upos = (size_t)pos;
    if (textLen != upos)
        memmove(buffer + upos + newTextLen, buffer + upos, textLen - upos);
    memcpy(buffer + upos, text, newTextLen * sizeof(char));
    buffer[textLen + newTextLen] = '\0';

    bufferDirty = true;
    if (cursorPos >= pos)
        cursorPos += (int)newTextLen;
    selectionStart = selectionEnd = cursorPos;
}

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

	// Window
	
	PDVec2 (*getWindowSize)();
	float (*getFontHeight)();
	float (*getFontWidth)();

    void (*beginChild)(const char* stringId, PDVec2 size, bool border, PDWindowFlags extraFlags);
    void (*endChild)();

    // Text

    void (*text)(const char* fmt, ...);
    void (*textColored)(PDVec4 col, const char* fmt, ...);
    void (*textWrapped)(const char* fmt, ...);
	bool (*scInputText)(const char* label, char* buf, int buf_size, float xSize, float ySize, int flags, void (*callback)(void*), void* userData);
    bool (*inputText)(const char* label, char* buf, int buf_size, int flags, void (*callback)(PDInputTextCallbackData*), void* userData);

    // Widgets

    int (*button)(const char* label);
    int (*buttonSmall)(const char* label);
    int (*buttonSize)(const char* label, int width, int height, int repeatWhenHeld);

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

    void (*pushStyleVarV)(PDStyleVar styleVar, PDVec2 value);
    void (*pushStyleVarF)(PDStyleVar styleVar, float value);
    void (*popStyleVar)(int count);
	
    // Rendering

	void (*fillRect)(PDRect rect, unsigned int color); 

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _cplusplus
}
#endif

#endif

