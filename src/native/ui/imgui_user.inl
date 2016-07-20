
#include "imgui_sc_editor.h"
#include <Scintilla.h>

namespace ImGui
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FillRect(ImVec2 pos, ImVec2 size, unsigned int color)
{
	ImGuiWindow* window = GetCurrentWindow();
	ImVec2 currentPos = window->Pos + pos;
    window->DrawList->AddRectFilled(currentPos, currentPos + size, color, 0.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConvexPolyFilled(void* vertices, int count, unsigned int color, bool aa)
{
	ImVec2* verts = (ImVec2*)vertices;
	ImVec2 temp_verts[4096];

	ImGuiWindow* window = GetCurrentWindow();
	ImVec2 current_pos = window->Pos;

	assert(count < 4096);

	// offset the verts temporary here so we don't need to mutate the data
	for (int i = 0; i < count; ++i) {
		temp_verts[i] = verts[i] + current_pos;
	}

    window->DrawList->AddConvexPolyFilled(temp_verts, count, color, aa);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CircleFilled(ImVec2 pos, float radius, unsigned int color, int segment_count, bool aa)
{
	(void)aa;
	ImGuiWindow* window = GetCurrentWindow();
	ImVec2 current_pos = window->Pos + pos;
    window->DrawList->AddCircleFilled(current_pos, radius, color, segment_count);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float GetTextWidth(const char* textStart, const char* textEnd)
{
	ImVec2 size = CalcTextSize(textStart, textEnd);
	return size.x;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImVec2 GetRelativeMousePos()
{
    const ImGuiState* g = GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    ImVec2 pos = g->IO.MousePos - window->Pos;
    ImVec2 zero = ImVec2(0.0f, 0.0f);
    return ImClamp(pos, zero, window->Size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsFocusWindowKeyDown(int key, bool repeat)
{
	if (!IsWindowFocused())
		return false;

	//if (!GetWindowIsFocused())
	//	return false;

    //ImGuiState& g = GImGui;
    //ImGuiWindow* window = GetCurrentWindow();

    // Only send keyboard events to selected window

    //if (g.FocusedWindow != window)
     //   return false;

    return IsKeyPressed(key, repeat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GetWindowRect(ImGuiWindow* window, ImVec2* pos, ImVec2* size)
{
    *pos = window->Pos;
    *size = window->Size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SetWindowRect(ImGuiWindow* window, const ImVec2 pos, const ImVec2 size)
{
    window->PosFloat = pos;
    window->Pos = ImVec2((float)(int)window->PosFloat.x, (float)(int)window->PosFloat.y);
    window->Size = size;
    window->SizeFull = size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsActiveWindow(ImGuiWindow* window)
{
    const ImGuiState* g = GImGui;
    return g->FocusedWindow == window;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImScEditor* ScInputText(const char* label, float xSize, float ySize, void (*callback)(void*), void* userData)
{
    const ImGuiStyle& style = ImGui::GetStyle();

    ImGuiWindow* window = GetCurrentWindow();
    const ImGuiID id = window->GetID(label);

    (void)callback;
    (void)userData;

	ImFont* font = GetWindowFont();

    ScEditor_setFont(font);
    ScEditor_setDrawList(GetWindowDrawList());

	ImGuiStorage* storage = GetStateStorage();
	ScEditor* editor = (ScEditor*)storage->GetVoidPtr(id);

	if (!editor)
	{
		(void)xSize;
		(void)ySize;
		editor = ScEditor_create((int)xSize, (int)ySize);
		storage->SetVoidPtr(id, (void*)editor);
	}

	float title_height = window->TitleBarHeight();

	title_height -= (int)font->Ascent;

	ImScEditor* editorInterface = ScEditor_getInterface(editor);

	float textSize = ImGui::GetTextLineHeight();

	ScEditor_resize(editor, 0, (int)font->Ascent, (int)window->Size.x - (int)style.ScrollbarSize, (int)(window->Size.y - (int)title_height));

	int lineCount = (int)editorInterface->SendCommand(SCI_GETLINECOUNT, 0, 0);

	editorInterface->HandleInput();

	ScEditor_setPos(window->PosFloat.x, window->PosFloat.y + title_height);

	ImGuiListClipper clipper(lineCount, textSize);
	// TODO: Might not be the fully correct way to do this but works better than without it
	// This is here because we don't do any really rendering to imgui (scintilla adds text directly using
	// DrawList->AddText(...) so we need to advance the cursor ourselfs
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + clipper.ItemsCount * clipper.ItemsHeight);

	editorInterface->ScrollTo(clipper.DisplayStart);

	clipper.End();

	return editorInterface;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}

