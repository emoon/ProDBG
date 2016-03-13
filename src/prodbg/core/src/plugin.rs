///!
///! Standard plugin is of the type of view plugins and backend plugins
///! which follows the same structure in the shared libs
///!

use dynamic_reload::Lib;
use libc::{c_char, c_void};
use std::rc::Rc;
use std::mem::transmute;
use std::ffi::CStr;

#[repr(C)]
pub struct CBasePlugin {
    pub name: *const c_char,
}

pub struct Plugin {
    pub lib: Rc<Lib>,
    pub name: String,
    pub type_name: String,
    pub plugin_funcs: *mut CBasePlugin,
}

impl Plugin {
    pub fn new(lib: &Rc<Lib>, plugin_type: *const c_char, plugin: *mut c_void) -> Plugin {
        unsafe {
            let plugin_funcs: *mut CBasePlugin = transmute(plugin);

            Plugin {
                lib: lib.clone(),
                type_name: CStr::from_ptr(plugin_type).to_string_lossy().into_owned(),
                name: CStr::from_ptr((*plugin_funcs).name).to_string_lossy().into_owned(),
                plugin_funcs: plugin_funcs,
            }
        }
    }
}

#[cfg(test)]
mod tests {
}
