use std::io;
use std::fs::File;
use std::io::Write;

extern crate clang;

#[derive(Debug)]
pub struct Variable {
    pub name: String,
    pub c_type: String,
    pub rust_ffi_type: String,
    pub rust_type: String,
    // This is set if we for example need to call
    // foo.get_ptr() to access the real rust type
    pub access_type: Option<&'static str>,
}

#[derive(Debug)]
pub struct FuncPtr {
    pub name: String,
    pub function_args: Vec<Variable>,
    pub return_val: Option<Variable>,
}

#[derive(Debug)]
pub enum StructEntry {
    Var(Variable),
    FunctionPtr(FuncPtr),
}

#[derive(Debug)]
pub struct Struct {
    pub name: String,
    pub entries: Vec<StructEntry>,
}

///
/// Checks if a type is a function pointer or not. I'm sure there is a better way for this
/// but it works so meh.
///
fn is_type_function_ptr(t: &clang::Type) -> bool {
    let type_name = t.get_display_name();

    if type_name.find("(*)").is_some() {
        true
    } else {
        false
    }
}

fn get_primitive_type(name: &str) -> Option<&'static str> {
    match name {
        "_Bool" => Some("bool"),
        "int" => Some("i32"),
        "uint8_t" => Some("u8"),
        "uint16_t" => Some("u16"),
        "uint32_t" => Some("u32"),
        "uint64_t" => Some("u64"),
        "int8_t" => Some("s8"),
        "int16_t" => Some("s16"),
        "int32_t" => Some("s32"),
        "int64_t" => Some("s64"),
        "constchar*" => Some("*const c_char"),
        "void*" => Some("*const c_void"),
        "void" => Some(""),
        _ => None,
    }
}


///
/// Maps a C type to Rust
///
fn map_c_type_to_rust(name: &String) -> (String, String, Option<&'static str>) {
    let s = name.replace("struct ", "");
    let s = s.replace(" ", "");

    if let Some(prim_type) = get_primitive_type(&s) {
        if prim_type == "*const c_char" {
            return (prim_type.to_owned(), "str".to_owned(), Some(".as_ptr()"));
        } else {
            return (prim_type.to_owned(), prim_type.to_owned(), None);
        }
    }

    // found pointer, return as is
    if s.find("*").is_some() {
        if s.find("PU").is_some() {
            (format!("*const {}", &s[..s.len() - 1]), s[2..s.len() - 1].to_owned(), Some(".get_obj()"))
        } else {
            (format!("*const {}", &s[..s.len() - 1]), "".to_owned(), None)
        }
    } else {
        panic!("Unable to decode type {}", name);
    }
}

///
/// Generates a regular variable in a struct
///
fn generate_var(var: &clang::Entity) -> StructEntry {
    let t = var.get_type().unwrap();
    let display_name = t.get_display_name();

    let types = map_c_type_to_rust(&display_name);

    let var = Variable {
        name: var.get_display_name().unwrap().to_owned(),
        c_type: display_name.to_owned(),
        rust_ffi_type: types.0,
        rust_type: types.1,
        access_type: types.2,
    };

    StructEntry::Var(var)
}

///
/// Generates a function pointer
///
fn generate_function_ptr(var: &clang::Entity) -> StructEntry {
    let t = var.get_type().unwrap();
    let display_name = t.get_display_name();

    let mut func_ptr = FuncPtr {
        name: var.get_display_name().unwrap().to_owned(),
        function_args: Vec::new(),
        return_val: None,
    };

    // handle the return type

    let type_end = display_name.find("(*)").unwrap();	// will always be ok inside this function as we found this before
    let ret_type = &display_name[..type_end];
    let types = map_c_type_to_rust(&ret_type.to_owned());

    if ret_type != "void " {
        func_ptr.return_val = Some(Variable {
            name: "".to_owned(),
            c_type: ret_type.to_owned(),
            rust_ffi_type: types.0,
            rust_type: types.1,
            access_type: types.2,
        });
    }

    for c in var.get_children().iter() {
        let t = c.get_type().unwrap();

        if c.get_kind() == clang::EntityKind::TypeRef {
            continue;
        }

        let types = map_c_type_to_rust(&t.get_display_name());

        let param = Variable {
            name: c.get_display_name().unwrap().to_owned(),
            c_type: t.get_display_name(),
            rust_ffi_type: types.0,
            rust_type: types.1,
            access_type: types.2,
        };

        func_ptr.function_args.push(param);
    }

    StructEntry::FunctionPtr(func_ptr)
}

pub fn build_data(structs: &Vec<clang::Entity>) -> Vec<Struct> {
    let mut struct_entries = Vec::new();

    for struct_ in structs {
        if struct_.get_children().len() == 0 {
            continue;
        }

        let mut struct_data = Struct {
            name: struct_.get_name().unwrap().to_owned(),
            entries: Vec::new(),
        };

        for field in struct_.get_children() {
            // let name = field.get_name().unwrap();
            let t = field.get_type().unwrap();

            if is_type_function_ptr(&t) {
                struct_data.entries.push(generate_function_ptr(&field));
            } else {
                struct_data.entries.push(generate_var(&field));
            }
        }

        struct_entries.push(struct_data);
    }

    struct_entries
}

impl FuncPtr {
    pub fn write_func_def<F>(&self, f: &mut File, filter: F) -> io::Result<()>
                         where F: Fn(usize, &Variable) -> (String, String) {
        let arg_count = self.function_args.len();

        for (i, arg) in self.function_args.iter().enumerate() {
            let filter_arg = filter(i, &arg);

            if filter_arg.1 == "" {
                f.write_fmt(format_args!("{}", filter_arg.0))?;
            } else {
                f.write_fmt(format_args!("{}: {}", filter_arg.0, filter_arg.1))?;
            }

            if i != arg_count - 1 {
                f.write_all(b", ")?;
            }
        }

        f.write_all(b")")?;

        if let Some(ref ret_var) = self.return_val {
            let filter_arg = filter(arg_count, &ret_var);
            if filter_arg.1 != "" {
                f.write_fmt(format_args!(" -> {}", filter_arg.1))?;
            }
        }

        Ok(())
    }
}

