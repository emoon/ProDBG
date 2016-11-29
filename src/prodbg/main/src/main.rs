
fn main() {
    unsafe { qt_main(); }
}

extern "C" {
    fn qt_main();
}


// dummy hack, this is here because we link with the plugin_api and it expects this to be present.
#[no_mangle]
pub fn init_plugin() {}
