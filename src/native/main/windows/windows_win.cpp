#include "core/core.h"
#include "core/alloc.h"
#include "core/math.h"
#include "core/plugin_handler.h"
#include "resource.h"
#include "ui/menu.h"
#include "../prodbg.h"
#include "../prodbg_version.h"
#include "../settings.h"
#include <uv.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <pd_common.h>
#include <pd_keys.h>

// TODO: Why doesn't winuser.h define this properly?
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef SPI_GETWHEELSCROLLCHARS
#define SPI_GETWHEELSCROLLCHARS 0x006C
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ACCEL s_accelTable[2048];
static int s_accelCount = 0;
static HWND s_window;
static bool s_active = false;
static HMENU s_popupMenu = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void closeWindow() {
    if (s_window) {
        DestroyWindow(s_window);
    }

    //UnregisterClass(L"ProDBG", s_instance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool createWindow(const wchar_t* title, int width, int height) {
    WNDCLASS wc;
    DWORD exStyle;
    DWORD style;
    RECT rect;
    PDGRect settingsRect;

    HINSTANCE instance = GetModuleHandle(0);
    memset(&wc, 0, sizeof(wc));

    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc  = (WNDPROC)WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"ProDBG";
    wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    //wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);

    Settings_getWindowRect(&settingsRect);

    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;

    if (!RegisterClass(&wc)) {
        MessageBox(0, L"Failed To Register Window Class", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    style = WS_OVERLAPPEDWINDOW;

    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    // Create The Window
    if (!(s_window = CreateWindowEx(exStyle, L"ProDBG", PRODBG_VERSION, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                    0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, instance, NULL))) {
        closeWindow();                              // Reset The Display
        return FALSE;                               // Return FALSE
    }

    prodbg_create((void*)s_window, width, height);

    ShowWindow(s_window, SW_SHOW);
    SetForegroundWindow(s_window);
    SetFocus(s_window);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addAccelarator(const PDMenuItem* desc) {
    uint8_t virt = 0;
    uint32_t winMod = desc->winMod;
    uint32_t key = desc->key;
    ACCEL* accel = &s_accelTable[s_accelCount++];

    if (winMod & PRODBG_KEY_ALT)
        virt |= 0x10;
    if (winMod & PRODBG_KEY_CTRL)
        virt |= 0x08;
    if (winMod & PRODBG_KEY_SHIFT)
        virt |= 0x04;

    if (key < 127) {
        if (virt != 0) {
            accel->key = (char)(toupper(key));
            virt |= 1;
        }else {
            accel->key = (char)(key);
        }
    }else {
        virt |= 1;

        switch (key) {
            case PRODBG_KEY_ARROW_DOWN:
                accel->key = VK_DOWN; break;
            case PRODBG_KEY_ARROW_UP:
                accel->key = VK_UP; break;
            case PRODBG_KEY_ARROW_RIGHT:
                accel->key = VK_RIGHT; break;
            case PRODBG_KEY_ARROW_LEFT:
                accel->key = VK_LEFT; break;
            case PRODBG_KEY_F1:
                accel->key = VK_F1; break;
            case PRODBG_KEY_F2:
                accel->key = VK_F2; break;
            case PRODBG_KEY_F3:
                accel->key = VK_F3; break;
            case PRODBG_KEY_F4:
                accel->key = VK_F4; break;
            case PRODBG_KEY_F5:
                accel->key = VK_F5; break;
            case PRODBG_KEY_F6:
                accel->key = VK_F6; break;
            case PRODBG_KEY_F7:
                accel->key = VK_F7; break;
            case PRODBG_KEY_F8:
                accel->key = VK_F8; break;
            case PRODBG_KEY_F9:
                accel->key = VK_F9; break;
            case PRODBG_KEY_F10:
                accel->key = VK_F10; break;
            case PRODBG_KEY_F11:
                accel->key = VK_F11; break;
            case PRODBG_KEY_F12:
                accel->key = VK_F12; break;
                //case PRODBG_KEY_ESC: accel->key = VK_ESCAPE; break;
                //case PRODBG_KEY_TAB: accel->key = VK_TAB; break;
                //case PRODBG_KEY_BACKSPACE: accel->key = VK_BACK; break;
                //case PRODBG_KEY_ENTER: accel->key = VK_RETURN; break;
                //case PRODBG_KEY_SPACE: accel->key = VK_SPACE; break;
                //case PRODBG_KEY_PAGE_DOWN: accel->key = VK_NEXT; break;
                //case PRODBG_KEY_PAGE_UP: accel->key = VK_PRIOR; break;
        }
    }

    accel->fVirt = virt;
    accel->cmd = (WORD)desc->id;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void formatName(char* outName, int keyMod, int key, const char* name) {
    char modName[64] = { 0 };
    char keyName[64] = { 0 };

    if ((keyMod & PRODBG_KEY_COMMAND))
        strcat(modName, "Win-");

    if ((keyMod & PRODBG_KEY_CTRL))
        strcat(modName, "Ctrl-");

    if ((keyMod & PRODBG_KEY_ALT))
        strcat(modName, "Alt-");

    if ((keyMod & PRODBG_KEY_SHIFT))
        strcat(modName, "Shift-");

    if (key < 127) {
        keyName[0] = toupper(key);
        keyName[1] = 0;
    }else {
        switch (key) {
            case PRODBG_KEY_ARROW_DOWN:
                strcpy(keyName, "Down"); break;
            case PRODBG_KEY_ARROW_UP:
                strcpy(keyName, "Up"); break;
            case PRODBG_KEY_ARROW_RIGHT:
                strcpy(keyName, "Right"); break;
            case PRODBG_KEY_ARROW_LEFT:
                strcpy(keyName, "Left"); break;
        }
    }

    sprintf(outName, "%s\t%s%s", name, modName, keyName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void buildSubMenu(HMENU parentMenu, PDMenuItem menuDesc[], wchar_t* name, uint32_t idOffset) {
    wchar_t tempWchar[512];

    PDMenuItem* desc = &menuDesc[0];
    HMENU menu = CreatePopupMenu();
    AppendMenu(parentMenu, MF_STRING | MF_POPUP, (UINT)menu, name);

    while (desc->name) {
        if (desc->id == PRODBG_MENU_SEPARATOR) {
            AppendMenu(menu, MF_SEPARATOR, 0, L"-");
        }else if (desc->id == PRODBG_MENU_SUB_MENU) {
            HMENU subMenu = CreatePopupMenu();

            uv_utf8_to_utf16(desc->name, tempWchar, sizeof_array(tempWchar));

            AppendMenu(menu, MF_STRING | MF_POPUP, (UINT)subMenu, tempWchar);
        }else {
            char temp[512];

            formatName(temp, desc->winMod, desc->key, desc->name);

            uv_utf8_to_utf16(temp, tempWchar, sizeof_array(tempWchar));

            AppendMenu(menu, MF_STRING, desc->id + idOffset, tempWchar);
            addAccelarator(desc);
        }

        desc++;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void buildPopupSubmenu(HMENU parentMenu, wchar_t* inName, PDMenuItem* pluginsMenu, int count, uint32_t startId, uint32_t idMask) {
    for (int i = 0; i < count; ++i) {
        pluginsMenu[i].key = 0;
        pluginsMenu[i].id = (uint32_t)i | idMask;
    }

    buildSubMenu(parentMenu, pluginsMenu, inName, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_buildMenu() {
    HMENU mainMenu = CreateMenu();
    buildSubMenu(mainMenu, g_fileMenu, L"&File", 0);
    buildSubMenu(mainMenu, g_debugMenu, L"&Debug", 0);
    SetMenu(s_window, mainMenu);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO: Remove platform duplication

static PDMenuItem* buildPluginsMenu(PluginData** plugins, int count) {
    PDMenuItem* menu = (PDMenuItem*)alloc_zero(sizeof(PDMenuItem) * (count + 1)); // + 1 as array needs to end with zeros

    for (int i = 0; i < count; ++i) {
        PluginData* pluginData = plugins[i];
        PDPluginBase* pluginBase = (PDPluginBase*)pluginData->plugin;
        PDMenuItem* entry = &menu[i];

        entry->name = pluginBase->name;
        entry->id = PRODBG_MENU_PLUGIN_START + i;
        entry->key = '1' + i;
        entry->macMod = PRODBG_KEY_COMMAND;
        entry->winMod = PRODBG_KEY_CTRL;
    }

    return menu;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void buildPopupMenu(PDMenuItem* pluginsMenu, int count) {
    // TODO: Support rebuild of this menu

    if (s_popupMenu)
        return;

    s_popupMenu = CreatePopupMenu();

    buildPopupSubmenu(s_popupMenu, L"Split Horizontally", pluginsMenu, count,
                      PRODBG_MENU_POPUP_SPLIT_HORZ, PRODBG_MENU_POPUP_SPLIT_HORZ_SHIFT);

    buildPopupSubmenu(s_popupMenu, L"Split Vertically", pluginsMenu, count,
                      PRODBG_MENU_POPUP_SPLIT_VERT, PRODBG_MENU_POPUP_SPLIT_VERT_SHIFT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_addMenu(const char* name, PDMenuItem* items, uint32_t idOffset) {
    wchar_t tempWchar[512];
    HMENU mainMenu = GetMenu(s_window);

    uv_utf8_to_utf16(name, tempWchar, sizeof_array(tempWchar));

    buildSubMenu(mainMenu, items, tempWchar, idOffset);

    DrawMenuBar(s_window);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Window_buildPluginMenu(PluginData** plugins, int count) {
    HMENU mainMenu = GetMenu(s_window);

    PDMenuItem* menu = buildPluginsMenu(plugins, count);

    buildSubMenu(mainMenu, menu, L"&Plugins", 0);
    buildPopupMenu(menu, count);

    DrawMenuBar(s_window);

    return PRODBG_MENU_PLUGIN_START;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_setTitle(const wchar_t* title) {
    SetWindowText(s_window, title);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieves and translates modifier keys

static int getKeyMods() {
    int mods = 0;

    if (GetKeyState(VK_SHIFT) & (1 << 31))
        mods |= PDKEY_SHIFT;
    if (GetKeyState(VK_CONTROL) & (1 << 31))
        mods |= PDKEY_CTRL;
    if (GetKeyState(VK_MENU) & (1 << 31))
        mods |= PDKEY_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & (1 << 31))
        mods |= PDKEY_SUPER;

    return mods;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Translates a Windows key to the corresponding PRODBG key (code taken from GLFW)

static int translateKey(WPARAM wParam, LPARAM lParam) {
    // Check for numeric keypad keys
    // NOTE: This way we always force "NumLock = ON", which is intentional since
    // the returned key code should correspond to a physical location.

    if ((HIWORD(lParam) & 0x100) == 0) {
        switch (MapVirtualKey(HIWORD(lParam) & 0xFF, 1)) {
            case VK_INSERT:
                return PDKEY_KP_0;
            case VK_END:
                return PDKEY_KP_1;
            case VK_DOWN:
                return PDKEY_KP_2;
            case VK_NEXT:
                return PDKEY_KP_3;
            case VK_LEFT:
                return PDKEY_KP_4;
            case VK_CLEAR:
                return PDKEY_KP_5;
            case VK_RIGHT:
                return PDKEY_KP_6;
            case VK_HOME:
                return PDKEY_KP_7;
            case VK_UP:
                return PDKEY_KP_8;
            case VK_PRIOR:
                return PDKEY_KP_9;
            case VK_DIVIDE:
                return PDKEY_KP_DIVIDE;
            case VK_MULTIPLY:
                return PDKEY_KP_MULTIPLY;
            case VK_SUBTRACT:
                return PDKEY_KP_SUBTRACT;
            case VK_ADD:
                return PDKEY_KP_ADD;
            case VK_DELETE:
                return PDKEY_KP_DECIMAL;
            default:
                break;
        }
    }

    // Check which key was pressed or released
    switch (wParam) {
        // The SHIFT keys require special handling
        case VK_SHIFT:
        {
            // Compare scan code for this key with that of VK_RSHIFT in
            // order to determine which shift key was pressed (left or
            // right)
            const DWORD scancode = MapVirtualKey(VK_RSHIFT, 0);
            if ((DWORD)((lParam & 0x01ff0000) >> 16) == scancode)
                return PDKEY_RIGHT_SHIFT;

            return PDKEY_LEFT_SHIFT;
        }

        // The CTRL keys require special handling
        case VK_CONTROL:
        {
            MSG next;
            DWORD time;

            // Is this an extended key (i.e. right key)?
            if (lParam & 0x01000000)
                return PDKEY_RIGHT_CONTROL;

            // Here is a trick: "Alt Gr" sends LCTRL, then RALT. We only
            // want the RALT message, so we try to see if the next message
            // is a RALT message. In that case, this is a false LCTRL!
            time = GetMessageTime();

            if (PeekMessage(&next, NULL, 0, 0, PM_NOREMOVE)) {
                if (next.message == WM_KEYDOWN ||
                    next.message == WM_SYSKEYDOWN ||
                    next.message == WM_KEYUP ||
                    next.message == WM_SYSKEYUP) {
                    if (next.wParam == VK_MENU &&
                        (next.lParam & 0x01000000) &&
                        next.time == time) {
                        // Next message is a RALT down message, which
                        // means that this is not a proper LCTRL message
                        return PDKEY_UNKNOWN;
                    }
                }
            }

            return PDKEY_LEFT_CONTROL;
        }

        // The ALT keys require special handling
        case VK_MENU:
        {
            // Is this an extended key (i.e. right key)?
            if (lParam & 0x01000000)
                return PDKEY_RIGHT_ALT;

            return PDKEY_LEFT_ALT;
        }

        // The ENTER keys require special handling
        case VK_RETURN:
        {
            // Is this an extended key (i.e. right key)?
            if (lParam & 0x01000000)
                return PDKEY_KP_ENTER;

            return PDKEY_ENTER;
        }

        // Funcion keys (non-printable keys)
        case VK_ESCAPE:
            return PDKEY_ESCAPE;
        case VK_TAB:
            return PDKEY_TAB;
        case VK_BACK:
            return PDKEY_BACKSPACE;
        case VK_HOME:
            return PDKEY_HOME;
        case VK_END:
            return PDKEY_END;
        case VK_PRIOR:
            return PDKEY_PAGE_UP;
        case VK_NEXT:
            return PDKEY_PAGE_DOWN;
        case VK_INSERT:
            return PDKEY_INSERT;
        case VK_DELETE:
            return PDKEY_DELETE;
        case VK_LEFT:
            return PDKEY_LEFT;
        case VK_UP:
            return PDKEY_UP;
        case VK_RIGHT:
            return PDKEY_RIGHT;
        case VK_DOWN:
            return PDKEY_DOWN;
        case VK_F1:
            return PDKEY_F1;
        case VK_F2:
            return PDKEY_F2;
        case VK_F3:
            return PDKEY_F3;
        case VK_F4:
            return PDKEY_F4;
        case VK_F5:
            return PDKEY_F5;
        case VK_F6:
            return PDKEY_F6;
        case VK_F7:
            return PDKEY_F7;
        case VK_F8:
            return PDKEY_F8;
        case VK_F9:
            return PDKEY_F9;
        case VK_F10:
            return PDKEY_F10;
        case VK_F11:
            return PDKEY_F11;
        case VK_F12:
            return PDKEY_F12;
        case VK_F13:
            return PDKEY_F13;
        case VK_F14:
            return PDKEY_F14;
        case VK_F15:
            return PDKEY_F15;
        case VK_F16:
            return PDKEY_F16;
        case VK_F17:
            return PDKEY_F17;
        case VK_F18:
            return PDKEY_F18;
        case VK_F19:
            return PDKEY_F19;
        case VK_F20:
            return PDKEY_F20;
        case VK_F21:
            return PDKEY_F21;
        case VK_F22:
            return PDKEY_F22;
        case VK_F23:
            return PDKEY_F23;
        case VK_F24:
            return PDKEY_F24;
        case VK_NUMLOCK:
            return PDKEY_NUM_LOCK;
        case VK_CAPITAL:
            return PDKEY_CAPS_LOCK;
        case VK_SNAPSHOT:
            return PDKEY_PRINT_SCREEN;
        case VK_SCROLL:
            return PDKEY_SCROLL_LOCK;
        case VK_PAUSE:
            return PDKEY_PAUSE;
        case VK_LWIN:
            return PDKEY_LEFT_SUPER;
        case VK_RWIN:
            return PDKEY_RIGHT_SUPER;
        case VK_APPS:
            return PDKEY_MENU;

        // Numeric keypad
        case VK_NUMPAD0:
            return PDKEY_KP_0;
        case VK_NUMPAD1:
            return PDKEY_KP_1;
        case VK_NUMPAD2:
            return PDKEY_KP_2;
        case VK_NUMPAD3:
            return PDKEY_KP_3;
        case VK_NUMPAD4:
            return PDKEY_KP_4;
        case VK_NUMPAD5:
            return PDKEY_KP_5;
        case VK_NUMPAD6:
            return PDKEY_KP_6;
        case VK_NUMPAD7:
            return PDKEY_KP_7;
        case VK_NUMPAD8:
            return PDKEY_KP_8;
        case VK_NUMPAD9:
            return PDKEY_KP_9;
        case VK_DIVIDE:
            return PDKEY_KP_DIVIDE;
        case VK_MULTIPLY:
            return PDKEY_KP_MULTIPLY;
        case VK_SUBTRACT:
            return PDKEY_KP_SUBTRACT;
        case VK_ADD:
            return PDKEY_KP_ADD;
        case VK_DECIMAL:
            return PDKEY_KP_DECIMAL;

        // Printable keys are mapped according to US layout
        case VK_SPACE:
            return PDKEY_SPACE;
        case 0x30:
            return PDKEY_0;
        case 0x31:
            return PDKEY_1;
        case 0x32:
            return PDKEY_2;
        case 0x33:
            return PDKEY_3;
        case 0x34:
            return PDKEY_4;
        case 0x35:
            return PDKEY_5;
        case 0x36:
            return PDKEY_6;
        case 0x37:
            return PDKEY_7;
        case 0x38:
            return PDKEY_8;
        case 0x39:
            return PDKEY_9;
        case 0x41:
            return PDKEY_A;
        case 0x42:
            return PDKEY_B;
        case 0x43:
            return PDKEY_C;
        case 0x44:
            return PDKEY_D;
        case 0x45:
            return PDKEY_E;
        case 0x46:
            return PDKEY_F;
        case 0x47:
            return PDKEY_G;
        case 0x48:
            return PDKEY_H;
        case 0x49:
            return PDKEY_I;
        case 0x4A:
            return PDKEY_J;
        case 0x4B:
            return PDKEY_K;
        case 0x4C:
            return PDKEY_L;
        case 0x4D:
            return PDKEY_M;
        case 0x4E:
            return PDKEY_N;
        case 0x4F:
            return PDKEY_O;
        case 0x50:
            return PDKEY_P;
        case 0x51:
            return PDKEY_Q;
        case 0x52:
            return PDKEY_R;
        case 0x53:
            return PDKEY_S;
        case 0x54:
            return PDKEY_T;
        case 0x55:
            return PDKEY_U;
        case 0x56:
            return PDKEY_V;
        case 0x57:
            return PDKEY_W;
        case 0x58:
            return PDKEY_X;
        case 0x59:
            return PDKEY_Y;
        case 0x5A:
            return PDKEY_Z;
        case 0xBD:
            return PDKEY_MINUS;
        case 0xBB:
            return PDKEY_EQUAL;
        case 0xDB:
            return PDKEY_LEFT_BRACKET;
        case 0xDD:
            return PDKEY_RIGHT_BRACKET;
        case 0xDC:
            return PDKEY_BACKSLASH;
        case 0xBA:
            return PDKEY_SEMICOLON;
        case 0xDE:
            return PDKEY_APOSTROPHE;
        case 0xC0:
            return PDKEY_GRAVE_ACCENT;
        case 0xBC:
            return PDKEY_COMMA;
        case 0xBE:
            return PDKEY_PERIOD;
        case 0xBF:
            return PDKEY_SLASH;
        case 0xDF:
            break;
        case 0xE2:
            break;
        default:
            break;
    }

    // No matching translation was found
    return PDKEY_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   static void handleMouseWheel(HWND window, int axis, WPARAM wParam, LPARAM lParam)
   {
    static int linesPerRotation = -1;
    if (linesPerRotation == -1)
    {
        if (!SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &linesPerRotation, 0))
        {
            // The default is 3, so use it if SystemParametersInfo() failed
            linesPerRotation = 3;
        }
    }

    static int columnsPerRotation = -1;
    if (columnsPerRotation == -1)
    {
        if (!SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &columnsPerRotation, 0))
        {
            // This setting is not supported on Windows 2000/XP, so use the value of 1
            // http://msdn.microsoft.com/en-us/library/ms997498.aspx
            columnsPerRotation = 1;
        }
    }

    PDMouseWheelEvent wheelEvent;

    POINT pt;

    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);

    ScreenToClient(window, &pt);

    //wheelEvent.wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    wheelEvent.wheelDelta = WHEEL_DELTA;
    wheelEvent.rotation = short(HIWORD(wParam));

    wheelEvent.keyFlags = 0;

    if ((LOWORD(wParam) & MK_CONTROL) != 0)
        wheelEvent.keyFlags |= PDKEY_CTRL;

    if ((LOWORD(wParam) & MK_SHIFT) != 0)
        wheelEvent.keyFlags |= PDKEY_SHIFT;

    wheelEvent.linesPerRotation = linesPerRotation;
    wheelEvent.columnsPerRotation = columnsPerRotation;
    wheelEvent.wheelAxis = axis;
    wheelEvent.deltaX = float(pt.x);
    wheelEvent.deltaY = float(pt.y);

    //ProDBG_scroll(wheelEvent);
   }
 */

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_ACTIVATE:
        {
            if (!HIWORD(wParam))
                s_active = true;
            else
                s_active = true;

            break;
        }

        case WM_LBUTTONDOWN:
        {
            ProDBG_set_mouste_state(0, 1);
            ProDBG_update();
            break;
        }

        case WM_LBUTTONUP:
        {
            ProDBG_set_mouste_state(0, 0);
            ProDBG_update();
            break;
        }

        case WM_MOUSEWHEEL:
        {
            //handleMouseWheel(window, PDWHEEL_AXIS_VERTICAL, wParam, lParam);
            break;
        }

        case WM_MOUSEHWHEEL:
        {
            //handleMouseWheel(window, PDWHEEL_AXIS_HORIZONTAL, wParam, lParam);
            break;
        }

        case WM_RBUTTONDOWN:
        {
            if (s_popupMenu) {
                POINT p;
                GetCursorPos(&p);

                TrackPopupMenuEx(s_popupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, p.x, p.y, window, NULL);
            }

            break;
        }

        case WM_MOUSEMOVE:
        {
            const short pos_x = GET_X_LPARAM(lParam);
            const short pos_y = GET_Y_LPARAM(lParam);

            if (wParam & MK_LBUTTON)
                prodbg_set_mouse_state(0, 1);
            else
                prodbg_set_mouse_state(0, 0);

            ProDBG_setMousePos((float)pos_x, (float)pos_y);
            ProDBG_update();

            return 0;
        }

        case WM_COMMAND:
        {
            const int key = translateKey(wParam, lParam);
            int loword = LOWORD(wParam);

            //if ((loword >= PRODBG_MENU_START) && (loword < PRODBG_MENU_END))
            ProDBG_event(loword);

            break;
        }

        case WM_SYSCOMMAND:
        {
            switch (wParam) {
                // prevent screensaver and power saving
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }
            break;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            const int scancode = (lParam >> 16) & 0xff;
            const int key = translateKey(wParam, lParam);
            if (key == PDKEY_UNKNOWN)
                break;

            ProDBG_keyDown(key, getKeyMods());

            break;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            const int mods = getKeyMods();
            const int scancode = (lParam >> 16) & 0xff;
            const int key = translateKey(wParam, lParam);
            if (key == PDKEY_UNKNOWN)
                break;

            ProDBG_keyUp(key, getKeyMods());

            break;
        }

        case WM_CLOSE:
        {
            RECT rect;
            PDGRect settingsRect;

            GetWindowRect(window, &rect);

            settingsRect.x = rect.left;
            settingsRect.y = rect.top;
            settingsRect.width = rect.right;
            settingsRect.height = rect.bottom;

            Settings_setWindowRect(&settingsRect);

            prodbg_destroy();
            PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE:
        {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            ProDBG_setWindowSize(width, height);

            //TODO: Update size
            return 0;
        }

        case WM_CHAR:
        {
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (wParam > 0 && wParam < 0x10000)
                ProDBG_addChar((unsigned short)wParam);
            break;
        }
    }

    return DefWindowProc(window, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CALLBACK timedCallback(HWND hwnd, UINT id, UINT_PTR ptr, DWORD meh) {
    ProDBG_timedUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmndLine, int show) {
    MSG msg;
    HACCEL accel;
    bool done = false;

    memset(&msg, 0, sizeof(MSG));

    Settings_load();

    if (!createWindow(L"ProDBG", 1280, 720))
        return 0;

    Window_buildMenu();
    ProDBG_applicationLaunched();

    accel = CreateAcceleratorTable(s_accelTable, s_accelCount);

    // Update timed function every 16 ms

    SetTimer(s_window, 1, 1, timedCallback);

    while (!done) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (!TranslateAccelerator(s_window, accel, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                if (WM_QUIT == msg.message)
                    done = true;
            }
        }

        Sleep(1); // to prevent hammering the thread
    }

    closeWindow();
    return (int)msg.wParam;
}

