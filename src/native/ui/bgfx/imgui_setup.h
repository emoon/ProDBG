#pragma once

extern "C"
{

void imgui_setup(int width, int height);
void imgui_update_size(int width, int height);
void imgui_pre_update(float deltaTime);
void imgui_post_update();

void imgui_set_key_down(int key, int modifier);
void imgui_set_key_up(int key, int modifier);
void imgui_set_mouse_pos(float x, float y);
void imgui_set_scroll(float scroll);
void imgui_set_mouse_state(int index, int state);
void imgui_add_input_character(unsigned short c);

}

