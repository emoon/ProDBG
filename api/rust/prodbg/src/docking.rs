use std::os::raw;

#[repr(u32)]
pub enum DockingCursor {
    Default = 0,
    SizeHorizontal = 1,
    SizeVertical = 2,
}

#[repr(u32)]
pub enum DockingSplitDir {
    Horizontal = 0,
    Vertical = 1,
}

pub type CDockHandle = *mut raw::c_void;

pub struct CDockingCallbacks {
    pub update_window_size: extern "C" fn(user_data: *mut raw::c_void,
                                          x: raw::c_int,
                                          y: raw::c_int,
                                          width: raw::c_int,
                                          height: raw::c_int),
    pub set_cursor_style: extern "C" fn(user_data: *mut raw::c_void, cursor: DockingCursor),
    pub save_user_data: extern "C" fn(item: *mut raw::c_void, user_data: *mut raw::c_void),
    pub load_user_data: extern "C" fn(item: *mut raw::c_void) -> *mut raw::c_void,
}

pub struct CDocking {
    pub name: *const raw::c_char,
    pub create_instance: extern "C" fn(x: raw::c_int,
                                       y: raw::c_int,
                                       width: raw::c_int,
                                       height: raw::c_int)
                                       -> *mut raw::c_void,
    pub destroy_instance: extern "C" fn(instance: *mut raw::c_void),
    pub get_handle_at: extern "C" fn(x: raw::c_int, y: raw::c_int) -> *mut raw::c_void,
    pub set_callbacks: extern "C" fn(instance: *mut raw::c_void, callbacks: *mut CDockingCallbacks),
    pub split: extern "C" fn(instance: *mut raw::c_void,
                             user_data: *mut raw::c_void,
                             dir: DockingSplitDir,
                             handle: CDockHandle),
    pub close_dock: extern "C" fn(instance: *mut raw::c_void, handle: CDockHandle),
    pub update_size: extern "C" fn(instance: *mut raw::c_void,
                                   user_data: *mut raw::c_void,
                                   width: raw::c_int,
                                   height: raw::c_int),
    pub set_mouse: extern "C" fn(instance: *mut raw::c_void,
                                 user_data: *mut raw::c_void,
                                 x: raw::c_int,
                                 y: raw::c_int,
                                 left_down: raw::c_uchar),
    pub save_state: extern "C" fn(instance: *mut raw::c_void, filename: *const raw::c_char),
    pub load_state: extern "C" fn(instance: *mut raw::c_void, filename: *const raw::c_char),
    pub update: extern "C" fn(instance: *mut raw::c_void),
}
