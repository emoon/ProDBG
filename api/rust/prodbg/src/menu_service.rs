use libc::{c_char, c_uint, c_void};
use CFixedString;

pub struct PDMenuHandle(pub u64);
pub struct PDMenuItem(pub u64);

/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#[repr(C)]
pub struct CMenuFuncs1 {
    pub create_menu: fn(title: *const c_char) -> *mut c_void,
    pub destroy_menu: fn(handle: *mut c_void),
    pub add_sub_menu: fn(name: *const c_char, parent: u64, child: u64),

    pub add_menu_item: fn(menu: *mut c_void,
                          name: *const c_char,
                          id: c_uint,
                          key: c_uint,
                          modifier: c_uint)
                          -> PDMenuItem,
    pub remove_menu_item: fn(item: u64),
}

pub struct MenuFuncs {
    pub api: *mut CMenuFuncs1,
}

impl MenuFuncs {
    pub fn create_menu(&mut self, name: &str) -> *mut c_void {
        let mut title = CFixedString::from_str(name);
        unsafe { ((*self.api).create_menu)(title.as_ptr()) }
    }

    pub fn destroy_menu(&mut self, handle: *mut c_void) {
        unsafe { ((*self.api).destroy_menu)(handle) }
    }

    pub fn add_menu_item(&mut self,
                         item: *mut c_void,
                         name: &str,
                         id: usize,
                         key: u32,
                         modifier: u32)
                         -> PDMenuItem {
        let mut title = CFixedString::from_str(name);
        unsafe { ((*self.api).add_menu_item)(item, title.as_ptr(), id as c_uint, key, modifier) }
    }
}
