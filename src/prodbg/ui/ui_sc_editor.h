#pragma once

struct ScEditor;
struct PDMouseWheelEvent;

ScEditor* ScEditor_create(int width, int height);

void ScEditor_resize(ScEditor* editor, int width, int height);

void ScEditor_tick(ScEditor* editor);

void ScEditor_render(ScEditor* editor);

// TODO: Should hide this more from the API (i.e. a proper input system + ImGui integration)
void ScEditor_scrollMouse(ScEditor* editor, const PDMouseWheelEvent& wheelEvent);

