use std::io;
use std::fs::File;
use std::io::Write;
use heck::SnakeCase;
use api_parser::*;

impl Variable {
    pub fn get_rust_ffi_type(&self) -> String {
        if self.primitive {
            self.vtype.clone()
        } else {
            if self.vtype == "String" {
                "*const c_char".to_owned()
            } else if self.vtype == "self" {
                "*const c_void".to_owned()
            } else {
                format!("*const PU{}", self.vtype)
            }
        }
    }
}

///
/// Generate ffi function
///
fn generate_ffi_function(f: &mut File, func: &Function) -> io::Result<()> {
    f.write_fmt(format_args!("    pub {}: extern \"C\" fn(", func.name))?;

    func.write_func_def(f, |_, arg| (arg.name.to_owned(), arg.get_rust_ffi_type()))?;
    f.write_all(b")")
}

///
/// Generate ffi function
///
fn generate_ffi_callback(f: &mut File, func: &Function) -> io::Result<()> {
    f.write_fmt(format_args!("    pub connect_{}: extern \"C\" fn(object: *const c_void, user_data: *const c_void,
                                        callback: extern \"C\" fn(self_c: *const c_void))",
                func.name))
}

fn generate_struct_body_recursive(f: &mut File, api_def: &ApiDef, sdef: &Struct) -> io::Result<()> {
    if let Some(ref inherit_name) = sdef.inherit {
        for sdef in &api_def.entries {
            if &sdef.name == inherit_name {
                generate_struct_body_recursive(f, api_def, &sdef)?;
            }
        }
    }

    for entry in &sdef.entries {
        match *entry {
            StructEntry::Var(ref var) => {
                f.write_fmt(format_args!("    pub {}: {},\n",
                                            var.name,
                                            var.get_rust_ffi_type()))?;
            }

            StructEntry::Function(ref func) => {
                if func.callback {
                    generate_ffi_callback(f, func)?;
                } else {
                    generate_ffi_function(f, func)?;
                }

                f.write_all(b",\n")?;
            }
        }
    }

    Ok(())
}


pub fn generate_ffi_bindings(filename: &str, api_def: &ApiDef, structs: &Vec<Struct>) -> io::Result<()> {
    let mut f = File::create(filename)?;

    f.write_all(b"use std::os::raw::{c_void, c_char};\n\n")?;

    for struct_ in structs {
        f.write_all(b"#[repr(C)]\n")?;
        f.write_fmt(format_args!("pub struct PU{} {{\n", struct_.name))?;

        generate_struct_body_recursive(&mut f, api_def, &struct_)?;

        if !struct_.is_pod() {
            f.write_all(b"    pub privd: *const c_void,\n")?;
        }

        f.write_all(b"}\n\n")?;
    }

    f.write_all(b"#[repr(C)]\n")?;
    f.write_all(b"pub struct PU {\n")?;

    for struct_ in structs.iter().filter(|s| !s.is_pod()) {
        f.write_fmt(format_args!("    pub create_{}: extern \"C\" fn(priv_data: *const c_void) -> *const PU{},\n",
                                 struct_.name.to_snake_case(), struct_.name))?;
    }

    f.write_all(b"    pub privd: *const c_void,\n")?;
    f.write_all(b"}\n\n")?;

    Ok(())
}
