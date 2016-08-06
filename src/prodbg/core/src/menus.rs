use std::os::raw::{c_char, c_void, c_uint};
use prodbg_api::{CMenuFuncs1, PDMenuItem};
use std::ffi::CStr;
use std::mem::transmute;
use minifb;

fn create_menu(_priv_data: *mut c_void, title: *const c_char) -> *mut c_void {
    unsafe {
        let name = CStr::from_ptr(title);
        let menu = minifb::Menu::new(name.to_str().unwrap()).unwrap();
        let t = transmute(Box::new(menu));
        t
    }
}

fn destroy_menu(_priv_data: *mut c_void, handle: *mut c_void) {
    let _: Box<minifb::Menu> = unsafe { transmute(handle) };
    // implicitly dropped
}

fn add_menu_item(priv_data: *mut c_void,
                 m: *mut c_void,
                 name: *const c_char,
                 id: c_uint,
                 _key: u32,
                 modifier: u32)
                 -> PDMenuItem {
    unsafe {
        let menu: &mut minifb::Menu = &mut *(m as *mut minifb::Menu);
        let menu_id_offset: u32 = *(priv_data as *mut u32);
        let name = CStr::from_ptr(name);
        // TODO: Fix correct key mapping here
        let item = menu.add_item(name.to_str().unwrap(), (id + menu_id_offset) as usize)
            .shortcut(minifb::Key::Unknown, modifier as usize)
            .build();
        PDMenuItem(item.0)
    }
}

fn add_sub_menu(_priv_data: *mut c_void, name: *const c_char, parent: u64, child: u64) {
    unsafe {
        let name = CStr::from_ptr(name);
        let p: &mut minifb::Menu = &mut *(parent as *mut minifb::Menu);
        let c: &mut minifb::Menu = &mut *(child as *mut minifb::Menu);
        p.add_sub_menu(name.to_str().unwrap(), &c);
    }
}

fn remove_menu_item(_priv_data: *mut c_void, _item: u64) {}

pub fn get_menu_funcs(menu_id_offset: u32) -> CMenuFuncs1 {
    CMenuFuncs1 {
        private_data: unsafe { transmute(Box::new(menu_id_offset)) },
        create_menu: create_menu,
        destroy_menu: destroy_menu,
        add_sub_menu: add_sub_menu,
        add_menu_item: add_menu_item,
        remove_menu_item: remove_menu_item,
    }
}
