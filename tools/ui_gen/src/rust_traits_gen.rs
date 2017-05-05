use std::io;
use std::fs::File;
use std::io::Write;
use std::collections::HashMap;

use ::data::*;

struct MatchName {
	c_name: &'static str,
	rust_name: &'static str,
}

fn remap_rust_type(var: &Variable) -> String {
    if var.name.find("opt_").is_some() {
        format!("Option<&{}>", var.rust_type)
    } else {
        if var.access_type.is_some() {
            format!("&{}", var.rust_type)
        } else {
            var.rust_type.to_owned()
        }
    }
}

///
/// generate function entry
///
fn generate_function_entry(f: &mut File, func_ptr: &FuncPtr) -> io::Result<()> {
    f.write_fmt(format_args!("    fn {}(", func_ptr.name))?;
    func_ptr.write_func_def(f, |index, arg| {
        if index == 0 {
            ("&self".to_owned(), "".to_owned())
        } else {
            (arg.name.to_owned(), remap_rust_type(&arg))
        }
    })?;

    f.write_all(b" {\n")?;

    // Handle strings (as they need to use CString before call down to the C code

    let mut str_in_count = 0;
    let mut name_remap = HashMap::with_capacity(func_ptr.function_args.len());

    for arg in &func_ptr.function_args {
        if arg.rust_type == "str" {
            f.write_fmt(format_args!("        let str_in_{} = CString::new({}).unwrap();\n", str_in_count, arg.name))?;
            name_remap.insert(arg.name.to_owned(), format!("str_in_{}", str_in_count));
            str_in_count += 1;
        } else {
            name_remap.insert(arg.name.to_owned(), arg.name.to_owned());
        }
    }

    f.write_all(b"        unsafe {\n")?;
    f.write_all(b"            let obj = self.get_obj();\n")?;
    f.write_all(b"            let funcs = self.get_funcs();\n")?;
    f.write_fmt(format_args!("            ((*funcs).{})(", func_ptr.name))?;

    let arg_count = func_ptr.function_args.len();

    func_ptr.write_func_def(f, |index, arg| {
        if index == arg_count {
            return ("".to_owned(), "".to_owned());
        }

        if index == 0 {
            ("obj".to_owned(), "".to_owned())
        } else {
            if let Some(access) = arg.access_type {
                (format!("{}{}", name_remap.get(&arg.name.to_owned()).unwrap(), access), "".to_owned())
            } else {
                (arg.name.to_owned(), "".to_owned())
            }
        }
    })?;

    f.write_all(b"\n")?;
    f.write_all(b"        }\n")?;
    f.write_all(b"    }\n\n")?;

    Ok(())
}


///
/// Generates traits (currenly on Widget trait)
///
pub fn generate_traits(filename: &str, structs: &Vec<Struct>) -> io::Result<()> {
	let trait_names = [
		MatchName {
			c_name: "PUWidgetFuncs",
			rust_name: "Widget",
		}
	];

	let mut f = File::create(filename)?;

    f.write_all(b"use ::ffi_gen;\n\n")?;

	for struct_ in structs {
		for trait_name in &trait_names {
			if struct_.name != trait_name.c_name {
				continue;
			}

        	f.write_fmt(format_args!("pub trait {} {{\n", trait_name.rust_name))?;

        	for entry in &struct_.entries {
        		match entry {
					&StructEntry::FunctionPtr(ref func_ptr) => generate_function_entry(&mut f, func_ptr)?,
                	_ => (),
				}
			}

	        f.write_all(b"    fn get_obj(&self) -> *const ffi_gen::PUWidget;\n")?;
	        f.write_all(b"    fn get_funcs(&self) -> *const ffi_gen::PUWidgetFuncs;\n")?;
        	f.write_all(b"}\n\n")?;
		}
	}


	Ok(())
}

