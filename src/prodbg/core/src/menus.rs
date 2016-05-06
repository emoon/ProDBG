use std::ptr;

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
    ptr::null_mut();

}
