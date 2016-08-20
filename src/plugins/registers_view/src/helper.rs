use prodbg_api::Ui;

/// Returns index if character under mouse cursor. Assumes one-line text and monospace font.
pub fn get_text_cursor_index(ui: &Ui, len: usize) -> usize {
    let width = ui.get_item_rect_size().x;
    let left_border = ui.get_item_rect_min().x;
    let mouse = ui.get_mouse_pos().x;
    let space_per_symbol = width / len as f32;
    if mouse < left_border {
        return 0;
    }
    let res = ((mouse - left_border) / space_per_symbol) as usize;
    if res > len - 1 {
        return len - 1;
    }
    return res;
}
