use std::ptr;
use ui_ffi::*;

use CFixedString;

pub struct Ui {
    pub api: *mut CPdUI,
}

impl Ui {
    pub fn set_title(&self, title: &str) {
        unsafe {
            let t = CFixedString::from_str(title).as_ptr();
            ((*self.api).set_title)((*self.api).private_data, t);
        }
    }

    #[inline]
    pub fn get_window_size(&self) -> PDVec2 {
        unsafe { ((*self.api).get_window_size)() }
    }

    #[inline]
    pub fn get_window_pos(&self) -> PDVec2 {
        unsafe { ((*self.api).get_window_pos)() }
    }

    pub fn begin_child(&self, id: &str, pos: Option<PDVec2>, border: bool, flags: u32) {
        unsafe {
            let t = CFixedString::from_str(id).as_ptr();
            match pos {
                Some(p) => ((*self.api).begin_child)(t, p, border as i32, flags as i32),
                None => {
                    ((*self.api).begin_child)(t,
                                              PDVec2 { x: 0.0, y: 0.0 },
                                              border as i32,
                                              flags as i32)
                }
            }
        }
    }

    #[inline]
    pub fn end_child(&self) {
        unsafe { ((*self.api).end_child)() }
    }

    #[inline]
    pub fn get_scroll_y(&self) -> f32 {
        unsafe { ((*self.api).get_scroll_y)() as f32 }
    }

    #[inline]
    pub fn get_scroll_max_y(&self) -> f32 {
        unsafe { ((*self.api).get_scroll_max_y)() as f32 }
    }

    #[inline]
    pub fn set_scroll_y(&self, pos: f32) {
        unsafe { ((*self.api).set_scroll_y)(pos) }
    }

    #[inline]
    pub fn set_scroll_here(&self, center_ratio: f32) {
        unsafe { ((*self.api).set_scroll_here)(center_ratio) }
    }

    #[inline]
    pub fn set_scroll_from_pos_y(&self, pos_y: f32, center_ratio: f32) {
        unsafe { ((*self.api).set_scroll_from_pos_y)(pos_y, center_ratio) }
    }

    #[inline]
    pub fn set_keyboard_focus_here(&self, offset: i32) {
        unsafe { ((*self.api).set_keyboard_focus_here)(offset) }
    }
    
    // TODO: push/pop font

    #[inline]
	pub fn push_style_color(&self, index: usize, color: u32) {
        unsafe { ((*self.api).push_style_color)(index as u32, color) }
    }

    #[inline]
	pub fn pop_style_color(&self, index: usize) {
        unsafe { ((*self.api).pop_style_color)(index as i32) }
    }

    #[inline]
	pub fn push_style_var(&self, index: usize, val: f32) {
        unsafe { ((*self.api).push_style_var)(index as u32, val) }
    }

    #[inline]
	pub fn push_style_var_vec(&self, index: usize, val: PDVec2) {
        unsafe { ((*self.api).push_style_var_vec)(index as u32, val) }
    }

    // Text

    pub fn text(&self, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).text)(t);
        }
    }

    pub fn text_colored(&self, color: u32, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).text_colored)(color, t);
        }
    }

    pub fn text_disabled(&self, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).text_disabled)(t);
        }
    }

    pub fn text_wrapped(&self, text: &str) {
        unsafe {
            let t = CFixedString::from_str(text).as_ptr();
            ((*self.api).text_wrapped)(t);
        }
    }

	pub fn columns(&self, count: isize, id: Option<&str>, border: bool) {
	    unsafe {
            match id {
                Some(p) => {
                    let t = CFixedString::from_str(p).as_ptr();
                    ((*self.api).columns)(count as i32, t, border as i32)
                }
                None => ((*self.api).columns)(count as i32, ptr::null(), border as i32),
            }
        }
    }

    #[inline]
    pub fn next_column(&self) {
        unsafe { ((*self.api).next_column)() }
    }

    pub fn button(&self, title: &str, pos: Option<PDVec2>) -> bool {
        unsafe {
            let t = CFixedString::from_str(title).as_ptr();
            match pos {
                Some(p) => ((*self.api).button)(t, p) != 0,
                None => ((*self.api).button)(t, PDVec2 { x: 0.0, y: 0.0 }) != 0,
            }
        }
    }
}

