use libc::{c_char};

pub enum ServiceType {
    Message1,
    Capstone1,
}

pub struct Service {
    pub service_func: extern "C" fn(data: *const c_char),
}

impl Service {

}
