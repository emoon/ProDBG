#include "../cursor.h"
//#include <foundation/apple.h>
#import <Cocoa/Cocoa.h>

static NSCursor* s_cursors[CursorType_Count];
static CursorType s_lastCursor = CursorType_Default;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void cursor_init() {
    s_cursors[CursorType_Default] = [[NSCursor arrowCursor] retain];
    s_cursors[CursorType_SizeHorizontal] = [[NSCursor resizeUpDownCursor] retain];
    s_cursors[CursorType_SizeVertical] = [[NSCursor resizeLeftRightCursor] retain];
    s_cursors[CursorType_SizeAll] = [[NSCursor closedHandCursor] retain];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" void cursor_set_type(enum CursorType type) {
    if (type == s_lastCursor)
        return;

    s_lastCursor = type;

    NSCursor* cursor = s_cursors[(size_t)type];

    assert(cursor);

    [cursor set];
}
