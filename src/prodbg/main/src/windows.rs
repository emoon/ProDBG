extern crate minifb;
extern crate bgfx_rs;
extern crate viewdock;

use bgfx_rs::Bgfx;
use libc::{c_void, c_int};
use minifb::{Scale, WindowOptions, MouseMode, MouseButton, Key, KeyRepeat};
use core::view_plugins::{ViewHandle, ViewPlugins, ViewInstance};
use core::session::{Sessions, Session, SessionHandle};
use core::reader_wrapper::ReaderWrapper;
use self::viewdock::{Workspace, Rect, Direction, DockHandle, SplitHandle};
use menu::Menu;
use imgui_sys::Imgui;
use prodbg_api::ui_ffi::{PDVec2};
use prodbg_api::view::CViewCallbacks;

const WIDTH: usize = 1280;
const HEIGHT: usize = 800;

enum State {
    Default,
    DraggingSlider,
}

pub struct MouseState {
    handle: Option<(SplitHandle, Direction)>,
    state: State,
    prev_mouse: (f32, f32)
}

impl MouseState {
    pub fn new() -> MouseState {
        MouseState {
            handle: None,
            state: State::Default,
            prev_mouse: (0.0, 0.0),
        }
    }
}

pub struct Window<'a> {
    /// minifb window
    pub win: minifb::Window,
    pub menu: Menu<'a>,

    /// Views in this window
    pub views: Vec<ViewHandle>,

    ///
    pub ws: Workspace,

    pub mouse_state: MouseState,
}

struct WindowState {
    pub showed_popup: u32,
    pub should_close: bool,
}

///! Windows keeps track of all different windows that are present with in the application
///! There are several ways windows can be created:
///!
///! 1. User opens a new window using a shortcut or menu selection.
///! 2. User "undocks" a view from an existing window giving it it's own floating window.
///! 3. etc

pub struct Windows<'a> {
    /// All the windows being tracked
    windows: Vec<Window<'a>>,
    current: usize,
}

impl<'a> Windows<'a> {
    pub fn new() -> Windows<'a> {
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

        let window = Self::create_window_with_menus().expect("Unable to create window");

        self.windows.push(window)
   }

    pub fn create_window(width: usize, height: usize) -> minifb::Result<Window<'a>> {
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
                    menu: Menu::new(),
                    views: Vec::new(),
                    mouse_state: MouseState::new(),
                    ws: Workspace::new(Rect::new(0.0, 0.0, width as f32, (height - 20) as f32)).unwrap(),
                })
            }
            Err(err) => Err(err),
        }
    }

    pub fn create_window_with_menus() -> minifb::Result<Window<'a>> {
        let mut window = try!(Self::create_window(WIDTH, HEIGHT));

        try!(window.win.add_menu("File", &window.menu.file_menu));
        try!(window.win.add_menu("Debug", &window.menu.debug_menu));

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

    pub fn get_current(&mut self) -> &'a mut Window {
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

impl<'a> Window<'a> {
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

    fn update_view(&self, instance: &mut ViewInstance, session: &mut Session, show_context_menu: bool, mouse: (f32, f32)) -> WindowState {
        let ui = instance.ui;

        if let Some(rect) = self.ws.get_rect_by_handle(DockHandle(instance.handle.0)) {
            Imgui::set_window_pos(rect.x, rect.y);
            Imgui::set_window_size(rect.width, rect.height);
        }

        let open = Imgui::begin_window(&instance.name, true);
        Imgui::init_state(ui.api);

        let pos = ui.get_window_pos();
        let size = ui.get_window_size();

        if Self::is_inside(mouse, pos, size) && show_context_menu {
            Imgui::mark_show_popup(ui.api, true);
        } else {
            Imgui::mark_show_popup(ui.api, false);
        }

        // Make sure we move the cursor to the start of the stream here
        ReaderWrapper::reset_reader(&mut session.reader);

        unsafe {
            let plugin_funcs = instance.plugin_type.plugin_funcs as *mut CViewCallbacks;
            ((*plugin_funcs).update.unwrap())(instance.plugin_data,
                                                ui.api as *mut c_void,
                                                session.reader.api as *mut c_void,
                                                session.get_current_writer().api as *mut c_void);
        }

        let has_shown_menu = Imgui::has_showed_popup(ui.api);

        Imgui::end_window();

        WindowState {
            showed_popup: has_shown_menu,
            should_close: !open,
        }
    }

    pub fn remove_views(&mut self, view_plugins: &mut ViewPlugins, views: &Vec<ViewHandle>) {
        for view in views {
            view_plugins.destroy_instance(*view);
            self.ws.delete_by_handle(DockHandle(view.0));
        }
    }

    fn update_mouse_state(&mut self, mouse_pos: (f32, f32)) {
        match self.mouse_state.state {
            State::Default => {
                if let Some(h) = self.ws.is_hovering_sizer(mouse_pos) {
                    if h.1 == Direction::Vertical {
                        Bgfx::cursor_set_type(2);
                    } else {
                        Bgfx::cursor_set_type(1);
                    }

                    if self.win.get_mouse_down(MouseButton::Left) {
                        self.mouse_state.handle = Some(h);
                        self.mouse_state.state = State::DraggingSlider;
                    }
                } else {
                    Bgfx::cursor_set_type(0);
                }
            },

            State::DraggingSlider => {
                let pm = self.mouse_state.prev_mouse;
                let delta = (pm.0 - mouse_pos.0, pm.1 - mouse_pos.1);

                if self.win.get_mouse_down(MouseButton::Left) {
                    let t = self.mouse_state.handle.unwrap();
                    if t.1 == Direction::Vertical {
                        Bgfx::cursor_set_type(2);
                    } else {
                        Bgfx::cursor_set_type(1);
                    }
                    self.ws.drag_sizer(t.0, delta);
                } else {
                    self.mouse_state.handle = None;
                    self.mouse_state.state = State::Default;
                    Bgfx::cursor_set_type(0);
                }
            }
        }

        /*
        if let Some(handle) = ws.is_hovering_sizer(mouse_pos) {
            let delta = (prev_mouse.0 - mouse_pos.0, prev_mouse.1 - mouse_pos.1);

            if window.get_mouse_down(MouseButton::Left) {
                println!("drangging sizer");
                ws.drag_sizer(handle, delta);
            }
        }
        */

        self.mouse_state.prev_mouse = mouse_pos;
    }

    pub fn update(&mut self, sessions: &mut Sessions, view_plugins: &mut ViewPlugins) {
        let mut views_to_delete = Vec::new();
        let mut has_shown_menu = 0u32;

        self.win.update();
        self.ws.update();

        let mouse = self.win.get_mouse_pos(MouseMode::Clamp).unwrap_or((0.0, 0.0));

        self.update_mouse_state(mouse);

        Bgfx::set_mouse_pos(mouse);
        Bgfx::set_mouse_state(0, self.win.get_mouse_down(MouseButton::Left));

        let show_context_menu = self.win.get_mouse_down(MouseButton::Right);

        for view in &self.views {
            if let Some(ref mut v) = view_plugins.get_view(*view) {
                if let Some(ref mut s) = sessions.get_session(v.session_handle) {
                    let state = Self::update_view(self, v, s, show_context_menu, mouse);

                    if state.should_close {
                        views_to_delete.push(*view);
                    }

                    has_shown_menu |= state.showed_popup;
                }
            }
        }

        if self.win.is_key_pressed(Key::Down, KeyRepeat::No) {
            self.ws.dump_tree();
        }

        if self.win.is_key_pressed(Key::Up, KeyRepeat::No) {
            let _ = self.ws.save("/Users/danielcollin/code/temp/test.xml");
        }

        if self.win.is_key_pressed(Key::Right, KeyRepeat::No) {
            let ws = Workspace::load("/Users/danielcollin/code/temp/test.xml").unwrap();
            let docks = ws.get_docks();
            self.views.clear();

            for dock in &docks {
                //println!("create stuff... {} - {}", dock.name, dock.handle.0);
                let ui = Imgui::create_ui_instance();
                let handle = ViewHandle(dock.handle.0);
                if dock.name == "" {
                    view_plugins.create_instance_with_handle(ui, &"Bitmap View".to_owned(), SessionHandle(0), ViewHandle(dock.handle.0));
                } else {
                    view_plugins.create_instance_with_handle(ui, &dock.name, SessionHandle(0), ViewHandle(dock.handle.0));
                }
                self.views.push(handle);
            }

            self.ws = ws;
        }

        // if now plugin has showed a menu we do it here
        // TODO: Handle diffrent cases when attach menu on to plugin menu or not

        if has_shown_menu == 0 && show_context_menu {
            Self::show_popup(self, true, mouse, view_plugins);
        } else {
            Self::show_popup(self, false, mouse, view_plugins);
        }

        Self::remove_views(self, view_plugins, &views_to_delete);
    }

    fn split_view(&mut self, name: &String, view_plugins: &mut ViewPlugins, pos: (f32, f32), direction: Direction) {
        let ui = Imgui::create_ui_instance();
        if let Some(handle) = view_plugins.create_instance(ui, name, SessionHandle(0)) {
            if let Some(dock_handle) = self.ws.get_hover_dock(pos) {
                let new_handle = DockHandle(handle.0);
                self.ws.split_by_dock_handle(direction, dock_handle, new_handle);
                self.ws.set_name_to_handle(name, new_handle);
                //self.ws.dump_tree_linear();
            } else {
                self.ws.new_split(DockHandle(handle.0), direction);
                //println!("no split");
            }

            self.views.push(handle);
        }
    }

    fn show_popup(&mut self, show: bool, mouse_pos: (f32, f32), view_plugins: &mut ViewPlugins) {
        let ui = Imgui::get_ui();

        if show {
            ui.open_popup("plugins");
        }

        if ui.begin_popup("plugins") {
            let plugin_names = view_plugins.get_plugin_names();

            if ui.begin_menu("Split Horizontally", true) {
                for name in &plugin_names {
                    if ui.menu_item(name, false, true) {
                        Self::split_view(self, &name, view_plugins, mouse_pos, Direction::Horizontal);
                    }
                }
                ui.end_menu();
            }

            if ui.begin_menu("Split Vertically", true) {
                for name in &plugin_names {
                    if ui.menu_item(name, false, true) {
                        Self::split_view(self, &name, view_plugins, mouse_pos, Direction::Vertical);
                    }
                }
                ui.end_menu();
            }

            ui.end_popup();
        }
    }
}
