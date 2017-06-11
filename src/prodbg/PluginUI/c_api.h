
#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct PURect;
struct PUWidget;
struct PUPushButton;
struct PUSlider;
struct PUPainter;

struct PURect {
    float x;
    float y;
    float width;
    float height;
};

struct PUWidget {
    void (*show)(void* priv_data);
    void* priv_data;
};

struct PUPushButton {
    void (*show)(void* priv_data);
    void (*connect_released)(void* object, void* user_data, void (*callback)(void* user_data));
    void (*set_text)(const char* text, void* priv_data);
    void (*set_flat)(bool flat, void* priv_data);
    void* priv_data;
};

struct PUSlider {
    void (*connect_value_changed)(void* object, void* user_data, void (*callback)(int value, void* user_data));
    void* priv_data;
};

struct PUPainter {
    void (*draw_line)(int x1, int y1, int x2, int y2, void* priv_data);
    void* priv_data;
};

typedef struct PU { 
    struct PUWidget* (*create_widget)(void* priv_data);
    struct PUPushButton* (*create_push_button)(void* priv_data);
    struct PUSlider* (*create_slider)(void* priv_data);
    struct PUPainter* (*create_painter)(void* priv_data);
    void* priv_data;
} PU;

#ifdef __cplusplus
}
#endif
