#pragma once

struct ScEditor;
struct PDMouseWheelEvent;
struct ImDrawList;
struct ImFont;
struct ImScEditor;

void ScEditor_setDrawList(ImDrawList* drawList);
void ScEditor_setFont(ImFont* drawList);

ScEditor* ScEditor_create(int width, int height);

void ScEditor_setPos(float x, float y);
void ScEditor_resize(ScEditor* editor, int x, int y, int width, int height);

void ScEditor_tick(ScEditor* editor);

void ScEditor_render(ScEditor* editor);

ImScEditor* ScEditor_getInterface(ScEditor* editor);

// TODO: Should hide this more from the API (i.e. a proper input system + ImGui integration)
void ScEditor_scrollMouse(ScEditor* editor, const PDMouseWheelEvent& wheelEvent);

