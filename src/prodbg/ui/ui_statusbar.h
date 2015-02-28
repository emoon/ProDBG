#pragma once

// TODO: Move to settings?
extern float g_statusBarSize;

#if defined(__clang__) || defined(__gcc__)
void UIStatusBar_setText(const char* format, ...) __attribute__((format(printf, 1, 2)));
#else
void UIStatusBar_setText(const char* format, ...);
#endif

void UIStatusBar_render();

