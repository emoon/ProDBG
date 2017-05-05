use std::io;
use std::fs::File;
use std::io::Write;

use ::data::*;

pub fn generate_ffi_bindings(filename: &str, structs: &Vec<Struct>) -> io::Result<()> {
    let mut f = File::create(filename)?;

    f.write_all(b"use std::os::raw::{c_void, c_char};\n\n")?;

    for struct_ in structs {
        f.write_all(b"#[repr(C)]\n")?;
        f.write_fmt(format_args!("pub struct {} {{\n", struct_.name))?;

        for entry in &struct_.entries {
            match entry {
                &StructEntry::Var(ref var) => {
                    f.write_fmt(format_args!("    pub {}: {},\n", var.name, var.rust_ffi_type))?;
                }

                &StructEntry::FunctionPtr(ref func_ptr) => {
                    f.write_fmt(format_args!("    pub {}: extern \"C\" fn(", func_ptr.name))?;

                    func_ptr.write_func_def(&mut f, |_, arg| {
                        (arg.name.to_owned(), arg.rust_ffi_type.to_owned())
                    })?;

                    f.write_all(b",\n")?;
                }
            }
        }

        f.write_all(b"}\n\n")?;
    }

    Ok(())
}
