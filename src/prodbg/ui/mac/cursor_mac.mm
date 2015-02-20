#include "../cursor.h"
#import <Cocoa/Cocoa.h>

static NSCursor* s_cursors[CursorType_Count];

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Cursor_init()
{
	s_cursors[CursorType_Default] = [[NSCursor arrowCursor] retain];
	s_cursors[CursorType_SizeHorizontal] = [[NSCursor resizeUpDownCursor] retain];
	s_cursors[CursorType_SizeVertical] = [[NSCursor resizeLeftRightCursor] retain];
	s_cursors[CursorType_SizeAll] = [[NSCursor closedHandCursor] retain];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Cunsor_setType(enum CursorType type)
{
	NSCursor* cursor = s_cursors[(size_t)type];

	assert(cursor);

	[cursor set];
}
