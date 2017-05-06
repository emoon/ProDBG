use std::ffi::CString;

use ffi_gen::*;

use traits_gen::*;

pub struct PushButton {
    pub widget_funcs: *const PUWidgetFuncs,
    pub funcs: *const PUPushButtonFuncs,
    pub obj: *const PUPushButton,
}

impl PushButton {
    pub fn set_title(&self, text: &str) {
        let str_in_0 = CString::new(text).unwrap();
        unsafe {
            ((*self.funcs).set_title)(self.obj, str_in_0.as_ptr())
        }
    }

    #[inline]
    pub fn get_obj(&self) -> *const PUPushButton { self.obj }
}

impl Widget for PushButton {
   fn get_obj(&self) -> *const PUWidget {
       unsafe { (*self.obj).base }
   }
   fn get_funcs(&self) -> *const PUWidgetFuncs {
       self.widget_funcs
   }
}

