

// These are keycodes return by the getKeyPressed function in PDUI

/* Taken from glfw and renamed and slightly
 * These key codes are inspired by the *USB HID Usage Tables v1.12* (p. 53-60),
 * but re-arranged to map to 7-bit ASCII for printable keys (function keys are
 * put in the 256+ range).
 *
 * The naming of the key codes follow these rules:
 *  - The US keyboard layout is used
 *  - Names of printable alpha-numeric characters are used (e.g. "A", "R",
 *    "3", etc.)
 *  - For non-alphanumeric characters, Unicode:ish names are used (e.g.
 *    "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
 *    correspond to the Unicode standard (usually for brevity)
 *  - Keys that lack a clear US mapping are named "WORLD_x"
 *  - For non-printable keys, custom names are used (e.g. "F4",
 *    "BACKSPACE", etc.)
 */

#ifndef PD_KEYS_H_
#define PD_KEYS_H_

#define PDKEY_UNKNOWN            0
#define PDKEY_SPACE              32
#define PDKEY_APOSTROPHE         39
#define PDKEY_COMMA              44
#define PDKEY_MINUS              45
#define PDKEY_PERIOD             46
#define PDKEY_SLASH              47
#define PDKEY_0                  48
#define PDKEY_1                  49
#define PDKEY_2                  50
#define PDKEY_3                  51
#define PDKEY_4                  52
#define PDKEY_5                  53
#define PDKEY_6                  54
#define PDKEY_7                  55
#define PDKEY_8                  56
#define PDKEY_9                  57
#define PDKEY_SEMICOLON          59
#define PDKEY_EQUAL              61
#define PDKEY_A                  65
#define PDKEY_B                  66
#define PDKEY_C                  67
#define PDKEY_D                  68
#define PDKEY_E                  69
#define PDKEY_F                  70
#define PDKEY_G                  71
#define PDKEY_H                  72
#define PDKEY_I                  73
#define PDKEY_J                  74
#define PDKEY_K                  75
#define PDKEY_L                  76
#define PDKEY_M                  77
#define PDKEY_N                  78
#define PDKEY_O                  79
#define PDKEY_P                  80
#define PDKEY_Q                  81
#define PDKEY_R                  82
#define PDKEY_S                  83
#define PDKEY_T                  84
#define PDKEY_U                  85
#define PDKEY_V                  86
#define PDKEY_W                  87
#define PDKEY_X                  88
#define PDKEY_Y                  89
#define PDKEY_Z                  90
#define PDKEY_LEFT_BRACKET       91
#define PDKEY_BACKSLASH          92
#define PDKEY_RIGHT_BRACKET      93
#define PDKEY_GRAVE_ACCENT       96

// Function keys

#define PDKEY_ESCAPE             256
#define PDKEY_ENTER              257
#define PDKEY_TAB                258
#define PDKEY_BACKSPACE          259
#define PDKEY_INSERT             260
#define PDKEY_DELETE             261
#define PDKEY_RIGHT              262
#define PDKEY_LEFT               263
#define PDKEY_DOWN               264
#define PDKEY_UP                 265
#define PDKEY_PAGE_UP            266
#define PDKEY_PAGE_DOWN          267
#define PDKEY_HOME               268
#define PDKEY_END                269
#define PDKEY_CAPS_LOCK          280
#define PDKEY_SCROLL_LOCK        281
#define PDKEY_NUM_LOCK           282
#define PDKEY_PRINT_SCREEN       283
#define PDKEY_PAUSE              284
#define PDKEY_F1                 290
#define PDKEY_F2                 291
#define PDKEY_F3                 292
#define PDKEY_F4                 293
#define PDKEY_F5                 294
#define PDKEY_F6                 295
#define PDKEY_F7                 296
#define PDKEY_F8                 297
#define PDKEY_F9                 298
#define PDKEY_F10                299
#define PDKEY_F11                300
#define PDKEY_F12                301
#define PDKEY_F13                302
#define PDKEY_F14                303
#define PDKEY_F15                304
#define PDKEY_F16                305
#define PDKEY_F17                306
#define PDKEY_F18                307
#define PDKEY_F19                308
#define PDKEY_F20                309
#define PDKEY_F21                310
#define PDKEY_F22                311
#define PDKEY_F23                312
#define PDKEY_F24                313
#define PDKEY_F25                314
#define PDKEY_KP_0               320
#define PDKEY_KP_1               321
#define PDKEY_KP_2               322
#define PDKEY_KP_3               323
#define PDKEY_KP_4               324
#define PDKEY_KP_5               325
#define PDKEY_KP_6               326
#define PDKEY_KP_7               327
#define PDKEY_KP_8               328
#define PDKEY_KP_9               329
#define PDKEY_KP_DECIMAL         330
#define PDKEY_KP_DIVIDE          331
#define PDKEY_KP_MULTIPLY        332
#define PDKEY_KP_SUBTRACT        333
#define PDKEY_KP_ADD             334
#define PDKEY_KP_ENTER           335
#define PDKEY_KP_EQUAL           336
#define PDKEY_LEFT_SHIFT         340
#define PDKEY_LEFT_CONTROL       341
#define PDKEY_LEFT_ALT           342
#define PDKEY_LEFT_SUPER         343
#define PDKEY_RIGHT_SHIFT        344
#define PDKEY_RIGHT_CONTROL      345
#define PDKEY_RIGHT_ALT          346
#define PDKEY_RIGHT_SUPER        347
#define PDKEY_MENU               348
#define PDKEY_MAX                349

#define PDKEY_SHIFT              1
#define PDKEY_ALT                2
#define PDKEY_CTRL               4
#define PDKEY_SUPER              8

#define PDWHEEL_AXIS_VERTICAL    0
#define PDWHEEL_AXIS_HORIZONTAL  1

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Refactor this?

struct PDMouseWheelEvent
{
    float deltaX;
    float deltaY;
    short wheelDelta;
    short rotation;
    int wheelAxis;
    int keyFlags;
    int linesPerRotation;
    int columnsPerRotation;
};

#endif
