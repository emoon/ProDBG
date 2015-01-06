
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

float GetTextWidth(const char* textStart, const char* textEnd)
{
	ImVec2 size = CalcTextSize(textStart, textEnd);
	return size.x; 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ImVec2 GetRelativeMousePos()
{
    ImGuiState& g = GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    ImVec2 pos = g.IO.MousePos - window->Pos;
    ImVec2 zero = ImVec2(0.0f, 0.0f);
    return ImClamp(pos, zero, window->Size);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsFocusWindowKeyDown(int key, bool repeat)
{
	if (!GetWindowIsFocused())
		return false;

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
    ImGuiState& g = GImGui;
    return g.FocusedWindow == window;
}

}

