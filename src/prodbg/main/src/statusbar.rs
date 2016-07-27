//use prodbg_api::backend::Status;
use prodbg_api::{Color, ImGuiCol};
use super::imgui_sys::*;
//use imgui_sys::*;

pub struct Statusbar {
    backend_name: String,
    status: String,
}

impl Statusbar {
    pub fn new() -> Statusbar {
        Statusbar {
            backend_name: "Dummy Backend".to_owned(),
            status: "None".to_owned(), 
        }
    }

    pub fn get_size(&self) -> f32 {
        Imgui::get_ui().get_font_size()
    }

    pub fn update(&self, window_size: (usize, usize)) {
        let ui = Imgui::get_ui();
        let status_size = self.get_size();

        let y_pos = window_size.1 as f32 - status_size; 

        Imgui::set_window_pos(0.0, y_pos);
        Imgui::set_window_size(window_size.0 as f32, status_size); 

        ui.push_style_color(ImGuiCol::WindowBg, Color::from_rgb(50, 30, 40));

        Imgui::begin_window_flags("", true, 
                                      WINDOWFLAGS_NO_TITLE_BAR |
                                      WINDOWFLAGS_NO_COLLAPSE |
                                      WINDOWFLAGS_NO_RESIZE |
                                      WINDOWFLAGS_NO_MOVE);

        // TODO: Remove hard-coded value
        ui.set_cursor_pos((2.0, 0.0));
        ui.text(&format!("{} {}", self.backend_name, self.status));

        Imgui::end_window();

        ui.pop_style_color(1);
    }
}
