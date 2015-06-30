#pragma once

struct InputState;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IMGUI_setup(int width, int height);
void IMGUI_updateSize(int width, int height);
void IMGUI_preUpdate(float deltaTime);
void IMGUI_postUpdate();

void IMGUI_setKeyDown(int key, int modifier);
void IMGUI_setKeyUp(int key, int modifier);
void IMGUI_setMousePos(float x, float y);
void IMGUI_setScroll(float scroll);
void IMGUI_setMouseState(int state);

void IMGUI_addInputCharacter(unsigned short c);


