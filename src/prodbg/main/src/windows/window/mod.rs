mod menus;
mod mouse;
mod keys;
mod popup;
mod layout;

use minifb::{self, Scale, WindowOptions};
use core::view_plugins::{ViewHandle, ViewPlugins};
use core::backend_plugin::{BackendHandle, BackendPlugins};
use core::session::{Session, SessionHandle, Sessions};
use core::reader_wrapper::ReaderWrapper;
use super::viewdock::{Direction, DockHandle, ItemTarget, Rect, Workspace};
use std::fs::File;
use std::io;
use menu::Menu;
use imgui_sys::Imgui;
use prodbg_api::ui_ffi::PDVec2;
use prodbg_api::view::CViewCallbacks;
use prodbg_api::backend::CBackendCallbacks;
use std::os::raw::c_void;
use std::collections::VecDeque;
use std::io::{Read, Write};
use statusbar::Statusbar;
use prodbg_api::events;
use self::mouse::MouseState;
use self::popup::ViewRenameState;
use self::layout::{PluginInstanceInfo, WindowLayout};
use std::collections::HashMap;


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

fn restore_view_plugins(docks: &[DockHandle],
                        view_plugins: &mut ViewPlugins,
                        info: &mut HashMap<u64, PluginInstanceInfo>) {
    for dock in docks.iter() {
        if !view_plugins.get_view(ViewHandle(dock.0)).is_some() {
            let info = match info.remove(&dock.0) {
                None => panic!("Could not restore view: no info in `removed_instances` found"),
                Some(info) => info,
            };
            if info.restore(view_plugins).is_none() {
                panic!("Could not restore view");
            }
        }
    }
}


pub struct Window {
    /// minifb window
    pub win: minifb::Window,
    pub menu: Menu,

    pub ws: Workspace,
    // TODO: should we serialize Workspace if this is stored in memory only?
    ws_states: VecDeque<String>,
    cur_state_index: usize,
    removed_instances: HashMap<u64, PluginInstanceInfo>,

    pub mouse_state: MouseState,

    pub menu_id_offset: u32,

    pub overlay: Option<(DockHandle, Rect)>,
    pub context_menu_data: Option<(DockHandle, (f32, f32))>,

    pub statusbar: Statusbar,
    pub custom_menu_height: f32,

    /// Backend that is currently being configured.
    pub config_backend: Option<BackendHandle>,

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

        let ws_states = VecDeque::with_capacity(WORKSPACE_UNDO_LIMIT);
        let mut res = Window {
            win: win,
            menu: Menu::new(),
            menu_id_offset: 1000,
            mouse_state: MouseState::new(),
            ws: ws,
            ws_states: ws_states,
            cur_state_index: 0usize,
            removed_instances: HashMap::new(),
            overlay: None,
            context_menu_data: None,
            statusbar: Statusbar::new(),
            custom_menu_height: 0.0,
            config_backend: None,
            view_rename_state: ViewRenameState::None,
        };

        res.initialize_workspace_state();

        Ok(res)
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

		// If we have a backend configuration running
        self.update_backend_configure(backend_plugins);

        // Status bar needs full size of window
        let win_size = self.win.get_size();

        self.update_statusbar(sessions, backend_plugins, win_size);

        let width = win_size.0 as f32;
        let height = (win_size.1 as f32) - self.statusbar.get_size() - self.custom_menu_height;
        // Workspace needs area without menus and status bar
        self.ws.update_rect(Rect::new(0.0, self.custom_menu_height, width, height));

        let mut views_to_delete = Vec::new();
        let mut has_shown_menu = 0u32;
        let show_context_menu = self.update_mouse_state();
        let mouse = self.get_mouse_pos();
        let docks = self.ws.get_docks();
        for dock in docks {
            let view_handle = ViewHandle(dock.0);
            let session = match view_plugins.get_view(view_handle)
                .and_then(|v| sessions.get_session(v.session_handle)) {
                None => continue,
                Some(s) => s,
            };
            let state = Self::update_view(&mut self.ws,
                                          view_plugins,
                                          view_handle,
                                          session,
                                          show_context_menu,
                                          mouse,
                                          &self.overlay);

            if state.should_close {
                views_to_delete.push(view_handle);
            }
            has_shown_menu |= state.showed_popup;
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

    /// Updates the statusbar at the bottom of the window to show which state the debugger currently is in
    fn update_statusbar(&self, sessions: &mut Sessions, backend_plugins: &mut BackendPlugins, size: (usize, usize)) {
    	let session = sessions.get_current();

        if let Some(ref backend) = backend_plugins.get_backend(session.backend) {
        	let name = &backend.plugin_type.name;
        	self.statusbar.update(&name, size);
		} else {
        	self.statusbar.update("", size);
		}
    }

    pub fn save_layout(&mut self,
                       filename: &str,
                       view_plugins: &mut ViewPlugins)
                       -> io::Result<()> {
        let mut file = try!(File::create(filename));
        let layout = WindowLayout::from_current_state(self.ws.clone(), view_plugins);
        let state = layout.to_string();
        println!("writing state to disk");
        file.write_all(state.as_str().as_bytes())
    }

    pub fn load_layout(&mut self,
                       filename: &str,
                       view_plugins: &mut ViewPlugins)
                       -> io::Result<()> {
        let mut data = String::new();

        let mut file = try!(File::open(filename));
        try!(file.read_to_string(&mut data));

        let layout = match WindowLayout::from_string(&data) {
            Ok(layout) => layout,
            Err(error) => return Result::Err(io::Error::new(io::ErrorKind::InvalidData, error)),
        };
        self.ws = layout.workspace;
        // TODO: should we check here that handles stored in Workspace and handles restored in
        // ViewPlugins are the same?
        WindowLayout::restore_view_plugins(view_plugins, &layout.infos);
        self.initialize_workspace_state();
        Ok(())
    }

    fn update_view(ws: &mut Workspace,
                   view_plugins: &mut ViewPlugins,
                   handle: ViewHandle,
                   session: &mut Session,
                   show_context_menu: bool,
                   mouse: (f32, f32),
                   overlay: &Option<(DockHandle, Rect)>)
                   -> WindowState {

        let ws_container = match ws.root_area
            .as_mut()
            .and_then(|root| root.get_container_by_dock_handle_mut(DockHandle(handle.0))) {

            None => {
                panic!("Tried to update view {} but it is not in workspace",
                       handle.0)
            }
            Some(container) => container,
        };

        if ws_container.docks[ws_container.active_dock].0 != handle.0 {
            // This view is in hidden tab
            return WindowState {
                showed_popup: 0,
                should_close: false,
            };
        }

        let tab_names: Vec<String> = ws_container.docks
            .iter()
            .map(|dock_handle| {
                view_plugins.get_view(ViewHandle(dock_handle.0))
                    .map(|plugin| plugin.name.clone())
                    .unwrap_or("Not loaded".to_string())
            })
            .collect();

        let instance = match view_plugins.get_view(handle) {
            None => {
                return WindowState {
                    showed_popup: 0,
                    should_close: false,
                }
            }
            Some(instance) => instance,
        };

        Imgui::set_window_pos(ws_container.rect.x, ws_container.rect.y);
        Imgui::set_window_size(ws_container.rect.width, ws_container.rect.height);
        // TODO: should we avoid repeating window names? Add handle or something like this.
        let open = Imgui::begin_window(&instance.name, true);

        if tab_names.len() > 1 {
            Imgui::begin_window_child("tabs", 20.0);
            let mut borders = Vec::with_capacity(tab_names.len());
            // TODO: should repeated window names be avoided?
            for (i, name) in tab_names.iter().enumerate() {
                if Imgui::tab(name,
                              i == ws_container.active_dock,
                              i == tab_names.len() - 1) {
                    ws_container.active_dock = i;
                }
                borders.push(Imgui::tab_pos());
            }
            ws_container.update_tab_borders(&borders);
            Imgui::end_window_child();
            Imgui::separator();
            Imgui::begin_window_child("body", 0.0);
        }

        let ui = &instance.ui;
        Imgui::init_state(ui.api);

        let pos = ui.get_window_pos();
        let size = ui.get_window_size();

        Imgui::mark_show_popup(ui.api, is_inside(mouse, pos, size) && show_context_menu);

        // Draw drag zone
        if let &Some((handle, rect)) = overlay {
            if handle.0 == handle.0 {
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

        if tab_names.len() > 1 {
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
            if let Some(instance) = view_plugins.get_view(*view) {
                self.removed_instances.insert(view.0, PluginInstanceInfo::new(instance));
            }
            view_plugins.destroy_instance(*view);
            self.ws.delete_dock_by_handle(DockHandle(view.0));
        }
    }

    fn update_backend_configure(&mut self, backend_plugins: &mut BackendPlugins) {
    	if self.config_backend == None {
    		return;
    	}

    	let backend = backend_plugins.get_backend(self.config_backend).unwrap();

        unsafe {
            let plugin_funcs = backend.plugin_type.plugin_funcs as *mut CBackendCallbacks;
            if let Some(show_config) = (*plugin_funcs).show_config {
            	let ui = Imgui::get_ui();
            	ui.open_popup("config");
            	if ui.begin_popup_modal("config") {
            		show_config(backend.plugin_data, Imgui::get_ui_funs() as *mut c_void);

            		if ui.button("Ok", Some(PDVec2 { x: 120.0, y: 0.0 })) {
            			self.config_backend = None;
            			ui.close_current_popup();
            		}

            		ui.same_line(0, -1);

            		if ui.button("Cancel", Some(PDVec2 { x: 120.0, y: 0.0 })) {
            			self.config_backend = None;
            			ui.close_current_popup();
            		}

					ui.end_popup();
            	}
            }
        }
    }

    fn has_source_code_view(&self, view_plugins: &mut ViewPlugins) -> bool {
        // TODO: Use setting for this name
        for handle in self.ws.get_docks() {
            if let Some(plugin) = view_plugins.get_view(ViewHandle(handle.0)) {
                if plugin.name == "Source Code View" {
                    return true;
                }
            }
        }

        false
    }

    fn open_source_file(&mut self,
                        filename: &str,
                        view_plugins: &mut ViewPlugins,
                        session: &mut Session) {
        // check if we already have a source view open and just post the message.
        if !self.has_source_code_view(view_plugins) {
            let mouse = self.get_mouse_pos();
            // This is somewhat hacky to set a "correct" split view for
            self.context_menu_data = self.ws
                .get_dock_handle_at_pos(mouse)
                .map(|handle| (handle, mouse));
            self.split_view(&"Source Code View".to_owned(),
                            view_plugins,
                            Direction::Vertical);
        }

        let writer = session.get_current_writer();
        writer.event_begin(events::EVENT_SET_SOURCE_CODE_FILE as u16);
        writer.write_string("filename", filename);
        writer.event_end();
    }

    fn initialize_workspace_state(&mut self) {
        self.ws_states.clear();
        self.ws_states.push_back(self.ws.save_state());
    }

    fn restore_workspace_state(&mut self, view_plugins: &mut ViewPlugins) {
        // workspace will recalculate dock areas on the next update
        let docks_before_restore = self.ws.get_docks();
        self.ws = Workspace::from_state(&self.ws_states[self.cur_state_index]).unwrap();
        let docks = self.ws.get_docks();
        let views_to_delete: Vec<ViewHandle> = docks_before_restore.iter()
            .filter(|&dock_before| !docks.iter().any(|dock| dock_before == dock))
            .map(|dock_before| ViewHandle(dock_before.0))
            .collect();
        Self::remove_views(self, view_plugins, &views_to_delete);
        restore_view_plugins(&docks, view_plugins, &mut self.removed_instances);
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

    fn split_view(&mut self,
                  plugin_name: &str,
                  view_plugins: &mut ViewPlugins,
                  direction: Direction) {
        let ui = Imgui::create_ui_instance();
        if let Some(handle) =
               view_plugins.create_instance(ui, plugin_name, None, None, SessionHandle(0), None) {
            let new_dock = DockHandle(handle.0);
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
        }
    }

    fn tab_view(&mut self, plugin_name: &str, view_plugins: &mut ViewPlugins) {
        let ui = Imgui::create_ui_instance();
        if let Some(handle) =
               view_plugins.create_instance(ui, plugin_name, None, None, SessionHandle(0), None) {

            let mut should_save_ws = false;
            if let Some((src_dock_handle, _)) = self.context_menu_data {
                if let Some(ref mut root) = self.ws.root_area {
                    if let Some(ref mut container) =
                           root.get_container_by_dock_handle_mut(src_dock_handle) {
                        container.append_dock(DockHandle(handle.0));
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
