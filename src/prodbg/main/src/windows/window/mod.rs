mod menus;
mod mouse;
mod keys;
mod popup;

use minifb::{self, Scale, WindowOptions};
use core::view_plugins::{ViewHandle, ViewPlugins, ViewInstance};
use core::backend_plugin::BackendPlugins;
use core::session::{Sessions, Session, SessionHandle};
use core::reader_wrapper::ReaderWrapper;
use super::viewdock::{self, Workspace, Rect, Direction, DockHandle, Dock, ItemTarget};
use std::fs::File;
use std::io;
use menu::Menu;
use imgui_sys::Imgui;
use prodbg_api::ui_ffi::PDVec2;
use prodbg_api::view::CViewCallbacks;
use std::os::raw::c_void;
use std::collections::VecDeque;
use std::io::{Read, Write};
use statusbar::Statusbar;
use prodbg_api::events;
use self::mouse::MouseState;
use self::popup::ViewRenameState;


const OVERLAY_COLOR: u32 = 0x8000FF00;
const WORKSPACE_UNDO_LIMIT: usize = 10;

struct WindowState {
    pub showed_popup: u32,
    pub should_close: bool,
}

fn is_inside(v: (f32, f32), pos: PDVec2, size: (f32, f32)) -> bool {
    let x0 = pos.x;
    let y0 = pos.y;
    let x1 = pos.x + size.0;
    let y1 = pos.y + size.1;

    v.0 >= x0 && v.0 < x1 && v.1 >= y0 && v.1 < y1
}


pub struct Window {
    /// minifb window
    pub win: minifb::Window,
    pub menu: Menu,

    /// Views in this window
    pub views: Vec<ViewHandle>,

    pub ws: Workspace,
    ws_states: VecDeque<String>,
    cur_state_index: usize,

    pub mouse_state: MouseState,

    pub menu_id_offset: u32,

    pub overlay: Option<(DockHandle, Rect)>,
    pub context_menu_data: Option<(DockHandle, (f32, f32))>,

    pub statusbar: Statusbar,
    pub custom_menu_height: f32,

    /// View currently being renamed
    view_rename_state: ViewRenameState,
}


impl Window {
    pub fn new(width: usize, height: usize) -> minifb::Result<Window> {
        let options = WindowOptions {
           resize: true,
           scale: Scale::X1,
           ..WindowOptions::default()
        };
        let win = try!(minifb::Window::new("ProDBG", width, height, options));
        let ws = Workspace::new(Rect::new(0.0, 0.0, width as f32, (height - 20) as f32));
        let mut ws_states = VecDeque::with_capacity(WORKSPACE_UNDO_LIMIT);

        ws_states.push_back(ws.save_state());

        Ok(Window {
            win: win,
            menu: Menu::new(),
            views: Vec::new(),
            menu_id_offset: 1000,
            mouse_state: MouseState::new(),
            ws: ws,
            ws_states: ws_states,
            cur_state_index: 0usize,
            overlay: None,
            context_menu_data: None,
            statusbar: Statusbar::new(),
            custom_menu_height: 0.0,
            view_rename_state: ViewRenameState::None,
        })
    }

    pub fn pre_update(&mut self) {
        self.update_imgui_mouse();
        self.update_imgui_keys();
    }

    pub fn update(&mut self,
                  sessions: &mut Sessions,
                  view_plugins: &mut ViewPlugins,
                  backend_plugins: &mut BackendPlugins) {

        // Update minifb window to get current window size
        self.win.update();

        // Update menus first to find out size of self-drawn menus (if any)
        self.update_menus(view_plugins, sessions, backend_plugins);

        let win_size = self.win.get_size();
        // Status bar needs full size of window
        self.statusbar.update(win_size);
        let width = win_size.0 as f32;
        let height = (win_size.1 as f32) - self.statusbar.get_size() - self.custom_menu_height;
        // Workspace needs area without menus and status bar
        self.ws.update_rect(Rect::new(0.0, self.custom_menu_height, width, height));

        let mut views_to_delete = Vec::new();
        let mut has_shown_menu = 0u32;
        let show_context_menu = self.update_mouse_state();
        let mouse = self.get_mouse_pos();
        for view in &self.views {
            if let Some(ref mut v) = view_plugins.get_view(*view) {
                if let Some(ref mut s) = sessions.get_session(v.session_handle) {
                    let state = Self::update_view(&mut self.ws,
                                                  v,
                                                  s,
                                                  show_context_menu,
                                                  mouse,
                                                  &self.overlay);

                    if state.should_close {
                        views_to_delete.push(*view);
                    }
                    has_shown_menu |= state.showed_popup;
                }
            }
        }
        if !views_to_delete.is_empty() {
            Self::remove_views(self, view_plugins, &views_to_delete);
            self.save_cur_workspace_state();
        }

        self.process_key_presses(view_plugins);

        // if now plugin has showed a menu we do it here
        // TODO: Handle diffrent cases when attach menu on to plugin menu or not
        self.render_popup(show_context_menu && has_shown_menu == 0, view_plugins);
    }

    pub fn save_layout(&mut self,
                       filename: &str,
                       _view_plugins: &mut ViewPlugins)
        -> io::Result<()> {
            let mut file = try!(File::create(filename));
            let state = self.ws.save_state();
            println!("writing state to disk");
            file.write_all(state.as_str().as_bytes())
        }

    pub fn load_layout(&mut self,
                       filename: &str,
                       view_plugins: &mut ViewPlugins)
        -> io::Result<()> {
            let mut data = "".to_owned();

            let mut file = try!(File::open(filename));
            try!(file.read_to_string(&mut data));

            self.ws = match Workspace::from_state(&data) {
                Ok(ws) => ws,
                Err(error) => return Result::Err(io::Error::new(io::ErrorKind::InvalidData, error)),
            };

            let docks = self.ws.get_docks();

            // TODO: Move this code to separate file and make it generic (copy'n'paste currently)
            for dock in &docks {
                let mut new_view_handles: Vec<ViewHandle> = Vec::new();
                if !self.views.iter().any(|view| view.0 == dock.handle.0) {
                    let ui = Imgui::create_ui_instance();
                    if let Some(handle) = view_plugins.create_instance(ui,
                                                                       &dock.plugin_name,
                                                                       dock.plugin_data.as_ref(),
                                                                       Some(&dock.name),
                                                                       SessionHandle(0),
                                                                       Some(ViewHandle(dock.handle.0))) {
                        new_view_handles.push(handle);
                    } else {
                        println!("Could not load view {}", dock.plugin_name);
                        self.ws.delete_dock_by_handle(dock.handle);
                    }
                }
                self.views.extend(new_view_handles);
            }

            Ok(())
        }

    fn update_view(ws: &mut Workspace,
                   instance: &mut ViewInstance,
                   session: &mut Session,
                   show_context_menu: bool,
                   mouse: (f32, f32),
                   overlay: &Option<(DockHandle, Rect)>)
        -> WindowState {
            let ui = &instance.ui;

            if let Some(ref root) = ws.root_area {
                if let Some(ref container) =
                    root.get_container_by_dock_handle(DockHandle(instance.handle.0)) {
                        if container.docks[container.active_dock].handle.0 != instance.handle.0 {
                            return WindowState {
                                showed_popup: 0,
                                should_close: false,
                            };
                        }
                    }
            }

            if let Some(rect) = ws.get_rect_by_handle(DockHandle(instance.handle.0)) {
                Imgui::set_window_pos(rect.x, rect.y);
                Imgui::set_window_size(rect.width, rect.height);
            }

            let open = Imgui::begin_window(&instance.name, true);

            let mut has_tabs = false;
            if let Some(ref mut root) = ws.root_area {
                if let Some(ref mut container) =
                    root.get_container_by_dock_handle_mut(DockHandle(instance.handle.0)) {
                        let tabs: Vec<String> =
                            container.docks.iter().map(|dock| dock.name.clone()).collect();
                        if tabs.len() > 1 {
                            has_tabs = true;
                            Imgui::begin_window_child("tabs", 20.0);
                            let mut borders = Vec::with_capacity(tabs.len());
                            for (i, t) in tabs.iter().enumerate() {
                                if Imgui::tab(t, i == container.active_dock, i == tabs.len() - 1) {
                                    container.active_dock = i;
                                }
                                borders.push(Imgui::tab_pos());
                            }
                            container.update_tab_borders(&borders);
                            Imgui::end_window_child();
                            Imgui::separator();
                            Imgui::begin_window_child("body", 0.0);
                        }
                    }
            }

            Imgui::init_state(ui.api);

            let pos = ui.get_window_pos();
            let size = ui.get_window_size();

            Imgui::mark_show_popup(ui.api, is_inside(mouse, pos, size) && show_context_menu);

            // Draw drag zone
            if let &Some((handle, rect)) = overlay {
                if handle.0 == instance.handle.0 {
                    Imgui::render_frame(rect.x, rect.y, rect.width, rect.height, OVERLAY_COLOR);
                }
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

            if has_tabs {
                Imgui::end_window_child();
            }
            Imgui::end_window();

            WindowState {
                showed_popup: has_shown_menu,
                should_close: !open,
            }
        }

    pub fn remove_views(&mut self, view_plugins: &mut ViewPlugins, views: &Vec<ViewHandle>) {
        for view in views {
            view_plugins.destroy_instance(*view);
            if let Some(pos) = self.views.iter().position(|v| v == view) {
                self.views.swap_remove(pos);
            }
            self.ws.delete_dock_by_handle(DockHandle(view.0));
        }
    }

    fn has_source_code_view(&self) -> bool {
        // TODO: Use setting for this name
        for dock in self.ws.get_docks() {
            if dock.plugin_name == "Source Code View" {
                return true;
            }
        }

        false
    }

    fn open_source_file(&mut self, filename: &str,
                        view_plugins: &mut ViewPlugins,
                        session: &mut Session) {
        // check if we already have a source view open and just post the message.
        if !self.has_source_code_view() {
            let mouse = self.get_mouse_pos();
            // This is somewhat hacky to set a "correct" split view for
            self.context_menu_data = self.ws.get_dock_handle_at_pos(mouse)
                                         .map(|handle| (handle, mouse));
            self.split_view(&"Source Code View".to_owned(), view_plugins, Direction::Vertical);
        }

        let writer = session.get_current_writer();
        writer.event_begin(events::EVENT_SET_SOURCE_CODE_FILE as u16);
        writer.write_string("filename", filename);
        writer.event_end();
    }

    fn restore_workspace_state(&mut self, view_plugins: &mut ViewPlugins) {
        // workspace will recalculate rects on the next update
        self.ws = Workspace::from_state(&self.ws_states[self.cur_state_index]).unwrap();
        let docks = self.ws.get_docks();
        let views_to_delete: Vec<ViewHandle> = self.views
            .iter()
            .filter(|view| docks.iter().find(|dock| view.0 == dock.handle.0).is_none())
            .map(|view| view.clone())
            .collect();
        Self::remove_views(self, view_plugins, &views_to_delete);

        for dock in &docks {
            let mut new_view_handles: Vec<ViewHandle> = Vec::new();
            if !self.views.iter().find(|view| view.0 == dock.handle.0).is_some() {
                let ui = Imgui::create_ui_instance();
                if let Some(handle) = view_plugins.create_instance(ui,
                                                                   &dock.plugin_name,
                                                                   dock.plugin_data.as_ref(),
                                                                   Some(&dock.name),
                                                                   SessionHandle(0),
                                                                   Some(ViewHandle(dock.handle.0))) {
                    new_view_handles.push(handle);
                } else {
                    panic!("Could not restore view");
                }
            }
            self.views.extend(new_view_handles);
        }
    }

    fn undo_workspace_change(&mut self, view_plugins: &mut ViewPlugins) {
        if self.cur_state_index > 0 {
            self.cur_state_index -= 1;
            self.restore_workspace_state(view_plugins);
        }
    }

    fn redo_workspace_change(&mut self, view_plugins: &mut ViewPlugins) {
        if self.cur_state_index < self.ws_states.len() - 1 {
            self.cur_state_index += 1;
            self.restore_workspace_state(view_plugins);
        }
    }

    fn save_cur_workspace_state(&mut self) {
        let state = self.ws.save_state();
        self.save_workspace_state(state);
    }

    fn save_workspace_state(&mut self, state: String) {
        self.ws_states.drain(self.cur_state_index + 1..);
        if self.cur_state_index == WORKSPACE_UNDO_LIMIT - 1 {
            self.ws_states.pop_front();
            self.cur_state_index -= 1;
        }
        self.ws_states.push_back(state);
        self.cur_state_index += 1;
    }

    fn split_view(&mut self, plugin_name: &str, view_plugins: &mut ViewPlugins, direction: Direction) {
        let ui = Imgui::create_ui_instance();
        if let Some(handle) = view_plugins.create_instance(ui, plugin_name, None, None, SessionHandle(0), None) {
            let name = &view_plugins.get_view(handle).unwrap().name;
            let new_dock = Dock::new(DockHandle(handle.0), name, plugin_name);
            if let Some((dock_handle, pos)) = self.context_menu_data {
                let position = self.ws
                    .get_rect_by_handle(dock_handle)
                    .map(|rect| {
                        let lower_rect = rect.split_by_direction(direction, &[0.5])[0];
                        return if lower_rect.point_is_inside(pos) {
                            0
                        } else {
                            1
                        };
                    })
                .unwrap_or(1);
                self.ws.create_dock_at(ItemTarget::SplitDock(dock_handle, direction, position),
                new_dock);
            } else {
                self.ws.initialize(new_dock);
            }

            self.save_cur_workspace_state();
            self.views.push(handle);
        }
    }

    fn tab_view(&mut self, plugin_name: &str, view_plugins: &mut ViewPlugins) {
        let ui = Imgui::create_ui_instance();
        if let Some(handle) = view_plugins.create_instance(ui, plugin_name, None, None, SessionHandle(0), None) {
            let new_handle = DockHandle(handle.0);
            let name = &view_plugins.get_view(handle).unwrap().name;
            let dock = viewdock::Dock::new(new_handle, name, plugin_name);
            self.views.push(handle);

            let mut should_save_ws = false;
            if let Some((src_dock_handle, _)) = self.context_menu_data {
                if let Some(ref mut root) = self.ws.root_area {
                    if let Some(ref mut container) =
                        root.get_container_by_dock_handle_mut(src_dock_handle) {
                            container.append_dock(dock);
                            should_save_ws = true;
                        }
                }
            }
            if should_save_ws {
                self.save_cur_workspace_state();
            }
        }
    }
}
