#pragma once

#include <bgfx.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PosTexColorVertex
{
<<<<<<< HEAD
    float x, y;
    float u, v;
    uint32_t color;
=======
	float x, y;
	float u, v;
	uint32_t color;
>>>>>>> docking-system
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PosColorVertex
{
<<<<<<< HEAD
    float x, y;
    uint32_t color;
=======
	float x, y;
	uint32_t color;
>>>>>>> docking-system
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIRender_init();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIRender_allocPosTexColorTb(bgfx::TransientVertexBuffer* buffer, uint32_t count);
void UIRender_allocPosColorTb(bgfx::TransientVertexBuffer* buffer, uint32_t count);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIRender_posTexColor(bgfx::TransientVertexBuffer* vertexBuffer, uint32_t offset, uint32_t count, bgfx::TextureHandle texHandle);
<<<<<<< HEAD
void UIRender_posTexRColor(bgfx::TransientVertexBuffer* vertexBuffer, uint32_t offset, uint32_t count, bgfx::TextureHandle texHandle);
=======
>>>>>>> docking-system
void UIRender_posColor(bgfx::TransientVertexBuffer* vertexBuffer, uint32_t offset, uint32_t count);


