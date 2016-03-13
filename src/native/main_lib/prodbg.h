#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" {

void prodbg_create(void* window, int width, int height);
void prodbg_set_window_size(int width, int height);

void prodbg_application_launched();
void prodbg_destroy();
void prodbg_update();
void prodbg_timed_update();

void prodbg_add_char(unsigned short c);
void prodbg_key_down(int key, int modifier);
void prodbg_key_up(int key, int modifier);
void prodbg_event(int eventId);
void prodbg_set_mouse_pos(float x, float y);
void prodbg_set_mouse_state(int button, int state);
void prodbg_set_scroll(float x, float y);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}
