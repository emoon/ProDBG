extern crate prodbg_ui;

//use prodbg_ui::*;

#[no_mangle]
pub fn init_plugin(ui_ptr: *const prodbg_ui::ffi_gen::PU) {
    let ui = prodbg_ui::ui_gen::Ui::new(ui_ptr);

    //

    ui.push_button_create();

    println!("Rust: memory_view_2");
}
