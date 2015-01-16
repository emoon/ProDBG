#pragma once

#include <bgfx.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIRender_init();

void UIRender_allocPosTexColorTb(bgfx::TransientVertexBuffer* buffer, int count);

void UIRender_posTexColor(
	bgfx::TransientVertexBuffer* vertexBuffer,
	int offset,
	int count,
	bgfx::TextureHandle texHandle,
	float width, float height);