//mod signal_wrappers;

use std::io;
use std::fs::File;
use std::io::Write;
use std::collections::HashMap;
use api_parser::*;
use c_api_gen::get_type_name;
use c_api_gen::generate_c_function_args;
use c_api_gen::is_struct_pod;
use c_api_gen::callback_fun_def_name;

use heck::SnakeCase;
use heck::MixedCase;

fn generate_bind_info(info: &mut HashMap<String, Function>, func: &Function) {
    if !func.callback {
        return;
    }

    let mut input_args = String::new();

    for arg in &func.function_args {
        let tname = get_type_name(&arg);

        input_args.push_str(&tname);
        input_args.push_str(", ");
    }

    input_args.push_str("void*"); // user_data

    if info.contains_key(&input_args) {
        return;
    }

    info.insert(input_args.clone(), func.clone());
}

// generates signal wrappers for the variatos depending on input parameters
pub fn build_signal_wrappers_info(info: &mut HashMap<String, Function>, api_def: &ApiDef) {
    for sdef in &api_def.entries {
        for entry in &sdef.entries {
            match *entry {
                StructEntry::Function(ref func) => generate_bind_info(info, func),
                _ => (),
            }
        }
    }
}

fn signal_type_callback(func: &Function) -> String {
    let mut name_def = "Signal_".to_owned();

    for arg in &func.function_args {
        name_def.push_str(&arg.vtype);
        name_def.push_str("_")
    }

    name_def.push_str("void");
    name_def
}

// Generate a signal wrapper that is in the style of this:
//
//    class QSlotWrapperNoArgs : public QObject {
//        Q_OBJECT
//    public:
//        QSlotWrapperNoArgs(void* data, SignalNoArgs func) {
//            m_func = func;
//            m_data = data;
//        }
//
//        Q_SLOT void method() {
//            m_func(m_data);
//        }
//
//    private:
//        SignalNoArgs m_func;
//        void* m_data;
//    };
//
pub fn generate_signal_wrappers(f: &mut File, info: &HashMap<String, Function>) -> io::Result<()> {
    for (_, func) in info {
        let signal_type_name = signal_type_callback(func);

        f.write_all(b"///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n\n")?;

        f.write_fmt(format_args!("typedef void (*{})({});\n\n",
                                    signal_type_name,
                                    generate_c_function_args(func)))?;

        f.write_fmt(format_args!("class QSlotWrapper{} : public QObject {{\n    Q_OBJECT\npublic:\n", signal_type_name))?;
        f.write_fmt(format_args!("    QSlotWrapper{}(void* data, {} func) {{\n",
                                    signal_type_name,
                                    signal_type_name))?;
        f.write_all(b"        m_func = func;\n")?;
        f.write_all(b"        m_data = data;\n")?;
        f.write_all(b"    }\n\n")?;

        f.write_all(b"    Q_SLOT void method(")?;

        let args_count = func.function_args.len();

        if args_count > 0 {
            let mut input_args = String::new();

            for i in 0..args_count - 1 {
                let arg = &func.function_args[i];
                f.write_fmt(format_args!("{} {},", arg.vtype, arg.name))?;
                input_args.push_str(&arg.name);
                input_args.push_str(", ");
            }

            f.write_fmt(format_args!("{} {}) {{\n",
                                        get_type_name(&func.function_args[args_count - 1]),
                                        func.function_args[args_count - 1].name))?;
            input_args.push_str(&func.function_args[args_count - 1].name);

            f.write_fmt(format_args!("        m_func({}, m_data);\n", input_args))?;
        } else {
            f.write_all(b") {\n")?;
            f.write_all(b"        m_func(m_data);\n")?;
        }

        f.write_all(b"    }\n")?;

        f.write_all(b"private:\n")?;
        f.write_fmt(format_args!("    {} m_func;\n", signal_type_name))?;
        f.write_all(b"    void* m_data;\n")?;
        f.write_all(b"};\n\n")?;

    }

    Ok(())

}

fn function_name(struct_name: &str, func: &Function) -> String {
    format!("{}_{}", struct_name.to_snake_case(), func.name)
}

fn generate_func_def(f: &mut File,
                     struct_name: &str,
                     func: &Function,
                     struct_name_map: &HashMap<&str, &str>)
                     -> io::Result<()> {
    let ret_value;

    if let Some(ref ret_val) = func.return_val {
        ret_value = get_type_name(&ret_val);
    } else {
        ret_value = "void".to_owned();
    }

    // write return value and function name
    f.write_fmt(format_args!("static {} {}({}) {{ \n",
                                ret_value,
                                function_name(struct_name, func),
                                generate_c_function_args(func)))?;

    let struct_qt_name = struct_name_map
        .get(struct_name)
        .unwrap_or_else(|| &struct_name);

    // (changed from snake_case to camelCase matches the Qt function)

    // QWidget* qt_data = (QWidget*)priv_data;
    f.write_fmt(format_args!("    Q{}* qt_data = (Q{}*)priv_data;\n",
                                struct_qt_name,
                                struct_qt_name))?;

    // TODO: Handle return functions

    // qt_data->func(params);
    f.write_fmt(format_args!("    qt_data->{}(", func.name.to_mixed_case()))?;

    func.write_c_func_def(f, |_, arg| {
        if arg.vtype == "String" {
            (format!("QString::fromLatin1({})", &arg.name), "".to_owned())
        } else {
            (arg.name.clone(), "".to_owned())
        }
    })?;

    f.write_all(b";\n")?;
    f.write_all(b"}\n\n")?;

    Ok(())
}


///
///
///
///

fn func_def_callback(f: &mut File, struct_name: &str, func: &Function) -> io::Result<()> {
    let signal_type_name = signal_type_callback(func);
    let func_name = function_name(struct_name, func);

    println!("struct_name {}", struct_name);

    f.write_fmt(format_args!("static {} {{\n",
                                callback_fun_def_name(false, &func_name, func)))?;

    //QSlotWrapperNoArgs* wrap = new QSlotWrapperNoArgs(reciver, (SignalNoArgs)callback);
    f.write_fmt(format_args!("    QSlotWrapper{}* wrap = new QSlotWrapper{}(user_data, ({})callback);\n", signal_type_name, signal_type_name, signal_type_name))?;
    f.write_all(b"    QObject* q_obj = (QObject*)object;\n")?;


    f.write_fmt(format_args!("    QObject::connect(q_obj, SIGNAL({}(",
                                func.name.to_mixed_case()))?;

    func.write_c_func_def(f, |_, arg| (get_type_name(arg), "".to_owned()))?;

    f.write_all(b"), wrap, SLOT(method(")?;

    func.write_c_func_def(f, |_, arg| (get_type_name(arg), "".to_owned()))?;

    f.write_all(b"));\n")?;
    f.write_all(b"}\n\n")?;

    Ok(())
}

///
///
///
///
fn generate_struct_body_recursive(f: &mut File,
                                  name: &str,
                                  sdef: &Struct,
                                  struct_name_map: &HashMap<&str, &str>,
                                  api_def: &ApiDef)
                                  -> io::Result<()> {
    if let Some(ref inherit_name) = sdef.inherit {
        for sdef in &api_def.entries {
            if &sdef.name == inherit_name {
                generate_struct_body_recursive(f, name, &sdef, struct_name_map, api_def)?;
            }
        }
    }

    for entry in &sdef.entries {
        match *entry {
            StructEntry::Function(ref func) => {
                f.write_all(b"///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n\n")?;

                if func.callback == false {
                    generate_func_def(f, name, func, struct_name_map)?;
                } else {
                    func_def_callback(f, name, func)?;
                }
            }

            _ => (),
        }
    }

    Ok(())
}

///
/// Generate includes from all the structs which are non-pod
///
fn generate_includes(f: &mut File,
                     struct_name_map: &HashMap<&str, &str>,
                     api_def: &ApiDef)
                     -> io::Result<()> {
    for sdef in api_def.entries.iter().filter(|v| !is_struct_pod(v)) {
        let struct_name = sdef.name.as_str();
        let struct_qt_name = struct_name_map
            .get(struct_name)
            .unwrap_or_else(|| &struct_name);
        f.write_fmt(format_args!("#include <Q{}>\n", struct_qt_name))?;
    }

    Ok(())
}

///
/// Generate the struct defs
///
fn generate_struct_def(f: &mut File,
                       struct_name: &str,
                       api_def: &ApiDef,
                       sdef: &Struct)
                       -> io::Result<()> {
    if let Some(ref inherit_name) = sdef.inherit {
        for sdef in &api_def.entries {
            if &sdef.name == inherit_name {
                generate_struct_def(f, struct_name, api_def, &sdef)?;
            }
        }
    }

    for entry in &sdef.entries {
        match *entry {
            StructEntry::Function(ref func) => {
                if func.callback {
                    f.write_fmt(format_args!("    connect_{},\n",
                                                function_name(struct_name, func)))?;
                } else {
                    f.write_fmt(format_args!("    {},\n", function_name(struct_name, func)))?;
                }
            }

            _ => (),
        }
    }

    Ok(())
}

///
///
///
///
///
fn generate_struct_defs(f: &mut File, api_def: &ApiDef) -> io::Result<()> {
    for sdef in api_def.entries.iter().filter(|v| !is_struct_pod(v)) {
        f.write_all(b"///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n\n")?;
        f.write_fmt(format_args!("static struct PU{} s_{} = {{\n",
                                    sdef.name,
                                    sdef.name.to_snake_case()))?;

        generate_struct_def(f, &sdef.name, api_def, sdef)?;

        f.write_all(b"    0,\n")?;
        f.write_all(b"};\n\n")?;
    }

    Ok(())
}

///
/// This is the main entry for generating the C/C++ buindings for Qt. The current API mimics Qt
/// fairly closely when it comes to names but at the start there is a map setup that used used
/// to translate between some struct names to fit the Qt version. If more structs gets added that
/// needs to be translated into another Qt name they needs to be added here as well
///

pub fn generate_qt_bindings(filename: &str, api_def: &ApiDef) -> io::Result<()> {
    let mut f = File::create(filename)?;
    let mut signals_info = HashMap::new();

    let mut struct_name_map = HashMap::new();

    struct_name_map.insert("Button", "AbstractButton");

    f.write_all(b"#include \"c_api.h\"\n")?;

    generate_includes(&mut f, &struct_name_map, api_def)?;
    build_signal_wrappers_info(&mut signals_info, api_def);
    generate_signal_wrappers(&mut f, &signals_info)?;

    // generate wrapper functions

    for sdef in &api_def.entries {
        generate_struct_body_recursive(&mut f, &sdef.name, &sdef, &struct_name_map, api_def)?;
    }

    generate_struct_defs(&mut f, api_def)?;

    Ok(())
}
