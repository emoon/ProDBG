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

    f.write_all(FOOTER)?;

    Ok(())
}

