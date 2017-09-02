use std::io;
use std::fs::File;
use std::io::Write;
use api_parser::*;
use heck::SnakeCase;

static HEADER: &'static [u8] = b"
#pragma once\n
#include <stdint.h>
#include <stdbool.h>\n
#ifdef __cplusplus
extern \"C\" {
#endif\n\n";

static FOOTER: &'static [u8] = b"
#ifdef __cplusplus
}
#endif\n";

pub fn generate_c_function_args(func: &Function) -> String {
    let mut function_args = String::new();
    let len = func.function_args.len();

    // write arguments
    for (i, arg) in func.function_args.iter().enumerate() {
        function_args.push_str(&arg.get_c_type());
        function_args.push_str(" ");
        function_args.push_str(&arg.name);

        if i != len - 1 {
            function_args.push_str(", ");
        }
    }

    function_args
}

fn generate_func_def(f: &mut File, func: &Function) -> io::Result<()> {
    let ret_value = func.return_val
        .as_ref()
        .map_or("void".to_owned(), |r| r.get_c_type());

    // write return value and function name
    f.write_fmt(format_args!("    {} (*{})(", ret_value, func.name))?;

    func.write_c_func_def(f, |_, arg| {
        (arg.get_c_type(), arg.name.to_owned())
    })?;

    f.write_all(b";\n")
}

pub fn callback_fun_def_name(def: bool, name: &str, func: &Function) -> String {
    let mut func_def;

    if def {
        func_def = format!("void (*connect_{})(void* object, void* user_data, void (*callback)(", name);
    } else {
        func_def = format!("void connect_{}(void* object, void* user_data, void (*callback)(", name);
    }

    let arg_count = func.function_args.len();

    for (i, arg) in func.function_args.iter().enumerate() {
        func_def.push_str(&arg.get_c_type());
        func_def.push_str(" ");
        func_def.push_str(&arg.name);

        if i != arg_count - 1 {
            func_def.push_str(", ");
        }
    }

    func_def.push_str("))");
    func_def
}

fn generate_callback_def(f: &mut File, func: &Function) -> io::Result<()> {
    f.write_fmt(format_args!("    {};\n", callback_fun_def_name(true, &func.name, func)))
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
                f.write_fmt(format_args!("    {} {};\n", var.get_c_type(), var.name))?;
            }

            StructEntry::Function(ref func) => {
                if func.callback == false {
                    generate_func_def(f, func)?;
                } else {
                    generate_callback_def(f, func)?;
                }
            }
        }
    }

    Ok(())
}

pub fn generate_c_api(filename: &str, api_def: &ApiDef) -> io::Result<()> {
    let mut f = File::create(filename)?;

    f.write_all(HEADER)?;

    // Write forward declarations

    for sdef in &api_def.entries {
        f.write_fmt(format_args!("struct PU{};\n", sdef.name))?;
    }

    f.write_all(b"\n")?;

    // Write the struct defs

    for sdef in &api_def.entries {
        f.write_fmt(format_args!("struct PU{} {{\n", sdef.name))?;

        generate_struct_body_recursive(&mut f, api_def, sdef)?;

        if !sdef.is_pod() {
            f.write_all(b"    void* priv_data;\n")?;
        }

        f.write_fmt(format_args!("}};\n\n"))?;
    }

    // generate C_API entry

    f.write_all(b"typedef struct PU { \n")?;

    for sdef in api_def.entries.iter().filter(|s| !s.is_pod()) {
        f.write_fmt(format_args!("    struct PU{}* (*create_{})(void* self);\n",
                                    sdef.name,
                                    sdef.name.to_snake_case()))?;
    }

    f.write_all(b"    void* priv_data;\n} PU;\n")?;

    f.write_all(FOOTER)?;

    Ok(())
}
