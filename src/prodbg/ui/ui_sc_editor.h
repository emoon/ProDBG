#pragma once

struct ScEditor;

ScEditor* ScEditor_create(int width, int height);

void ScEditor_resize(ScEditor* editor, int width, int height);

void ScEditor_tick(ScEditor* editor);

void ScEditor_render(ScEditor* editor);

