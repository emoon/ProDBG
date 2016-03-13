/*
///! This is a wrapper over the Docking API which is in C. This allows 
///! plugin authors to replace the docking system in ProDBG
///!
struct DockingWrapper {


}

fn update_window_size(user_data: *mut raw::c_void,
                      x: raw::c_int,
                      y: raw::c_int,
                      width: raw::c_int,
                      height: raw::c_int) {

}

fn set_cursor_style(user_data: *mut raw::c_void, cursor: DockingCursor) {

}

fn save_user_data(item: *mut raw::c_void, user_data: *mut raw::c_void) {
}

fn load_user_data(item: *mut raw::c_void) -> *mut raw::c_void {
}

static callbacks: CDockingCallbacks = CDockingCallbacks {
    update_window_size: update_window_size,
    set_cursor_style: set_cursor_style,
    save_user_data: save_user_data,
    load_user_data: load_user_data,
};

impl DockingWrapper {}
*/
