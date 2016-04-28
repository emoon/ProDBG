#include "../cursor.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <assert.h>

static HCURSOR s_cursors[CursorType_Count];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void cursor_init() {
    s_cursors[CursorType_Default] = LoadCursor(NULL, IDC_ARROW);
    s_cursors[CursorType_SizeHorizontal] = LoadCursor(NULL, IDC_SIZENS);
    s_cursors[CursorType_SizeVertical] = LoadCursor(NULL, IDC_SIZEWE);
    s_cursors[CursorType_SizeAll] = LoadCursor(NULL, IDC_SIZEALL);

    for (int i = 0; i < CursorType_Count; ++i)
        assert(s_cursors[i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void cursor_set_type(enum CursorType type) {
    SetCursor(s_cursors[(int)type]);
}
