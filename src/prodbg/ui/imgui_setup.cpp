#include <imgui.h>
#include "imgui_setup.h"
#include "ui_sc_editor.h"
#include "stb_image.h"
#include "core/core.h"
#include "core/file.h"
#include "core/log.h"
#include "ui_render.h"
#include <stdio.h>
#include <pd_keys.h>
#include <bgfx.h>
#include <bx/fpumath.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bgfx::TextureHandle s_textureId;
static ScEditor* s_editor;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void imguiRender(ImDrawList** const cmd_lists, int cmd_lists_count)
{
    (void)cmd_lists;
    (void)cmd_lists_count;

    const float width = ImGui::GetIO().DisplaySize.x;
    const float height = ImGui::GetIO().DisplaySize.y;

    float ortho[16];
    bx::mtxOrtho(ortho, 0.0f, width, height, 0.0f, -1.0f, 1.0f);

    bgfx::setViewTransform(0, NULL, ortho);

    // Render command lists

    for (int n = 0; n < cmd_lists_count; n++)
    {
        bgfx::TransientVertexBuffer tvb;

        uint32_t vtx_size = 0;

        const ImDrawList* cmd_list = cmd_lists[n];
        const ImDrawVert* vtx_buffer = cmd_list->vtx_buffer.begin();
        (void)vtx_buffer;

        const ImDrawCmd* pcmd_end_t = cmd_list->commands.end();

        for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end_t; pcmd++)
            vtx_size += (uint32_t)pcmd->vtx_count;

        UIRender_allocPosTexColorTb(&tvb, (uint32_t)vtx_size);

        ImDrawVert* verts = (ImDrawVert*)tvb.data;

        memcpy(verts, vtx_buffer, vtx_size * sizeof(ImDrawVert));

        uint32_t vtx_offset = 0;
        const ImDrawCmd* pcmd_end = cmd_list->commands.end();
        for (const ImDrawCmd* pcmd = cmd_list->commands.begin(); pcmd != pcmd_end; pcmd++)
        {
            /*
               bgfx::setScissor((uint16_t)pcmd->clip_rect.x,
                             (uint16_t)(height - pcmd->clip_rect.w),
                             (uint16_t)(pcmd->clip_rect.z - pcmd->clip_rect.x),
                             (uint16_t)(pcmd->clip_rect.w - pcmd->clip_rect.y));
             */
            bgfx::setState(0
                           | BGFX_STATE_RGB_WRITE
                           | BGFX_STATE_ALPHA_WRITE
                           | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                           | BGFX_STATE_MSAA);

            UIRender_posTexColor(&tvb, vtx_offset, pcmd->vtx_count, s_textureId);

            vtx_offset += pcmd->vtx_count;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setup(int width, int height)
{
    const void* png_data;
    unsigned int png_size;
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;
    io.PixelCenterOffset = 0.0f;

	s_editor = ScEditor_create();

    UIRender_init();

    ImGui::GetDefaultFontData(NULL, NULL, &png_data, &png_size);
    int tex_x, tex_y, pitch, tex_comp;

    void* tex_data = stbi_load_from_memory((const unsigned char*)png_data, (int)png_size, &tex_x, &tex_y, &tex_comp, 0);

    pitch = tex_x * 4;

    const bgfx::Memory* mem = bgfx::alloc((uint32_t)(tex_y * pitch));
    memcpy(mem->data, tex_data, size_t(pitch * tex_y));

    s_textureId = bgfx::createTexture2D((uint16_t)tex_x, (uint16_t)tex_y, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_MIN_POINT | BGFX_TEXTURE_MAG_POINT, mem);

    stbi_image_free(tex_data);

    io.RenderDrawListsFn = imguiRender;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_updateSize(int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;
    io.PixelCenterOffset = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void IMGUI_preUpdate(float x, float y, int mouseLmb, int keyDown, int keyMod)
{
    (void)keyDown;
    (void)keyMod;
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = 1.0f / 120.0f;    // TODO: Fix me
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = !!mouseLmb;

    ImGui::NewFrame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setMouse(float x, float y, int mouseLmb)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = !!mouseLmb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setKeyDown(int key, int modifier)
{
    ImGuiIO& io = ImGui::GetIO();
    assert(key >= 0 && key <= (int)sizeof_array(io.KeysDown));
    io.KeysDown[key] = true;
    io.KeyCtrl = !!(modifier & PDKEY_CTRL);
    io.KeyShift = !!(modifier & PDKEY_SHIFT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setKeyUp(int key, int modifier)
{
    ImGuiIO& io = ImGui::GetIO();
    assert(key >= 0 && key <= (int)sizeof_array(io.KeysDown));
    io.KeysDown[key] = false;
    io.KeyCtrl = !!(modifier & PDKEY_CTRL);
    io.KeyShift = !!(modifier & PDKEY_SHIFT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_postUpdate()
{
    ImGui::Render();
}



