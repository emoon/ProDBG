#include <imgui.h>
#include <stdio.h>
#include <ctype.h>
#include "imgui_setup.h"
#include "ui_sc_editor.h"
#include "stb_image.h"
#include "core/core.h"
#include "core/file.h"
#include "core/log.h"
#include "ui_render.h"
#include "core/input_state.h"
#include <stdio.h>
#include <pd_keys.h>
#include <bgfx.h>
#include <bx/fpumath.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bgfx::TextureHandle s_textureId;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void imguiRender(ImDrawData* draw_data) {
    const float width = ImGui::GetIO().DisplaySize.x;
    const float height = ImGui::GetIO().DisplaySize.y;

    float ortho[16];
    bx::mtxOrtho(ortho, 0.0f, width, height, 0.0f, -1.0f, 1.0f);

    bgfx::setViewTransform(0, NULL, ortho);

    // Render command lists
    for (int32_t ii = 0; ii < draw_data->CmdListsCount; ++ii) {
        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;

        const ImDrawList* cmd_list = draw_data->CmdLists[ii];
        uint32_t vtx_size = (uint32_t)cmd_list->VtxBuffer.size();
        uint32_t idx_size = (uint32_t)cmd_list->IdxBuffer.size();

        UIRender_allocPosTexColorTb(&tvb, (uint32_t)vtx_size);
        bgfx::allocTransientIndexBuffer(&tib, idx_size);

        ImDrawVert* verts = (ImDrawVert*)tvb.data;
        memcpy(verts, cmd_list->VtxBuffer.begin(), vtx_size * sizeof(ImDrawVert) );

        ImDrawIdx* indices = (ImDrawIdx*)tib.data;
        memcpy(indices, cmd_list->IdxBuffer.begin(), idx_size * sizeof(ImDrawIdx) );

        uint32_t elem_offset = 0;
        const ImDrawCmd* pcmd_begin = cmd_list->CmdBuffer.begin();
        const ImDrawCmd* pcmd_end = cmd_list->CmdBuffer.end();

        for (const ImDrawCmd* pcmd = pcmd_begin; pcmd != pcmd_end; pcmd++) {
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
                elem_offset += pcmd->ElemCount;
                continue;
            }

            if (0 == pcmd->ElemCount)
                continue;

            bgfx::setState(0
                           | BGFX_STATE_RGB_WRITE
                           | BGFX_STATE_ALPHA_WRITE
                           | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
                           | BGFX_STATE_MSAA
                           );
            bgfx::setScissor(uint16_t(bx::fmax(pcmd->ClipRect.x, 0.0f) )
                             , uint16_t(bx::fmax(pcmd->ClipRect.y, 0.0f) )
                             , uint16_t(bx::fmin(pcmd->ClipRect.z, 65535.0f) - bx::fmax(pcmd->ClipRect.x, 0.0f) )
                             , uint16_t(bx::fmin(pcmd->ClipRect.w, 65535.0f) - bx::fmax(pcmd->ClipRect.y, 0.0f) )
                             );
            union { void* ptr; bgfx::TextureHandle handle; } texture = { pcmd->TextureId };

            UIRender_posIdxTexColor(&tvb, &tib, vtx_size, elem_offset, pcmd->ElemCount, 0 != texture.handle.idx ? texture.handle : s_textureId);

            elem_offset += pcmd->ElemCount;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setup(int width, int height) {
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

    // TODO: Add this as config?
    // Update the style

    ImGuiStyle& style = ImGui::GetStyle();

    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.50f, 0.50f, 0.50f, 0.45f);
    style.Colors[ImGuiCol_CloseButton] = ImVec4(0.60f, 0.60f, 0.60f, 0.50f);
    style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.60f);
    style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_TitleBg];
    style.WindowPadding = ImVec2(4, 0);

    style.WindowRounding = 0.0f;

    UIRender_init();

    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fontData, &fWidth, &fHeight, &outBytes);

    const bgfx::Memory* mem = bgfx::alloc((uint32_t)(fWidth * fHeight * outBytes));
    memcpy(mem->data, fontData, size_t(fWidth * fHeight  * outBytes));

    s_textureId = bgfx::createTexture2D((uint16_t)fWidth, (uint16_t)fHeight, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_NONE, mem);

    io.RenderDrawListsFn = imguiRender;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_updateSize(int width, int height) {
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = 1.0f / 60.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_preUpdate(float deltaTime) {
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = deltaTime;

    ImGui::NewFrame();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setMouseState(int mouseLmb) {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[0] = !!mouseLmb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setMousePos(float x, float y) {
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(x, y);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setScroll(float scroll) {
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = scroll;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   void IMGUI_scrollMouse(const PDMouseWheelEvent& wheelEvent)
   {
   }
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool isAscii(int ch) {
    return (ch >= 0) && (ch < 0x80);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool isAlphaNumeric(char ch) {
    return isAscii(ch) && isalnum(ch);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setKeyDown(int key, int modifier) {
    ImGuiIO& io = ImGui::GetIO();
    assert(key >= 0 && key <= (int)sizeof_array(io.KeysDown));
    io.KeysDown[key] = true;
    io.KeyCtrl = !!(modifier & PDKEY_CTRL);
    io.KeyShift = !!(modifier & PDKEY_SHIFT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setKeyUp(int key, int modifier) {
    ImGuiIO& io = ImGui::GetIO();
    assert(key >= 0 && key <= (int)sizeof_array(io.KeysDown));
    io.KeysDown[key] = false;
    io.KeyCtrl = !!(modifier & PDKEY_CTRL);
    io.KeyShift = !!(modifier & PDKEY_SHIFT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_addInputCharacter(unsigned short c) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(c);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_postUpdate() {
    ImGui::Render();
}


