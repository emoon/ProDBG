#pragma once

struct ScEditor;
struct PDMouseWheelEvent;
struct ImDrawList;
struct ImFont;

void ScEditor_setDrawList(ImDrawList* drawList);
void ScEditor_setFont(ImFont* drawList);

ScEditor* ScEditor_create(int width, int height);

void ScEditor_resize(ScEditor* editor, int width, int height);

void ScEditor_tick(ScEditor* editor);

void ScEditor_render(ScEditor* editor);

// TODO: Should hide this more from the API (i.e. a proper input system + ImGui integration)
void ScEditor_scrollMouse(ScEditor* editor, const PDMouseWheelEvent& wheelEvent);

