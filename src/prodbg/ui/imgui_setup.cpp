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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    unsigned char* fontData;
    int fWidth;
    int fHeight;
    int outBytes;

    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_Tab]        = PDKEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = PDKEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = PDKEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = PDKEY_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = PDKEY_DOWN;
    io.KeyMap[ImGuiKey_Home]       = PDKEY_HOME;
    io.KeyMap[ImGuiKey_End]        = PDKEY_END;
    io.KeyMap[ImGuiKey_Delete]     = PDKEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = PDKEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]      = PDKEY_ENTER;
    io.KeyMap[ImGuiKey_Escape]     = PDKEY_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = PDKEY_A;
    io.KeyMap[ImGuiKey_C]          = PDKEY_C;
    io.KeyMap[ImGuiKey_V]          = PDKEY_V;
    io.KeyMap[ImGuiKey_X]          = PDKEY_X;
    io.KeyMap[ImGuiKey_Y]          = PDKEY_Y;
    io.KeyMap[ImGuiKey_Z]          = PDKEY_Z;

    s_editor = 0;
    //s_editor = ScEditor_create(width, height);

    UIRender_init();

    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fontData, &fWidth, &fHeight, &outBytes);

    const bgfx::Memory* mem = bgfx::alloc((uint32_t)(fWidth * fHeight * outBytes));
    memcpy(mem->data, fontData, size_t(fWidth * fHeight  * outBytes));

    s_textureId = bgfx::createTexture2D((uint16_t)fWidth, (uint16_t)fHeight, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_NONE, mem);

    io.RenderDrawListsFn = imguiRender;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_updateSize(int width, int height)
{
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;

    if (s_editor)
        ScEditor_resize(s_editor, 0, 0, width, height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void IMGUI_preUpdate(float x, float y, int mouseLmb, int keyDown, int keyMod, float deltaTime)
{
    (void)keyDown;
    (void)keyMod;
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = !!mouseLmb;

    ImGui::NewFrame();

    if (s_editor)
        ScEditor_tick(s_editor);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setMouse(float x, float y, int mouseLmb)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
    io.MouseDown[0] = !!mouseLmb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_scrollMouse(const PDMouseWheelEvent& wheelEvent)
{
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    //const float unitScale = 1.0f; // 1 unit = scrolling about 5 lines of text
    //io.MouseWheel = deltaY; TODO: Might not be scaled right for ImGui

    if (s_editor)
        ScEditor_scrollMouse(s_editor, wheelEvent);
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

    if (s_editor)
        ScEditor_render(s_editor);
}



