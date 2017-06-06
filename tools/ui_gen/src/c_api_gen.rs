use std::io;
use std::fs::File;
use std::io::Write;
use ::api_parser::*;

static HEADER: &'static [u8] = b"
#pragma once\n
#include <stdint.h>\n
#ifdef __cplusplus
extern \"C\" {
#endif\n\n";

static FOOTER: &'static [u8] = b"
#ifdef __cplusplus
}
#endif\n";

fn get_type_name(tname: &str, primitve: bool) -> String {
    if primitve {
        if tname == "f32" {
            "float".to_owned()
        } else if tname == "f64" {
            return "double".to_owned()
        } else {
            // here we will have u8/i8,u32/etc
           if tname.starts_with("u") {
                format!("uint{}_t", &tname[1..])
            } else {
                format!("int{}_t", &tname[1..])
            }
        }
    } else {
        // Unknown type here, we always assume to use a struct Type*
        format!("struct {}*", tname)
    }
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
        f.write_fmt(format_args!("struct PU{} {{;\n", sdef.name))?;

        for entry in &sdef.entries {
            match *entry {
                StructEntry::Var(ref var) => {
                    f.write_fmt(format_args!("    {} {},\n", get_type_name(&var.vtype, var.primitive), var.name))?;
                },

                _ => (),
            }
        }

        f.write_fmt(format_args!("}}\n\n"))?;
    }

    f.write_all(FOOTER)?;

    Ok(())
}

