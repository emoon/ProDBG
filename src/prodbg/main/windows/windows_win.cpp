#include <windows.h>
#include <windowsx.h>
#include "resource.h"
#include "../prodbg.h"
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static HWND s_window;
static bool s_active = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

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

	HINSTANCE instance = GetModuleHandle(0);
	memset(&wc, 0, sizeof(wc));

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc	= (WNDPROC)WndProc;
	wc.hInstance = instance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"ProDBG";
	wc.hIcon = LoadIcon(instance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MENU);

	rect.left = 0;
	rect.right = width;
	rect.top = 0;
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
		closeWindow();								// Reset The Display
		return FALSE;								// Return FALSE
	}

	prodbg::ProDBG_create((void*)s_window, width, height);

	ShowWindow(s_window, SW_SHOW);
	SetForegroundWindow(s_window);
	SetFocus(s_window);

	return TRUE;
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
			prodbg::ProDBG_setMouseState(0, 1);
			prodbg::ProDBG_update();
			break;
		}

		case WM_LBUTTONUP:
		{
			prodbg::ProDBG_setMouseState(0, 0);
			prodbg::ProDBG_update();
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
				prodbg::ProDBG_setMouseState(0, 1);
			else
				prodbg::ProDBG_setMouseState(0, 0);

			prodbg::ProDBG_setMousePos((float)pos_x, (float)pos_y);
			prodbg::ProDBG_update();

			return 0;
		}

		/*
        case WM_KEYDOWN:
        //case WM_SYSKEYDOWN:
		{
			int key = onKeyDown(wParam, lParam);

			if (key != -1)
			{
				int modifier = getModifiers();
				Emgui_sendKeyinput(key, modifier);
				Editor_keyDown(key, -1, modifier);
				Editor_update();
			}

			break;
		}
		*/

		case WM_COMMAND:
		{
			/*
			// make sure we send tabs, enters and backspaces down to emgui as well, a bit hacky but well

			if (LOWORD(wParam) == EDITOR_MENU_TAB)
				Emgui_sendKeyinput(EMGUI_KEY_TAB, getModifiers());

			if (LOWORD(wParam) == EDITOR_MENU_CANCEL_EDIT)
				Emgui_sendKeyinput(EMGUI_KEY_ESC, getModifiers());

			if (LOWORD(wParam) == EDITOR_MENU_ENTER_CURRENT_V)
				Emgui_sendKeyinput(EMGUI_KEY_ENTER, getModifiers());

			if (LOWORD(wParam) == EDITOR_MENU_DELETE_KEY)
				Emgui_sendKeyinput(EMGUI_KEY_BACKSPACE, getModifiers());

			switch (LOWORD(wParam))
			{
				case EDITOR_MENU_OPEN:
				case EDITOR_MENU_SAVE:
				case EDITOR_MENU_SAVE_AS:
				case EDITOR_MENU_REMOTE_EXPORT:
				case EDITOR_MENU_RECENT_FILE_0:
				case EDITOR_MENU_RECENT_FILE_1:
				case EDITOR_MENU_RECENT_FILE_2:
				case EDITOR_MENU_RECENT_FILE_3:
				case EDITOR_MENU_UNDO:
				case EDITOR_MENU_REDO:
				case EDITOR_MENU_CANCEL_EDIT:
				case EDITOR_MENU_DELETE_KEY:
				case EDITOR_MENU_CUT:
				case EDITOR_MENU_COPY:
				case EDITOR_MENU_PASTE:
				case EDITOR_MENU_MOVE_UP:
				case EDITOR_MENU_MOVE_DOWN:
				case EDITOR_MENU_SELECT_TRACK:
				case EDITOR_MENU_BIAS_P_001:
				case EDITOR_MENU_BIAS_P_01:
				case EDITOR_MENU_BIAS_P_1:
				case EDITOR_MENU_BIAS_P_10:
				case EDITOR_MENU_BIAS_P_100:
				case EDITOR_MENU_BIAS_P_1000:
				case EDITOR_MENU_BIAS_N_001:
				case EDITOR_MENU_BIAS_N_01:
				case EDITOR_MENU_BIAS_N_1:
				case EDITOR_MENU_BIAS_N_10:
				case EDITOR_MENU_BIAS_N_100:
				case EDITOR_MENU_BIAS_N_1000:
				case EDITOR_MENU_SCALE_101:	
				case EDITOR_MENU_SCALE_11:	
				case EDITOR_MENU_SCALE_12:	
				case EDITOR_MENU_SCALE_5:	
				case EDITOR_MENU_SCALE_100:	
				case EDITOR_MENU_SCALE_1000:	
				case EDITOR_MENU_SCALE_099:	
				case EDITOR_MENU_SCALE_09:	
				case EDITOR_MENU_SCALE_08:	
				case EDITOR_MENU_SCALE_05:	
				case EDITOR_MENU_SCALE_01:	
				case EDITOR_MENU_INTERPOLATION:
				case EDITOR_MENU_INVERT_SELECTION:
				case EDITOR_MENU_MUTE_TRACK:
				case EDITOR_MENU_ENTER_CURRENT_V:
				case EDITOR_MENU_TAB:
				case EDITOR_MENU_PLAY:
				case EDITOR_MENU_PLAY_LOOP:
				case EDITOR_MENU_ROWS_UP:
				case EDITOR_MENU_ROWS_DOWN:
				case EDITOR_MENU_ROWS_2X_UP:
				case EDITOR_MENU_ROWS_2X_DOWN:
				case EDITOR_MENU_PREV_BOOKMARK:
				case EDITOR_MENU_NEXT_BOOKMARK:
				case EDITOR_MENU_SCROLL_LEFT:
				case EDITOR_MENU_SCROLL_RIGHT:
				case EDITOR_MENU_PREV_KEY:
				case EDITOR_MENU_NEXT_KEY:
				case EDITOR_MENU_FOLD_TRACK:
				case EDITOR_MENU_UNFOLD_TRACK:
				case EDITOR_MENU_FOLD_GROUP:
				case EDITOR_MENU_UNFOLD_GROUP:
				case EDITOR_MENU_TOGGLE_BOOKMARK:
				case EDITOR_MENU_CLEAR_BOOKMARKS:
				case EDITOR_MENU_TOGGLE_LOOPMARK:
				case EDITOR_MENU_CLEAR_LOOPMARKS:
				{
					Editor_menuEvent(LOWORD(wParam));
					break;
				}
			}
			*/
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
			/*
			int res;
			
			if (!Editor_needsSave())
			{
				PostQuitMessage(0);
				return 0;
			}

			res = MessageBox(window, L"Do you want to save the work?", L"Save before exit?", MB_YESNOCANCEL | MB_ICONQUESTION); 

			if (res == IDCANCEL)
				return 0;

			if (res == IDYES)
				Editor_saveBeforeExit();
			*/

			PostQuitMessage(0);
			return 0;
		}

		case WM_SIZE:
		{
			//EMGFXBackend_updateViewPort(LOWORD(lParam), HIWORD(lParam));
			//Editor_setWindowSize(LOWORD(lParam), HIWORD(lParam));
			//Editor_update();
			return 0;
		}
	}

	return DefWindowProc(window, message, wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void CALLBACK timedCallback(HWND hwnd, UINT id, UINT_PTR ptr, DWORD meh)
{
	prodbg::ProDBG_timedUpdate();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmndLine, int show)
{
	MSG	msg;
	HACCEL accel;
	bool done = false;	

	memset(&msg, 0, sizeof(MSG));

	if (!createWindow(L"ProDBG", 1280, 720))
		return 0;

	//accel = CreateAcceleratorTable(s_accelTable, s_accelCount);

	// Update timed function every 16 ms

	SetTimer(s_window, 1, 16, timedCallback);

	while (!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//if (!TranslateAccelerator(s_window, accel, &msg))
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
	return msg.wParam;
}

