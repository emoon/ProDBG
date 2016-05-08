use libc::{c_char, c_uint};
use CFixedString; 

pub struct PDMenuHandle(pub u64);
pub struct PDMenuItem(pub u64);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#[repr(C)]
pub struct CMenuFuncs1 {
    pub create_menu: fn(title: *const c_char) -> u64,
    pub destroy_menu: fn(handle: u64),
    
    pub add_menu_item: fn(menu: u64, name: *const c_char, id: c_uint) -> PDMenuItem,
    pub remove_menu_item: fn(item: u64),

    pub set_flags: fn(item: u64, flags: c_uint),
    pub set_shortcut_key: fn(item: u64, accel_key: c_uint, modifier: c_uint),
}

pub struct MenuFuncs {
    pub api: *mut CMenuFuncs1,
}

impl MenuFuncs {
    pub fn create_menu(&mut self, name: &str) -> PDMenuHandle {
        let mut title = CFixedString::from_str(name);
        unsafe {
            PDMenuHandle(((*self.api).create_menu)(title.as_ptr()))
        }
    }

    pub fn destroy_menu(&mut self, handle: PDMenuHandle) {
        unsafe {
            ((*self.api).destroy_menu)(handle.0)
        }
    }

    pub fn add_menu_item(&mut self, item: PDMenuHandle, name: &str, id: usize) -> PDMenuItem {
        let mut title = CFixedString::from_str(name);
        unsafe {
            ((*self.api).add_menu_item)(item.0, title.as_ptr(), id as c_uint)
        }
    }

    pub fn set_flags(&mut self, item: PDMenuItem, flags: usize) {
        unsafe {
            ((*self.api).set_flags)(item.0, flags as c_uint) 
        }
    }

    pub fn set_shortcut_key(&mut self, item: PDMenuItem, key: usize, modifier: usize) {
        unsafe {
            ((*self.api).set_shortcut_key)(item.0, key as c_uint, modifier as c_uint) 
        }
    }
}


