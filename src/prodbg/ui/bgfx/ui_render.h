#pragma once

#include <bgfx.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PosTexColorVertex
{
    float x, y;
    float u, v;
    uint32_t color;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct PosColorVertex
{
    float x, y;
    uint32_t color;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UIRender_init();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIRender_allocPosTexColorTb(bgfx::TransientVertexBuffer* buffer, uint32_t count);
void UIRender_allocPosColorTb(bgfx::TransientVertexBuffer* buffer, uint32_t count);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UIRender_posTexColor(bgfx::TransientVertexBuffer* vertexBuffer, uint32_t offset, uint32_t count, bgfx::TextureHandle texHandle);
void UIRender_posIdxTexColor(bgfx::TransientVertexBuffer* vertexBuffer, bgfx::TransientIndexBuffer* indexBuffer, uint32_t vtxSize, uint32_t offset, uint32_t count, bgfx::TextureHandle texHandle);
void UIRender_posTexRColor(bgfx::TransientVertexBuffer* vertexBuffer, uint32_t offset, uint32_t count, bgfx::TextureHandle texHandle);
void UIRender_posColor(bgfx::TransientVertexBuffer* vertexBuffer, uint32_t offset, uint32_t count);


