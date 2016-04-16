extern crate minifb;
extern crate bgfx_rs;
extern crate viewdock;

use bgfx_rs::Bgfx;
use libc::{c_void, c_int};
use minifb::{Scale, WindowOptions, MouseMode, MouseButton};
use core::view_plugins::{ViewHandle, ViewPlugins, ViewInstance};
use core::session::{Sessions, Session, SessionHandle};
use self::viewdock::{Workspace, Rect};
use imgui_sys::Imgui;
use prodbg_api::ui_ffi::{PDVec2};
use prodbg_api::view::CViewCallbacks;

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
    fn is_inside(v: (f32, f32), pos: PDVec2, size: PDVec2) -> bool {
        let x0 = pos.x;
        let y0 = pos.y;
        let x1 = pos.x + size.x;
        let y1 = pos.y + size.y;

        if (v.0 >= x0 && v.0 < x1) && (v.1 >= y0 && v.1 < y1) {
            true
        } else {
            false
        }
    }

    fn update_view(instance: &mut ViewInstance, session: &mut Session, show_context_menu: bool, mouse: (f32, f32)) -> u32 {
        let ui = instance.ui;

        //bgfx_imgui_set_window_pos(0.0, 0.0);
        //bgfx_imgui_set_window_size(500.0, 500.0);

        Imgui::begin_window("Test", true);
        Imgui::init_state(ui.api);

        let pos = ui.get_window_pos();
        let size = ui.get_window_size();

        if Self::is_inside(mouse, pos, size) && show_context_menu {
            Imgui::mark_show_popup(ui.api, true);
        } else {
            Imgui::mark_show_popup(ui.api, false);
        }

        unsafe {
            let plugin_funcs = instance.plugin_type.plugin_funcs as *mut CViewCallbacks;
            ((*plugin_funcs).update.unwrap())(instance.plugin_data,
                                                ui.api as *mut c_void,
                                                session.reader.api as *mut c_void,
                                                session.get_current_writer().api as *mut c_void);
        }

        let has_shown_menu = Imgui::has_showed_popup(ui.api);

        Imgui::end_window();

        has_shown_menu
    }

    pub fn update(&mut self, sessions: &mut Sessions, view_plugins: &mut ViewPlugins) {
        self.win.update();

        let mut has_shown_menu = 0u32;

        let mouse = self.win.get_mouse_pos(MouseMode::Clamp).unwrap_or((0.0, 0.0));

        Bgfx::set_mouse_pos(mouse);
        Bgfx::set_mouse_state(0, self.win.get_mouse_down(MouseButton::Left));

        let show_context_menu = self.win.get_mouse_down(MouseButton::Right);

        for view in &self.views {
            if let Some(ref mut v) = view_plugins.get_view(*view) {
                if let Some(ref mut s) = sessions.get_session(v.session_handle) {
                    has_shown_menu |= Self::update_view(v, s, show_context_menu, mouse);
                }
            }
        }

        // if now plugin has showed a menu we do it here
        // TODO: Handle diffrent cases when attach menu on to plugin menu or not

        if has_shown_menu == 0 && show_context_menu {
            Self::show_popup(self, true, view_plugins);
        } else {
            Self::show_popup(self, false, view_plugins);
        }
    }

    fn add_view(&mut self, name: &String, view_plugins: &mut ViewPlugins) {
        let ui = Imgui::create_ui_instance();
        let view = view_plugins.create_instance(ui, name, SessionHandle(0)).unwrap();
        self.views.push(view);
    }

    fn show_popup(&mut self, show: bool, view_plugins: &mut ViewPlugins) {
        let ui = Imgui::get_ui();

        if show {
            ui.open_popup("plugins");
        }

        if ui.begin_popup("plugins") {
            let plugin_names = view_plugins.get_plugin_names();

            if ui.begin_menu("Split Horizontally", true) {
                for name in &plugin_names {
                    if ui.menu_item(name, false, true) {
                        Self::add_view(self, &name, view_plugins);
                    }
                }
                ui.end_menu();
            }

            if ui.begin_menu("Split Vertically", true) {
                for name in &plugin_names {
                    ui.menu_item(name, false, true);
                }
                ui.end_menu();
            }

            ui.end_popup();
        }
    }

    /*
    pub fn add_view(&mut self, view: ViewHandle) {
        self.views.push(view);
    }
    */
}
