#pragma once

enum CursorType {
    CursorType_Default,
    CursorType_SizeHorizontal,
    CursorType_SizeVertical,
    CursorType_SizeAll,
    CursorType_Count,
};

extern "C" void cursor_init();
extern "C" void cunsor_set_type(enum CursorType type);
