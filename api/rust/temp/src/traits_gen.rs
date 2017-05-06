use ::ffi_gen;

pub trait Widget {
    fn set_size(&self, width: i32, height: i32) {
        unsafe {
            let obj = self.get_obj();
            let funcs = self.get_funcs();
            ((*funcs).set_size)(obj, width, height)
        }
    }

    fn show(&self) {
        unsafe {
            let obj = self.get_obj();
            let funcs = self.get_funcs();
            ((*funcs).show)(obj)
        }
    }

    fn get_obj(&self) -> *const ffi_gen::PUWidget;
    fn get_funcs(&self) -> *const ffi_gen::PUWidgetFuncs;
}

