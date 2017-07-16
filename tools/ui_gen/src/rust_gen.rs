use std::io;
use std::fs::File;
use std::io::Write;
use std::io::{Error, ErrorKind};
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

trait TypeHandler {
    fn match_type(&self) -> &'static str;

    fn replace_arg(&self, arg: &Variable) -> (String, String) {
        (arg.name.to_owned(), arg.vtype.to_owned()
    }

    fn gen_body(&self, arg: &str, f: &mut File, index: usize) -> String;
}

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

fn get_arg(arg: &Variable, type_handlers: &Vec<Box<TypeHandler>>) {
    for handler in &type_handlers {
        handler.match_type() == arg.vtype {
            return handler.replace_arg(arg);
        }
    }

    if arg.vtype == "self" {
        ("&self".to_owned(), "".to_owned())
    } else if arg.primitive {
        (arg.name.to_owned(), arg.vtype.clone())
    } else {
        (arg.name.clone(), format!("&{}", arg.vtype.to_owned()))
    }
}

fn generate_func_impl(f: &mut File, func: &Function, type_handlers: &Vec<Box<TypeHandler>>) -> io::Result<()> {
    f.write_fmt(format_args!("    pub fn {}(", func.name))?;

    func.write_func_def(f, |_, arg| {
        get_arg(arg)
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

fn get_function_args(func: &Function) -> String {
    let mut args = String::new();

    for arg in &func.function_args {
        args.push_str(&arg.vtype);
        args.push_str(", ");
    }

    args
}

///
/// Generate something that looks like this
///
/// macro_rules! connect_released {
///     ($sender:expr, $data:expr, $call_type:ident, $callback:path) => {
///         {
///             extern "C" fn temp_call(target: *mut std::os::raw::c_void) {
///                 unsafe {
///                     let app = target as *mut $call_type;
///                     $callback(&mut *app);
///                 }
///             }
/// 
///             unsafe {
///                 let object = (*(*$sender.get_obj()).base).object;
///                 wrui::connect(object, b"2released()\0", $data, temp_call);
///             }
///         }
///     }
/// }
fn _generate_connect_impl(_f: &mut File, _func: &Function) -> io::Result<()> {
    Ok(())
}

///
/// This code assumes that the connection name has the same number of args 
///
fn generate_connect(_f: &mut File, api_def: &ApiDef) -> io::Result<()> {
    let mut connect_names: HashMap<String, String> = HashMap::new();

    for sdef in api_def.entries.iter().filter(|s| !s.is_pod()) {
        let funcs = api_def.collect_callback_functions(&sdef);

        for func in funcs.iter().filter(|s| s.callback) {
            let args = get_function_args(&func);
            let mut found = true;

            if let Some(ref current_args) = connect_names.get(&func.name) {
                if *current_args != &args {
                    println!("Signal: {} - has versions with diffrent args {} - {}", func.name, current_args, args);
                    return Err(Error::new(ErrorKind::Other, "Fail"));
                }
            } else {
                found = false;
            }

            if !found {
                connect_names.insert(func.name.clone(), args.clone());
            }
        }
    }

    let mut connect_list = connect_names.iter().collect::<Vec<(&String, &String)>>();
    connect_list.sort();

    println!("{:?}", connect_list);

    Ok(())
}

fn generate_impl(f: &mut File, api_def: &ApiDef, type_handlers: &Vec<Box<TypeHandler>>) -> io::Result<()> {
    for sdef in api_def.entries.iter().filter(|s| !s.is_pod()) {
        f.write_fmt(format_args!("impl {} {{\n", sdef.name))?;

        for func in api_def.collect_regular_functions(&sdef) {
            generate_func_impl(f, &func, type_handlers)?;
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


struct StringTypeHandler;

///
/// We need to handle strings in a special way. They need to be sent down using CString and the
/// pointer to it so have a generator for it
///
impl TypeHandler for StringTypeHandler {
    fn match_type(&self) -> &'static str { "String" }

    fn replace_arg(&self, arg: &str, arg_type: &str) -> (String, String) {
        (arg.to_owned(), "&str".to_owned()
    }

    fn get_arg(&self, _arg: &str) -> (String, String) 
        (arg.clone(), "&str".to_owned()
    }

    fn gen_body(&self, arg: &str, f: &mut File, index: usize) -> String {
        let new_arg_name = format!("str_in_{}_{}", arg, index);
        f.write_fmt(format_args!("        let {} = CString::new({}).unwrap();\n", arg_name, arg).unwrap();
        format!("{}.get_ptr()", new_arg_name)
    }
}


pub fn generate_rust_bindigs(filename: &str, api_def: &ApiDef) -> io::Result<()> {
    let mut f = File::create(filename)?;
    let mut type_handlers: Vec<Box<TypeHandler>> = Vec::new();
    
    type_handlers.push(Box::new(StringTypeHandler{}));


    //handlers.push(TypeHandler {
    //    type_name: "String",
    //    handler: Box::new(|| "move 2"),
    //});

    //let t: Box<(Fn() -> &'static str)> = Box::new(move || "move");
    //let t2: Box<(Fn() -> &'static str)> = Box::new(move || "move 2");
    //let t3 = Box::new(|| "move 2");
    //let t4 = Box::new(|| "move 2");

    //handlers.push(t);
    //handlers.push(t2);
    //handlers.push(t3);
    //handlers.push(t4);
    
    /*
    for h in handlers {
        let p = (h.handler)();

        println!("{}", p);
    }
    */

    //handlers.push(Box::new(move || "test"));


    /*
    let t = TypeHandler {
        type_name: "Test",
        arg_handle: Box::new(move || "Test".to_owned()),
        func_handle: Box::new(move |arg_handle| {
            "Test2".to_owned()
        }),
    };
    */

    //let _ = (t.arg_handle)();
    //let _ = (t.func_handle)("test");

    f.write_all(HEADER)?;

    generate_struct(&mut f, &api_def.entries)?;
    generate_impl(&mut f, &api_def)?;
    generate_connect(&mut f, &api_def)?;
    generate_ui_impl(&mut f, &api_def)?;

    Ok(())
}
