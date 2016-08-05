//! Adds key handling implementation to `Window`

use super::Window;
use minifb::{Key, KeyRepeat};
use imgui_sys::Imgui;
use core::view_plugins::ViewPlugins;

impl Window {
    pub fn update_imgui_keys(&mut self) {
        Imgui::clear_keys();

        self.win.get_keys_pressed(KeyRepeat::Yes).map(|keys| {
            for k in keys {
                Imgui::set_key_down(k as usize);
            }
        });
    }

    pub fn process_key_presses(&mut self, view_plugins: &mut ViewPlugins) {
        if self.win.is_key_pressed(Key::Z, KeyRepeat::No) {
            self.undo_workspace_change(view_plugins);
        }

        if self.win.is_key_pressed(Key::X, KeyRepeat::No) {
            self.redo_workspace_change(view_plugins);
        }
    }
}
