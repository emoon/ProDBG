#pragma once

enum CursorType {
    CursorType_Default,
    CursorType_SizeHorizontal,
    CursorType_SizeVertical,
    CursorType_SizeAll,
    CursorType_Count,
};

void Cursor_init();
void Cunsor_setType(enum CursorType type);
