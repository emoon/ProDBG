#include <stdint.h>

struct ImGuiWindow;

struct ImScEditor {
    intptr_t SendCommand(unsigned int message, uintptr_t p0, intptr_t p1);
    void ScrollTo(int line, bool moveThumb = true);
    void Update();
    void Draw();
    void HandleInput();
    void* userData;
    void* privateData;
};

namespace ImGui {

void FillRect(ImVec2 pos, ImVec2 size, unsigned int color);
void ConvexPolyFilled(void* vertices, int count, unsigned int color, bool aa);
void CircleFilled(ImVec2 pos, float radius, unsigned int color, int segment_count, bool aa);
float GetTextWidth(const char* textStart, const char* textEnd);

bool IsFocusWindowKeyDown(int key, bool repeat);
bool IsActiveWindow(ImGuiWindow* window);

ImVec2 GetRelativeMousePos();
ImScEditor* ScInputText(const char* label, float xSize, float ySize, void (* callback)(void*), void* userData);

}

