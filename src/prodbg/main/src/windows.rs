extern crate minifb;
extern crate bgfx_rs;
extern crate viewdock;

use bgfx_rs::Bgfx;
use libc::{c_void, c_int};
use minifb::{Scale, WindowOptions, MouseMode, MouseButton};
use core::view_plugins::{ViewHandle, ViewPlugins, ViewInstance};
use core::session::{Sessions, Session};
use self::viewdock::{Workspace, Rect};

const WIDTH: usize = 1280;
const HEIGHT: usize = 800;

pub struct Window {
    /// minifb window
    pub win: minifb::Window,

    /// Views in this window
    pub views: Vec<ViewHandle>,

    ///
    pub ws: Workspace,
}

///! Windows keeps track of all different windows that are present with in the application
///! There are several ways windows can be created:
///!
///! 1. User opens a new window using a shortcut or menu selection.
///! 2. User "undocks" a view from an existing window giving it it's own floating window.
///! 3. etc

pub struct Windows {
    /// All the windows being tracked
    windows: Vec<Window>,
    current: usize,
}

impl Windows {
    pub fn new() -> Windows {
        Windows {
            windows: Vec::new(),
            current: 0,
        }
    }

    /// Create a default window which will only be created if there are no other
    pub fn create_default(&mut self) {
        if self.windows.len() > 0 {
            return;
        }

        let window = Self::create_window(WIDTH, HEIGHT).expect("Unable to create window");

        self.windows.push(window)
   }

    pub fn create_window(width: usize, height: usize) -> minifb::Result<Window> {
        let res = minifb::Window::new("ProDBG",
                                      width,
                                      height,
                                      WindowOptions {
                                          resize: true,
                                          scale: Scale::X1,
                                          ..WindowOptions::default()
                                      });
        match res {
            Ok(win) => {
                Bgfx::create_window(win.get_window_handle() as *const c_void,
                                    width as c_int,
                                    height as c_int);
                Ok(Window {
                    win: win,
                    views: Vec::new(),
                    ws: Workspace::new(Rect::new(0.0, 0.0, WIDTH as f32, HEIGHT as f32)).unwrap(),
                })
            }
            Err(err) => Err(err),
        }
    }

    pub fn create_window_with_menus(&mut self) -> minifb::Result<Window> {
        const WIDTH: usize = 1280;
        const HEIGHT: usize = 1024;

        let window = try!(Self::create_window(WIDTH, HEIGHT));

        // for menu in &menus {
        // window.add_menu(m.name, m.menu);
        // }
        //

        Ok(window)
    }

    /*
    fn update_window(window: &mut Window, sessions: &mut Sessions, view_plugins: &mut ViewPlugins) {

        window.update(sessions, view_plugins);
    }
    */

    pub fn update(&mut self, sessions: &mut Sessions, view_plugins: &mut ViewPlugins) {
        for i in (0..self.windows.len()).rev() {
            self.windows[i].update(sessions, view_plugins);

            if !self.windows[i].win.is_open() {
                self.windows.swap_remove(i);
            }
        }
    }

    pub fn get_current(&mut self) -> &mut Window {
        let current = self.current;
        &mut self.windows[current]
    }

    /// Checks if application should exit (all window instances closed)
    pub fn should_exit(&self) -> bool {
        self.windows.len() == 0
    }

    /// Save the state of the windows (usually done when exiting the application)
    pub fn save(_filename: &str) {}

    /// Load the state of all the views from a previous run
    pub fn load(_filename: &str) {}
}

impl Window {
    fn update_view(_view: &mut ViewInstance, _session: &mut Session) {

    }

    pub fn update(&mut self, sessions: &mut Sessions, view_plugins: &mut ViewPlugins) {
        self.win.update();

        self.win.get_mouse_pos(MouseMode::Clamp).map(|mouse| {
            Bgfx::set_mouse_pos(mouse);
            Bgfx::set_mouse_state(0, self.win.get_mouse_down(MouseButton::Left));
        });

        for view in &self.views {
            if let Some(ref mut v) = view_plugins.get_view(*view) {
                if let Some(ref mut s) = sessions.get_session(v.session_handle) {
                    Self::update_view(v, s);
                }
            }
        }
    }

    pub fn add_view(&mut self, view: ViewHandle) {
        self.views.push(view);
    }
}
