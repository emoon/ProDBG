#![recursion_limit = "200"]
#[macro_use]
extern crate pest;
extern crate heck;

pub mod api_parser;
pub mod c_api_gen;
pub mod qt;
mod rust_ffi_gen;
mod rust_gen;

use std::io;

static INPUT_API: &str = "src/api.def";

static C_API_HEADER: &str = "../../src/prodbg/PluginUI/generated/c_api.h";
static QT_API_IMPL: &str = "../../src/prodbg/PluginUI/generated/qt_api_gen.cpp";
static QT_API_IMPL_HEADER: &str = "../../src/prodbg/PluginUI/generated/qt_api_gen.h";

static RUST_FFI_FILE: &str = "../../api/rust/prodbg_ui/src/ffi_gen.rs";
static UI_FILE: &str = "../../api/rust/prodbg_ui/src/lib.rs";

fn generate_code() -> io::Result<()> {
    let api_def = api_parser::ApiDef::new(INPUT_API);

    c_api_gen::generate_c_api(C_API_HEADER, &api_def)?;
    qt::generate_qt_bindings(QT_API_IMPL, QT_API_IMPL_HEADER, &api_def)?;

    rust_ffi_gen::generate_ffi_bindings(RUST_FFI_FILE, &api_def.entries)?;
    rust_gen::generate_rust_bindigs(UI_FILE, &api_def)?;

    Ok(())
}

fn main() {
    if let Err(err) = generate_code() {
        panic!("Unable to generate {} err {:?}", C_API_HEADER, err);
    }
}
