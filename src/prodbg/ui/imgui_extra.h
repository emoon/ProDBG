
namespace ImGui
{
	void FillRect(ImVec2 pos, ImVec2 size, unsigned int color);
	float GetTextWidth(const char* textStart, const char* textEnd);
	ImVec2 GetRelativeMousePos();
	bool IsFocusWindowKeyDown(int key, bool repeat);
    void GetWindowRect(ImGuiWindow* window, ImVec2* pos, ImVec2* size);
	void SetWindowRect(ImGuiWindow* window, const ImVec2 pos, const ImVec2 size);
	bool IsActiveWindow(ImGuiWindow* window);
	//struct ImGuiWindow* FindOrCreateWindow(const char* name, ImVec2 size, ImGuiWindowFlags flags);
}

