//! Implements popup handling functions for `Window`

use super::Window;
use core::view_plugins::{ViewPlugins, ViewHandle};
use core::session::SessionHandle;
use super::super::viewdock::{Direction, DockHandle};
use imgui_sys::Imgui;
use prodbg_api::{Ui, PDUIINPUTTEXTFLAGS_ENTERRETURNSTRUE, PDUIINPUTTEXTFLAGS_AUTOSELECTALL};

/// Current state of "Rename view" popup
pub enum ViewRenameState {
    None,
    Init(DockHandle),
    Showing(DockHandle, Box<[u8; 100]>),
}

enum ViewRenameRenderResult {
    Showing,
    Accepted,
    Canceled,
}

impl Window {
    pub fn render_popup(&mut self, should_open: bool, view_plugins: &mut ViewPlugins) {
        let ui = Imgui::get_ui();

        if should_open {
            ui.open_popup("plugins");
        }

        if ui.begin_popup("plugins") {
            let plugin_names = view_plugins.get_plugin_names();

            if self.ws.root_area.is_none() {
                self.render_new_view_menu(&plugin_names, view_plugins);
            } else {
                self.render_popup_regular(&plugin_names, view_plugins);
            }

            ui.end_popup();
        }
        self.render_view_rename(view_plugins);
    }

    fn render_new_view_menu(&mut self, names: &[String], view_plugins: &mut ViewPlugins) {
        let ui = Imgui::get_ui();

        if ui.begin_menu("New View", true) {
            for name in names {
                if ui.menu_item(name, false, true) {
                    Self::split_view(self, &name, view_plugins, Direction::Horizontal);
                }
            }
            ui.end_menu();
        }
    }

    fn render_popup_change_view(&mut self,
                                plugin_names: &Vec<String>,
                                view_plugins: &mut ViewPlugins) {
        let ui = Imgui::get_ui();

        if ui.begin_menu("Change View", true) {
            for name in plugin_names {
                if ui.menu_item(name, false, true) {
                    if let Some((dock_handle, _)) = self.context_menu_data {
                        view_plugins.destroy_instance(ViewHandle(dock_handle.0));
                        view_plugins.create_instance(Imgui::create_ui_instance(),
                                                     &name,
                                                     None,
                                                     None,
                                                     SessionHandle(0),
                                                     Some(ViewHandle(dock_handle.0)));
                    }
                }
            }
            ui.end_menu();
        }
    }

    fn render_popup_regular(&mut self,
                            plugin_names: &Vec<String>,
                            view_plugins: &mut ViewPlugins) {
        let ui = Imgui::get_ui();

        self.render_popup_change_view(plugin_names, view_plugins);

        if ui.begin_menu("Split Horizontally", true) {
            for name in plugin_names {
                if ui.menu_item(name, false, true) {
                    self.split_view(&name, view_plugins, Direction::Horizontal);
                }
            }
            ui.end_menu();
        }

        if ui.begin_menu("Split Vertically", true) {
            for name in plugin_names {
                if ui.menu_item(name, false, true) {
                    self.split_view(&name, view_plugins, Direction::Vertical);
                }
            }
            ui.end_menu();
        }

        if ui.begin_menu("Tab", true) {
            for name in plugin_names {
                if ui.menu_item(name, false, true) {
                    self.tab_view(&name, view_plugins);
                }
            }
            ui.end_menu();
        }

        if ui.menu_item("Rename", false, true) {
            if let Some(handle) = self.context_menu_data.map(|(handle, _)| handle) {
                self.view_rename_state = ViewRenameState::Init(handle);
            }
        }
    }

    fn render_view_rename_popup(ui: &Ui,
                                set_focus: bool,
                                buf: &mut [u8])
                                -> ViewRenameRenderResult {
        let mut res = ViewRenameRenderResult::Showing;
        if ui.begin_popup("##name_input_popup") {
            if set_focus {
                ui.set_keyboard_focus_here(0);
            }
            if ui.input_text("##name_input",
                             buf.as_mut(),
                             PDUIINPUTTEXTFLAGS_ENTERRETURNSTRUE |
                             PDUIINPUTTEXTFLAGS_AUTOSELECTALL,
                             None) {
                res = ViewRenameRenderResult::Accepted;
            }
            ui.end_popup();
        } else {
            res = ViewRenameRenderResult::Canceled;
        }
        res
    }

    fn render_view_rename(&mut self, view_plugins: &mut ViewPlugins) {
        let next_state =
            (|| {
                let handle = match self.view_rename_state {
                    ViewRenameState::None => return None,
                    ViewRenameState::Init(handle) => handle,
                    ViewRenameState::Showing(handle, _) => handle,
                };
                let plugin = match view_plugins.get_view(ViewHandle(handle.0)) {
                    None => return Some(ViewRenameState::None),
                    Some(plugin) => plugin,
                };
                let dock = match self.ws.get_dock_mut(handle) {
                    None => return Some(ViewRenameState::None),
                    Some(dock) => dock,
                };
                let ui = Imgui::get_ui();
                let set_focus = if let ViewRenameState::Init(_) = self.view_rename_state {
                    // TODO: is there a way to avoid allocation of 100 bytes on stack?
                    let mut buf: [u8; 100] = [0; 100];
                    buf[..plugin.name.len()].copy_from_slice(plugin.name.as_bytes());
                    ui.open_popup("##name_input_popup");
                    self.view_rename_state = ViewRenameState::Showing(handle, Box::new(buf));
                    true
                } else {
                    false
                };
                if let ViewRenameState::Showing(_, ref mut buf) = self.view_rename_state {
                    return match Self::render_view_rename_popup(&ui, set_focus, buf.as_mut()) {
                        ViewRenameRenderResult::Showing => None,
                        ViewRenameRenderResult::Canceled => Some(ViewRenameState::None),
                        ViewRenameRenderResult::Accepted => {
                            let null_index = buf.iter().position(|c| *c == 0).unwrap_or(buf.len());
                            if let Ok(parsed) = ::std::str::from_utf8(&buf[..null_index]) {
                                plugin.name = parsed.to_string();
                                dock.name = plugin.name.clone();
                            }
                            Some(ViewRenameState::None)
                        }
                    };
                }
                None
            })();

        if let Some(val) = next_state {
            self.view_rename_state = val
        }
    }
}
