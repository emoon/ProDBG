use libc::{c_char, c_void, c_uint};
use prodbg_api::{CMenuFuncs1, PDMenuItem};
//use prodbg_api::CFixedString;
use std::mem::transmute;
use std::ptr;
//use minifb;

static mut MENU_FUNCS: CMenuFuncs1 = CMenuFuncs1 { 
    create_menu: create_menu,
    destroy_menu: destroy_menu,
    add_menu_item: add_menu_item,
    remove_menu_item: remove_menu_item,
    set_flags: set_flags,
    set_shortcut_key: set_shortcut_key,
};

//fn create_menu(title: *const c_char) -> *mut c_void {
fn create_menu(title: *const c_char) -> u64 {
    unsafe {
        //let name = CStr::from_ptr(title);
        //let menu = minfb::Menu::new(name.to_str().unwrap());
        //menu.0
        //let menu_ptr = transmute(Box::new(menu));
        
    }

    0

    //ptr::null_mut();
}

//fn destroy_menu(_handle: *mut c_void) {
fn destroy_menu(_handle: u64) {
    //let _: Box<minifb::Menu> = unsafe { transmute(handle) };
    // implicitly dropped
}
    
fn add_menu_item(_menu: u64, _name: *const c_char, _id: c_uint) -> u64 {
    /*
    let menu: &mut minifb::Menu = unsafe { &mut *(ptr as *mut minifb::Menu) };
    let name = CStr::from_ptr(title);
    menu.add_item(name.as_str().unwrap(), id).build().0
    */
    0
}

fn remove_menu_item(_item: u64) {

}

fn set_flags(_item: u64, _flags: c_uint) {

}

fn set_shortcut_key(_item: u64, _accel_key: c_uint, _modifier: c_uint) {

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


pub fn get_menu_funcs1() -> *mut c_void {
    unsafe {
        let funcs: *mut c_void = transmute(&mut MENU_FUNCS);
        funcs
    }
    //ptr::null_mut()
}
