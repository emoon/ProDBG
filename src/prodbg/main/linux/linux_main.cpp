// dummy main

#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#include <bgfx.h>

struct PluginData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Display* s_display;
static Window s_window;
static Atom wmDeleteWindow;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_buildPluginMenu(struct PluginData**, int)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void processEvents()
{
	bool exit = false;

	while (!exit)
	{
		if (XPending(s_display))
		{
			XEvent event;
			XNextEvent(s_display, &event);

			switch (event.type)
			{
				case Expose:
					break;

				case ConfigureNotify:
					break;

				case ClientMessage:
				{
					if ( (Atom)event.xclient.data.l[0] == wmDeleteWindow)
						exit = true;

					break;
				}

				case ButtonPress:
				case ButtonRelease:
				{
					//const XButtonEvent& xbutton = event.xbutton;
					/*
					MouseButton::Enum mb;
					switch (xbutton.button)
					{
						case Button1: mb = MouseButton::Left;   break;
						case Button2: mb = MouseButton::Middle; break;
						case Button3: mb = MouseButton::Right;  break;
						default:      mb = MouseButton::None;   break;
					}
					*/

					//if (MouseButton::None != mb)
					//{
						/*
						m_eventQueue.postMouseEvent(defaultWindow
							, xbutton.x
							, xbutton.y
							, 0
							, mb
							, event.type == ButtonPress
							);
						*/
					//}
				
					break;
				}

				case MotionNotify:
				{
					//const XMotionEvent& xmotion = event.xmotion;
					/*
					m_eventQueue.postMouseEvent(defaultWindow
							, xmotion.x
							, xmotion.y
							, 0
							);
					*/
					break;
				}

				case KeyPress:
				case KeyRelease:
				{
					/*
					XKeyEvent& xkey = event.xkey;
					KeySym keysym = XLookupKeysym(&xkey, 0);

					switch (keysym)
					{
						case XK_Meta_L:    setModifier(Modifier::LeftMeta,   KeyPress == event.type); break;
						case XK_Meta_R:    setModifier(Modifier::RightMeta,  KeyPress == event.type); break;
						case XK_Control_L: setModifier(Modifier::LeftCtrl,   KeyPress == event.type); break;
						case XK_Control_R: setModifier(Modifier::RightCtrl,  KeyPress == event.type); break;
						case XK_Shift_L:   setModifier(Modifier::LeftShift,  KeyPress == event.type); break;
						case XK_Shift_R:   setModifier(Modifier::RightShift, KeyPress == event.type); break;
						case XK_Alt_L:     setModifier(Modifier::LeftAlt,    KeyPress == event.type); break;
						case XK_Alt_R:     setModifier(Modifier::RightAlt,   KeyPress == event.type); break;

						default:
						{
							Key::Enum key = fromXk(keysym);
							if (Key::None != key)
							{
								//m_eventQueue.postKeyEvent(defaultWindow, key, m_modifiers, KeyPress == event.type);
							}
						
							break;
						}
					}
					*/
				
					break;
				}

				case ResizeRequest:
				{
					//const XResizeRequestEvent& xresize = event.xresizerequest;
					//XResizeWindow(s_display, m_window, xresize.width, xresize.height);
					//m_eventQueue.postSizeEvent(defaultWindow, xresize.width, xresize.height);
				
					break;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv)
{
	XInitThreads();
	s_display = XOpenDisplay(0);

	int32_t screen = DefaultScreen(s_display);
	int32_t depth = DefaultDepth(s_display, screen);
	Visual* visual = DefaultVisual(s_display, screen);
	Window root = RootWindow(s_display, screen);

	XSetWindowAttributes windowAttrs = { 0 };
	windowAttrs.background_pixmap = 0;
	windowAttrs.border_pixel = 0;
	windowAttrs.event_mask = 0
			| ButtonPressMask
			| ButtonReleaseMask
			| ExposureMask
			| KeyPressMask
			| KeyReleaseMask
			| PointerMotionMask
			| ResizeRedirectMask
			| StructureNotifyMask
			;

	s_window = XCreateWindow(s_display
							, root
							, 0, 0
							, 800, 600, 0, depth
							, InputOutput
							, visual
							, CWBorderPixel|CWEventMask
							, &windowAttrs
							);

	// Clear window to black.
	XSetWindowAttributes attr = { 0 };
	XChangeWindowAttributes(s_display, s_window, CWBackPixel, &attr);

	const char* wmDeleteWindowName = "WM_DELETE_WINDOW";
	XInternAtoms(s_display, (char **)&wmDeleteWindowName, 1, False, &wmDeleteWindow);
	XSetWMProtocols(s_display, s_window, &wmDeleteWindow, 1);

	XMapWindow(s_display, s_window);
	XStoreName(s_display, s_window, "ProDBG");

	//
	//bgfx::x11SetDisplayWindow(s_display, s_window);

	processEvents();

	XUnmapWindow(s_display, s_window);
	XDestroyWindow(s_display, s_window);

	return EXIT_SUCCESS;
}

