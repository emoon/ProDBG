#include "core/core.h"
#include "core/alloc.h"
#include "core/math.h"
#include "core/plugin_handler.h"
#include "resource.h"
#include "ui/menu.h"
#include "../prodbg.h"
#include "../settings.h"
#include <uv.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <pd_common.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static ACCEL s_accelTable[2048];
static int s_accelCount = 0;
static HWND s_window;
static bool s_active = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void closeWindow()
{
    if (s_window)
    {
        DestroyWindow(s_window);
    }

    //UnregisterClass(L"ProDBG", s_instance);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool createWindow(const wchar_t* title, int width, int height)
{
    WNDCLASS wc;
    DWORD exStyle;
    DWORD style;
    RECT rect;
    Rect settingsRect;

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

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"Failed To Register Window Class", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    style = WS_OVERLAPPEDWINDOW;

    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    // Create The Window
    if (!(s_window = CreateWindowEx(exStyle, L"ProDBG", title, style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                    0, 0, rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, instance, NULL)))
    {
        closeWindow();                              // Reset The Display
        return FALSE;                               // Return FALSE
    }

    ProDBG_create((void*)s_window, width, height);

    ShowWindow(s_window, SW_SHOW);
    SetForegroundWindow(s_window);
    SetFocus(s_window);

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void addAccelarator(const MenuDescriptor* desc)
{
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

	if (key < 127)
	{
		if (virt != 0)
		{
			accel->key = (char)(key);
			virt |= 1;
		}
		else
		{
			accel->key = (char)(key);
		}
	}
	else
	{
		virt |= 1;

		switch (key)
		{
			case PRODBG_KEY_ARROW_DOWN : accel->key = VK_DOWN; break; 
			case PRODBG_KEY_ARROW_UP:  accel->key = VK_UP; break;
			case PRODBG_KEY_ARROW_RIGHT: accel->key = VK_RIGHT; break;
			case PRODBG_KEY_ARROW_LEFT: accel->key = VK_LEFT; break;
			case PRODBG_KEY_F1: accel->key = VK_F1; break;
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

static void formatName(char* outName, int keyMod, int key, const char* name)
{
	char modName[64] = { 0 } ;
	char keyName[64] = { 0 } ;

	if ((keyMod & PRODBG_KEY_COMMAND))
		strcat(modName, "Win-");

	if ((keyMod & PRODBG_KEY_CTRL))
		strcat(modName, "Ctrl-");

	if ((keyMod & PRODBG_KEY_ALT))
		strcat(modName, "Alt-");

	if ((keyMod & PRODBG_KEY_SHIFT))
		strcat(modName, "Shift-");

	if (key < 127)
	{
		keyName[0] = key;
		keyName[1] = 0;
	}
	else
	{
		switch (key)
		{
			case PRODBG_KEY_ARROW_DOWN : strcpy(keyName, "Down"); break;
			case PRODBG_KEY_ARROW_UP: strcpy(keyName, "Up"); break;
			case PRODBG_KEY_ARROW_RIGHT: strcpy(keyName, "Right"); break;
			case PRODBG_KEY_ARROW_LEFT: strcpy(keyName, "Left"); break;
		}
	}

	sprintf(outName, "%s\t%s%s", name, modName, keyName);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void buildSubMenu(HMENU parentMenu, MenuDescriptor menuDesc[], wchar_t* name)
{
	wchar_t tempWchar[512];

	MenuDescriptor* desc = &menuDesc[0];
	HMENU menu = CreatePopupMenu();
    AppendMenu(parentMenu, MF_STRING | MF_POPUP, (UINT)menu, name);

	while (desc->name)
	{
		if (desc->id == PRODBG_MENU_SEPARATOR)
		{
			AppendMenu(menu, MF_SEPARATOR, 0, L"-");
		}
		else if (desc->id == PRODBG_MENU_SUB_MENU)
		{
			HMENU subMenu = CreatePopupMenu();

			uv_utf8_to_utf16(desc->name, tempWchar, sizeof_array(tempWchar));

    		AppendMenu(menu, MF_STRING | MF_POPUP, (UINT)subMenu, tempWchar);
		}
		else
		{
			char temp[512];

			formatName(temp, desc->winMod, desc->key, desc->name);

			uv_utf8_to_utf16(temp, tempWchar, sizeof_array(tempWchar));

			AppendMenu(menu, MF_STRING, desc->id, tempWchar);
			addAccelarator(desc);
		}

		desc++;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_buildMenu()
{
	HMENU mainMenu = CreateMenu();
    buildSubMenu(mainMenu, g_fileMenu, L"&File");
    buildSubMenu(mainMenu, g_debugMenu, L"&Debug");
	SetMenu(s_window, mainMenu);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Window_buildPluginMenu(PluginData** plugins, int count)
{
	HMENU mainMenu = GetMenu(s_window);

    // TODO: Right now we only support up to 1 - 9 for keyboard shortcuts of the plugins but should be good
    // enough for now.

    if (count >= 10)
        count = 9;

    MenuDescriptor* menu = (MenuDescriptor*)alloc_zero(sizeof(MenuDescriptor) * (count + 1)); // + 1 as array needs to end with zeros

    for (int i = 0; i < count; ++i)
    {
        PluginData* pluginData = plugins[i];
        PDPluginBase* pluginBase = (PDPluginBase*)pluginData->plugin;
        MenuDescriptor* entry = &menu[i];

        // TODO: Hack hack!

        if (!strstr(pluginData->type, "View"))
            continue;

        entry->name = pluginBase->name;
        entry->id = PRODBG_MENU_PLUGIN_START + i;
        entry->key = '1' + i;
        entry->macMod = PRODBG_KEY_COMMAND;
        entry->winMod = PRODBG_KEY_CTRL;
    }

    buildSubMenu(mainMenu, menu, L"&Plugins");

    return PRODBG_MENU_PLUGIN_START;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_setTitle(const wchar_t* title)
{
    SetWindowText(s_window, title);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
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
            ProDBG_setMouseState(0, 1);
            ProDBG_update();
            break;
        }

        case WM_LBUTTONUP:
        {
            ProDBG_setMouseState(0, 0);
            ProDBG_update();
            break;
        }

        case WM_MOUSEWHEEL:
        {
            break;
        }

        case WM_MOUSEMOVE:
        {
            const short pos_x = GET_X_LPARAM(lParam);
            const short pos_y = GET_Y_LPARAM(lParam);

            if (wParam & MK_LBUTTON)
                ProDBG_setMouseState(0, 1);
            else
                ProDBG_setMouseState(0, 0);

            ProDBG_setMousePos((float)pos_x, (float)pos_y);
            ProDBG_update();

            return 0;
        }

        case WM_COMMAND:
        {
        	int loword = LOWORD(wParam);

        	if ((loword >= PRODBG_MENU_START) && (loword < PRODBG_MENU_END))
        		ProDBG_event(loword);

            break;
        }

        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                // prevent screensaver and power saving
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }
            break;
        }

        case WM_CLOSE:
        {
            RECT rect;
            Rect settingsRect;

            GetWindowRect(window, &rect);

            settingsRect.x = rect.left;
            settingsRect.y = rect.top;
            settingsRect.width = rect.right;
            settingsRect.height = rect.bottom;

            Settings_setWindowRect(&settingsRect);

            ProDBG_destroy();
            PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE:
        {
        	//TODO: Update size
            return 0;
        }
    }

    return DefWindowProc(window, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CALLBACK timedCallback(HWND hwnd, UINT id, UINT_PTR ptr, DWORD meh)
{
    ProDBG_timedUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmndLine, int show)
{
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

    SetTimer(s_window, 1, 16, timedCallback);

    while (!done)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(s_window, accel, &msg))
            {
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

