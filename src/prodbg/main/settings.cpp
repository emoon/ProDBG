#include "settings.h"
#include <core/math.h>
#include <jansson.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct Settings {
    PDGRect windowRect;
} Settings;

static Settings s_settings;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void initDefaultSettings(Settings* settings) {
    settings->windowRect.x = 0;
    settings->windowRect.y = 0;
    settings->windowRect.width = 800;
    settings->windowRect.height = 600;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Settings_getWindowRect(PDGRect* rect) {
    Settings* settings = &s_settings;
    *rect = settings->windowRect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Settings_setWindowRect(PDGRect* rect) {
    Settings* settings = &s_settings;
    settings->windowRect = *rect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Settings_load() {
    Settings* settings = &s_settings;
    const PDGRect* r = &settings->windowRect;

    json_error_t error;

    json_t* root = json_load_file("settings.json", 0, &error);

    if (!root) {
        initDefaultSettings(settings);
        return;
    }

    json_t* data = json_array_get(root, 0);
    json_unpack(data, "{s:i, s:i, s:i, s:i}", "x", &r->x, "y", &r->y, "width", &r->width, "height", &r->height);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Settings_save() {
    /*
       Settings* settings = &s_settings;
       const PDGRect* r = &settings->windowRect;

       json_t* a = json_array();
       json_t* windowRect = json_pack("{s:i, s:i, s:i, s:i}", "x", r->x, "y", r->y, "width", r->width, "height", r->height);

       json_array_append_new(a, windowRect);

       json_dump_file(a, "settings.json", 0);
     */
}

