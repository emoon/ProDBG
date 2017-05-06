
use ffi_gen::*;
use widgets_gen::*;
pub struct Ui {
    pu: *const PU
}

impl Ui {
    pub fn new(pu: *const PU) -> Ui { Ui { pu: pu } }
    pub fn api_version(&self) -> u64 {
        unsafe {
            (*self.pu).api_version
        }
    }

    pub fn push_button_create(&self) -> PushButton {
        unsafe {
            PushButton {
                widget_funcs: (*self.pu).widget_funcs,
                funcs: (*self.pu).push_button_funcs,
                obj: ((*self.pu).push_button_create)()
           }
        }
    }

}
