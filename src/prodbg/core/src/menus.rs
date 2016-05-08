use libc::{c_char, c_void, c_uint};
use prodbg_api::{CMenuFuncs1, PDMenuItem};
use std::mem::transmute;

static mut MENU_FUNCS: CMenuFuncs1 = CMenuFuncs1 { 
    create_menu: create_menu,
    destroy_menu: destroy_menu,
    add_menu_item: add_menu_item,
    remove_menu_item: remove_menu_item,
    set_flags: set_flags,
    set_shortcut_key: set_shortcut_key,
};

fn create_menu(_title: *const c_char) -> u64 {
    0
}

fn destroy_menu(_handle: u64) {

}
    
fn add_menu_item(_menu: u64, _name: *const c_char, _id: c_uint) -> PDMenuItem {
    PDMenuItem(0)
}

fn remove_menu_item(_item: u64) {

}

fn set_flags(_item: u64, _flags: c_uint) {

}

fn set_shortcut_key(_item: u64, _accel_key: c_uint, _modifier: c_uint) {

}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
    create_menu: extern "C" fn(title: *const c_char) -> u64,
    destroy_menu: extern "C" fn(handle: u64),
    
    add_menu_item: extern "C" fn(menu: u64, name: *const c_char, id: c_uint) -> PDMenuItem,
    remove_menu_item: extern "C" fn(item: u64),

    set_flags: extern "C" fn(item: u64, flags: c_uint),
    set_shortcut_key: extern "C" fn(item: u64, accel_key: c_uint, modifier: c_uint),
*/

pub fn get_menu_funcs1() -> *mut c_void {
    unsafe {
        let funcs: *mut c_void = transmute(&mut MENU_FUNCS);
        funcs
    }
    //ptr::null_mut()
}
