#pragma once

enum CursorType {
    CursorType_Default,
    CursorType_SizeHorizontal,
    CursorType_SizeVertical,
    CursorType_SizeAll,
    CursorType_Count,
};

void cursor_init();
void cunsor_set_type(enum CursorType type);
