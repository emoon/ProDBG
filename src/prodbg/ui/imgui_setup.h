#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setup(int width, int height);
void IMGUI_updateSize(int width, int height);
void IMGUI_preUpdate(float x, float y, int mouseLmb, int keyDown, int keyMod, float deltaTime);
void IMGUI_postUpdate();

void IMGUI_setMouse(float x, float y, int mouseLmb);
void IMGUI_setKeyDown(int key, int modifier);
void IMGUI_setKeyUp(int key, int modifier);


