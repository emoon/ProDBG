use std::io;
use std::fs::File;
use std::io::Write;
use api_parser::*;

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

pub fn get_type_name(arg: &Variable) -> String {
    let tname = &arg.vtype;
    let primitve = arg.primitive;

    if tname == "String" {
        return "const char*".to_owned();
    }

    if primitve {
        if tname == "f32" {
            return "float".to_owned();
        } else if tname == "bool" {
            return "bool".to_owned();
        } else if tname == "f64" {
            return "double".to_owned();
        } else {
            // here we will have u8/i8,u32/etc
            if tname.starts_with("u") {
                return format!("uint{}_t", &tname[1..]);
            } else {
                return format!("int{}_t", &tname[1..]);
            }
        }
    } else {
        // Unknown type here, we always assume to use a struct Type*
        format!("struct PU{}*", tname)
    }
}

fn is_struct_pod(sdef: &Struct) -> bool {
    sdef.entries
        .iter()
        .all(|e| match *e {
                 StructEntry::Var(ref _var) => true,
                 _ => false,
             })
}

pub fn generate_c_function_args(func: &Function) -> String {
    let mut function_args = String::new();

    // write arguments
    for arg in &func.function_args {
        function_args.push_str(&get_type_name(&arg));
        function_args.push_str(", ");
    }

    function_args.push_str("void*");
    function_args
}

fn generate_func_def(f: &mut File, func: &Function) -> io::Result<()> {
    let ret_value;

    if let Some(ref ret_val) = func.return_val {
        ret_value = get_type_name(&ret_val);
    } else {
        ret_value = "void".to_owned();
    }

    // write return value and function name
    f.write_fmt(format_args!("    {} (*{})({})\n", ret_value, func.name, generate_c_function_args(func)))
}

fn generate_callback_def(f: &mut File, func: &Function) -> io::Result<()> {
    // write return value and function name
    f.write_fmt(format_args!("    void connect_{}(void* object, void* user_data, void (*callback)(",
                                func.name))?;

    // write arguments
    for arg in &func.function_args {
        f.write_fmt(format_args!("{} {}, ",
                                    get_type_name(&arg),
                                    arg.name))?;
    }

    // write last parameter (always private data)
    f.write_fmt(format_args!("void* user_data));\n"))
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
                f.write_fmt(format_args!("    {} {};\n", get_type_name(&var), var.name))?;
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

    for sdef in &api_def.entries {
        println!("name {}", sdef.name);
    }

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

        f.write_fmt(format_args!("}};\n\n"))?;
    }

    // generate C_API entry

    f.write_all(b"typedef struct PU { \n")?;

    use heck::SnakeCase;

    for sdef in &api_def.entries {
        if is_struct_pod(sdef) {
            continue;
        }

        f.write_fmt(format_args!("    struct PU{}* create_{}(void* priv_data);\n",
                                    sdef.name,
                                    sdef.name.to_snake_case()))?;
    }

    f.write_all(b"    void* priv_data;\n} PU;\n")?;

    f.write_all(FOOTER)?;

    Ok(())
}
