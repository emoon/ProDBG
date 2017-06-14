use std::io;
use std::fs::File;
use std::io::Write;
use api_parser::*;
use std::collections::HashMap;
use heck::SnakeCase;

static HEADER: &'static [u8] = b"use rust_ffi::*;\n\n";

static UI_HEADER: &'static [u8] = b"pub struct Ui {
    pu: *const PU
}

impl Ui {
    pub fn new(pu: *const PU) -> Ui { Ui { pu: pu } }
\n";

///
///
///
fn generate_struct(f: &mut File, structs: &Vec<Struct>) -> io::Result<()> {
    for sdef in structs {
        f.write_fmt(format_args!("pub struct {} {{\n", sdef.name))?;

        if sdef.is_pod() {
            for entry in &sdef.entries {
                match *entry {
                    StructEntry::Var(ref var) => f.write_fmt(format_args!("    pub {}: {},\n", var.name, var.vtype))?,
                    _ => (),
                }
            }
        } else {
            // Assume for non-pod that we only use the FFI interface to do stuff.
            f.write_fmt(format_args!("    obj: const* PU{},\n", sdef.name))?;
        }

        f.write_all(b"}\n\n")?;
    }

    Ok(())
}

fn generate_func_impl(f: &mut File, func: &Function) -> io::Result<()> {
    f.write_fmt(format_args!("    pub fn {}(", func.name))?;

    func.write_func_def(f, |_, arg| {
        if arg.vtype == "String" {
            (arg.name.clone(), "&str".to_owned())
        } else if arg.vtype == "self" {
            ("&self".to_owned(), "".to_owned())
        } else if arg.primitive {
            (arg.name.to_owned(), arg.vtype.clone())
        } else {
            (arg.name.clone(), format!("&{}", arg.vtype.to_owned()))
        }
    })?;

    f.write_all(b") {\n")?;

    // Handle strings (as they need to use CString before call down to the C code

    let mut str_in_count = 0;
    let mut name_remap = HashMap::with_capacity(func.function_args.len());

    for arg in &func.function_args {
        if arg.vtype == "String" {
            f.write_fmt(format_args!("        let str_in_{} = CString::new({}).unwrap();\n", str_in_count, arg.name))?;
            name_remap.insert(arg.name.to_owned(), format!("str_in_{}.get_ptr()", str_in_count));
            str_in_count += 1;
        } else {
            name_remap.insert(arg.name.to_owned(), arg.name.to_owned());
        }
    }

    f.write_all(b"        unsafe {\n")?;
    f.write_fmt(format_args!("            ((*self.obj).{})(", func.name))?;

    func.write_func_def(f, |index, arg| {
        if index == 0 {
            ("(*self.obj).privd))".to_owned(), "".to_owned())
        } else if !arg.primitive {
            (format!("{}", name_remap.get(&arg.name.to_owned()).unwrap()), "".to_owned())
        } else {
            (arg.name.to_owned(), "".to_owned())
        }
    })?;

    f.write_all(b");\n")?;


    f.write_all(b"        }\n")?;
    f.write_all(b"    }\n\n")?;

    Ok(())
}

fn generate_impl(f: &mut File, api_def: &ApiDef) -> io::Result<()> {
    for sdef in api_def.entries.iter().filter(|s| !s.is_pod()) {
        f.write_fmt(format_args!("impl {} {{\n", sdef.name))?;

        let funcs = api_def.collect_functions(&sdef);

        for func in &funcs {
            if func.callback {
            } else {
                generate_func_impl(f, func)?;
            }
        }

        f.write_all(b"}\n\n")?;
    }

    Ok(())
}

fn generate_ui_impl(f: &mut File, api_def: &ApiDef) -> io::Result<()> {
    f.write_all(UI_HEADER)?;

    for sdef in api_def.entries.iter().filter(|s| !s.is_pod()) {
        let snake_name = sdef.name.to_snake_case();

        f.write_fmt(format_args!("    pub fn create_{}(&self) -> {} {{\n", snake_name, sdef.name))?;
        f.write_fmt(format_args!("        {} {{ obj: (*self.obj)(create_{})() }}\n", sdef.name, snake_name))?;
        f.write_all(b"    }\n\n")?;
    }

    f.write_all(b"}\n")?;

    Ok(())
}

pub fn generate_rust_bindigs(filename: &str, api_def: &ApiDef) -> io::Result<()> {
    let mut f = File::create(filename)?;

    f.write_all(HEADER)?;

    generate_struct(&mut f, &api_def.entries)?;
    generate_impl(&mut f, &api_def)?;
    generate_ui_impl(&mut f, &api_def)?;

    Ok(())
}
