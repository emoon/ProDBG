//! Module to map and pass keys from Minifb to ImGui.

use imgui_sys::Imgui;
use prodbg_api::ui_ffi::ImguiKey;
use minifb::{InputCallback, Key};


/// Passes input characters from Minifb to ImGui
pub struct KeyCharCallback;

impl InputCallback for KeyCharCallback {
    fn add_char(&mut self, key: u32) {
        Imgui::add_input_character(key as u16);
    }
}

pub fn setup_imgui_key_mappings() {
    Imgui::map_key(ImguiKey::Tab as usize, Key::Tab as usize);
    Imgui::map_key(ImguiKey::LeftArrow as usize, Key::Left as usize);
    Imgui::map_key(ImguiKey::RightArrow as usize, Key::Right as usize);
    Imgui::map_key(ImguiKey::DownArrow as usize, Key::Down as usize);
    Imgui::map_key(ImguiKey::UpArrow as usize, Key::Up as usize);
    Imgui::map_key(ImguiKey::PageUp as usize, Key::PageUp as usize);
    Imgui::map_key(ImguiKey::PageDown as usize, Key::PageDown as usize);
    Imgui::map_key(ImguiKey::Home as usize, Key::Home as usize);
    Imgui::map_key(ImguiKey::End as usize, Key::End as usize);
    Imgui::map_key(ImguiKey::Delete as usize, Key::Delete as usize);
    Imgui::map_key(ImguiKey::Backspace as usize, Key::Backspace as usize);
    Imgui::map_key(ImguiKey::Enter as usize, Key::Enter as usize);
    Imgui::map_key(ImguiKey::Escape as usize, Key::Escape as usize);
    Imgui::map_key(ImguiKey::A as usize, Key::A as usize);
    Imgui::map_key(ImguiKey::C as usize, Key::C as usize);
    Imgui::map_key(ImguiKey::V as usize, Key::V as usize);
    Imgui::map_key(ImguiKey::X as usize, Key::X as usize);
    Imgui::map_key(ImguiKey::Y as usize, Key::Y as usize);
    Imgui::map_key(ImguiKey::Z as usize, Key::Z as usize);
}
