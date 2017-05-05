use ::data::*;

use std::io;
use std::fs::File;
use std::io::Write;
use std::collections::HashMap;

///
/// 2. Check if function is create style function
///
fn is_create_func(func: &FuncPtr) -> bool {
	if func.name.find("_create").is_none() {
		return false;
	}

	func.return_val.is_some()
}

///
///
///
fn find_struct<'a>(name: &str, structs: &'a Vec<Struct>) -> &'a Struct {
	structs.iter().find(|&e| { e.name == name }).unwrap()
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
    f.write_fmt(format_args!("    pub fn {}(", func_ptr.name))?;
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
    f.write_fmt(format_args!("            ((*self.funcs).{})(", func_ptr.name))?;

    let arg_count = func_ptr.function_args.len();

    func_ptr.write_func_def(f, |index, arg| {
        if index == arg_count {
            return ("".to_owned(), "".to_owned());
        }

        if index == 0 {
            ("self.obj".to_owned(), "".to_owned())
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
/// Check if type has widget base member and we will imlpment Widget trait for it
///
fn has_widget_base(s: &Struct) -> bool {
	for entry in &s.entries {
		match entry {
			&StructEntry::Var(ref var) => {
			    if var.rust_type == "Widget" && var.name == "base" {
			        return true;
                }
            }
			_ => (),
		}
	}

	false
}

///
/// 3. Find name_funcs that maps to 2.
///
fn generate_struct(f: &mut File, func: &FuncPtr, structs: &Vec<Struct>) -> io::Result<()> {
	let type_name = func.return_val.as_ref().unwrap().rust_ffi_type[7..].to_owned();
	let funcs_name = type_name.clone() + "Funcs";
	let funcs_struct = find_struct(&funcs_name, structs);
	let data_struct = find_struct(&type_name, structs);

	f.write_fmt(format_args!("pub struct {} {{\n", &type_name[2..]))?;
	f.write_fmt(format_args!("    pub widget_funcs: *const {},\n", "PUWidgetFuncs"))?;
	f.write_fmt(format_args!("    pub funcs: *const {},\n", funcs_struct.name))?;
	f.write_fmt(format_args!("    pub obj: *const {},\n", type_name))?;
	f.write_all(b"}\n\n")?;

	f.write_fmt(format_args!("impl {} {{\n", &type_name[2..]))?;

	for entry in &funcs_struct.entries {
		match entry {
			&StructEntry::FunctionPtr(ref func_ptr) => generate_function_entry(f, func_ptr)?,
			_ => (),
		}
	}

	f.write_all(b"    #[inline]\n")?;
	f.write_fmt(format_args!("    pub fn get_obj(&self) -> *const {} {{ self.obj }}\n", type_name))?;

	// generate obj access function

	f.write_all(b"}\n\n")?;

	// For widgets we implemnt the Widget Trait also

	if has_widget_base(data_struct) {
	    f.write_fmt(format_args!("impl Widget for {} {{\n", &type_name[2..]))?;
	    f.write_all(b"   fn get_obj(&self) -> *const PUWidget {\n")?;
	    f.write_all(b"       unsafe { (*self.obj).base }\n")?;
	    f.write_all(b"   }\n")?;
	    f.write_all(b"   fn get_funcs(&self) -> *const PUWidgetFuncs {\n")?;
	    f.write_all(b"       self.widget_funcs\n")?;
	    f.write_all(b"   }\n")?;
	    f.write_all(b"}\n\n")?;
    }

	Ok(())
}

///
/// This functions generates "Real" Rust bindings (using the FFI wrapper)
///
/// It's done in this way:
///
/// 1. Find the the Wrui struct.
/// 2. Find a name_create function that returns PUX* inside the UI struct
/// 3. Find name_funcs that maps to 2.
/// 4. Generate struct X which uses functions in name_funcs and wraps PUX object
/// 5. If struct has PUWidget* base also generate Widget trait impl
///
pub fn generate_rust_binding(filename: &str, structs: &Vec<Struct>) -> io::Result<()> {
	let mut f = File::create(filename)?;

    f.write_all(b"use std::ffi::CString;\n\n")?;
    f.write_all(b"use ffi_gen::*;\n\n")?;
    f.write_all(b"use traits_gen::*;\n\n")?;

	let wrui_struct = structs.iter().find(|&e| { e.name == "PU" } ).unwrap();

	for entry in &wrui_struct.entries {
		match entry {
			&StructEntry::FunctionPtr(ref func_ptr) => {
				if !is_create_func(func_ptr) {
					continue;
				}

				generate_struct(&mut f, func_ptr, structs)?;
			}

			_ => (),
		}
	}

	Ok(())
}

