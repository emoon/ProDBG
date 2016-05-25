use libc::{c_char, c_uint, c_void};
use CFixedString;

pub struct PDMenuHandle(pub u64);
pub struct PDMenuItem(pub u64);

#[repr(C)]
pub struct CMenuFuncs1 {
    pub private_data: *mut c_void,
    pub create_menu: fn(priv_data: *mut c_void, title: *const c_char) -> *mut c_void,
    pub destroy_menu: fn(priv_data: *mut c_void, handle: *mut c_void),
    pub add_sub_menu: fn(priv_data: *mut c_void,
                         name: *const c_char,
                         parent: u64,
                         child: u64),

    pub add_menu_item: fn(priv_data: *mut c_void,
                          menu: *mut c_void,
                          name: *const c_char,
                          id: c_uint,
                          key: c_uint,
                          modifier: c_uint)
                          -> PDMenuItem,
    pub remove_menu_item: fn(priv_data: *mut c_void, item: u64),
}

pub struct MenuFuncs {
    pub api: *mut CMenuFuncs1,
}

impl MenuFuncs {
    pub fn create_menu(&mut self, name: &str) -> *mut c_void {
        let title = CFixedString::from_str(name);
        unsafe {
            let data = (*self.api).private_data;
            ((*self.api).create_menu)(data, title.as_ptr())
        }
    }

    pub fn destroy_menu(&mut self, handle: *mut c_void) {
        unsafe {
            let data = (*self.api).private_data;
            ((*self.api).destroy_menu)(data, handle)
        }
    }

    pub fn add_menu_item(&mut self,
                         item: *mut c_void,
                         name: &str,
                         id: usize,
                         key: u32,
                         modifier: u32)
                         -> PDMenuItem {
        unsafe {
            let data = (*self.api).private_data;
            let title = CFixedString::from_str(name);
            ((*self.api).add_menu_item)(data, item, title.as_ptr(), id as c_uint, key, modifier)
        }
    }
}
