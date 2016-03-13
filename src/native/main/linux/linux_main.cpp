// dummy main

#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#include <bgfx.h>
#include <bgfxplatform.h> // will include X11 which #defines None... Don't mess with order of includes.
#include "../prodbg.h"
#include "core/log.h"
#include <pd_menu.h>

struct PluginData;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static Display* s_display;
static Window s_window;
static Atom wmDeleteWindow;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_buildPluginMenu(struct PluginData**, int) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window_addMenu(const char*, PDMenuItem*, uint32_t) {
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void processEvents() {
    bool exit = false;

    while (!exit) {
        if (XPending(s_display)) {
            XEvent event;
            XNextEvent(s_display, &event);

            switch (event.type) {
                case Expose:
                    break;

                case ClientMessage:
                {
                    if ((Atom)event.xclient.data.l[0] == wmDeleteWindow)
                        exit = true;

                    break;
                }

                case ButtonPress:
                {
                    const XButtonEvent& xbutton = event.xbutton;

                    ProDBG_setMouseState(0, 1);
                    ProDBG_setMousePos(xbutton.x, xbutton.y);

                    ProDBG_update();
                    break;
                }

                case ButtonRelease:
                {
                    const XButtonEvent& xbutton = event.xbutton;

                    ProDBG_setMouseState(0, 0);
                    ProDBG_setMousePos(xbutton.x, xbutton.y);

                    ProDBG_update();

                    break;
                }

                case MotionNotify:
                {
                    const XMotionEvent& xmotion = event.xmotion;

                    ProDBG_setMousePos(xmotion.x, xmotion.y);

                    ProDBG_update();

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

                case ConfigureNotify:
                {
                    const XConfigureEvent& xev = event.xconfigure;
                    ProDBG_setWindowSize(xev.width, xev.height);
                    break;
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char** argv) {
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
                             | StructureNotifyMask
    ;

    int width = 800;
    int height = 600;

    s_window = XCreateWindow(s_display
                             , root
                             , 0, 0
                             , width, height, 0, depth
                             , InputOutput
                             , visual
                             , CWBorderPixel | CWEventMask
                             , &windowAttrs
                             );

    // Clear window to black.
    XSetWindowAttributes attr = { 0 };
    XChangeWindowAttributes(s_display, s_window, CWBackPixel, &attr);

    const char* wmDeleteWindowName = "WM_DELETE_WINDOW";
    XInternAtoms(s_display, (char**)&wmDeleteWindowName, 1, False, &wmDeleteWindow);
    XSetWMProtocols(s_display, s_window, &wmDeleteWindow, 1);

    XMapWindow(s_display, s_window);
    XStoreName(s_display, s_window, "ProDBG");

    bgfx::x11SetDisplayWindow(s_display, s_window);

    ProDBG_create((void*)s_window, width, height);

    processEvents();

    XUnmapWindow(s_display, s_window);
    XDestroyWindow(s_display, s_window);

    return EXIT_SUCCESS;
}

