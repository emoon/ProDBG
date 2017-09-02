
mod ffi_gen;
use ffi_gen::*;
use std::ffi::CString;

pub struct Rect {
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
}

pub struct Widget {
    obj: *const PUWidget,
}

pub struct PushButton {
    obj: *const PUPushButton,
}

pub struct Slider {
    obj: *const PUSlider,
}

pub struct Painter {
    obj: *const PUPainter,
}

impl Widget {
    pub fn show(&self) {
        unsafe {
            ((*self.obj).show)((*self.obj).privd);
        }
    }

}

impl PushButton {
    pub fn show(&self) {
        unsafe {
            ((*self.obj).show)((*self.obj).privd);
        }
    }

    pub fn set_text(&self, text: &str) {
        let str_in_text_1 = CString::new(text).unwrap();
        unsafe {
            ((*self.obj).set_text)((*self.obj).privd, str_in_text_1.as_ptr());
        }
    }

    pub fn set_flat(&self, flat: bool) {
        unsafe {
            ((*self.obj).set_flat)((*self.obj).privd, flat);
        }
    }

}

impl Slider {
}

impl Painter {
    pub fn draw_line(&self, x1: i32, y1: i32, x2: i32, y2: i32) {
        unsafe {
            ((*self.obj).draw_line)((*self.obj).privd, x1, y1, x2, y2);
        }
    }

}

macro_rules! connect_released {
  ($sender:expr, $data:expr, $call_type:ident) => {
    {
      extern "C" fn temp_call(self_c: *const c_void) {
          unsafe {
              let app = target as *mut $call_type;
              $callback(&mut *app);
          }
      }
      unsafe {
         ((*$sender.obj).connect_released)((*$sender.obj).privd, $data, temp_call);
      }
    }
}}

macro_rules! connect_value_changed {
  ($sender:expr, $data:expr, $call_type:ident) => {
    {
      extern "C" fn temp_call(self_c: *const c_void, value: i32) {
          unsafe {
              let app = target as *mut $call_type;
              $callback(&mut *app, value);
          }
      }
      unsafe {
         ((*$sender.obj).connect_value_changed)((*$sender.obj).privd, $data, temp_call);
      }
    }
}}

pub struct Ui {
    pu: *const PU
}

impl Ui {
    pub fn new(pu: *const PU) -> Ui { Ui { pu: pu } }

    pub fn create_widget(&self) -> Widget {
        Widget { obj: unsafe { ((*self.pu).create_widget)((*self.pu).privd) }}
    }

    pub fn create_push_button(&self) -> PushButton {
        PushButton { obj: unsafe { ((*self.pu).create_push_button)((*self.pu).privd) }}
    }

    pub fn create_slider(&self) -> Slider {
        Slider { obj: unsafe { ((*self.pu).create_slider)((*self.pu).privd) }}
    }

    pub fn create_painter(&self) -> Painter {
        Painter { obj: unsafe { ((*self.pu).create_painter)((*self.pu).privd) }}
    }

}
