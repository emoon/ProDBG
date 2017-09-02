//mod signal_wrappers;

use std::io;
use std::fs::File;
use std::io::Write;
use std::collections::{HashMap, BTreeMap};
use api_parser::*;
use c_api_gen::generate_c_function_args;
use c_api_gen::callback_fun_def_name;

use heck::SnakeCase;
use heck::MixedCase;

static HEADER: &'static [u8] = b"
struct PrivData {
    QWidget* parent;
    void* user_data;
};\n\n";

static CREATE_TEMPLATE: &'static [u8] = b"
template<typename T, typename QT> T* create_func(T* struct_data, void* priv_data) {
    PrivData* data = (PrivData*)priv_data;
    QT* qt_obj = new QT(data->parent);
    T* ctl = new T;
    memcpy(ctl, struct_data, sizeof(T));
    ctl->priv_data = qt_obj;
    return ctl;
}\n\n";

static FOOTER: &'static [u8] = b"
struct PU* PU_create_instance(void* user_data, QWidget* parent) {
    struct PU* instance = new PU;
    memcpy(instance, &s_pu, sizeof(PU));
    PrivData* priv_data = new PrivData;
    priv_data->parent = parent;
    priv_data->user_data = user_data;
    return instance;
}\n\n";

fn generate_bind_info(info: &mut HashMap<String, Function>, func: &Function) {
    let mut input_args = String::new();

    for arg in &func.function_args {
        let tname = arg.get_c_type();
        input_args.push_str(&tname);
        input_args.push_str(", ");
    }

    input_args.push_str("void*"); // user_data

    info.entry(input_args.clone()).or_insert(func.clone());
}

// generates signal wrappers for the variatos depending on input parameters
pub fn build_signal_wrappers_info(info: &mut HashMap<String, Function>, api_def: &ApiDef) {
    for sdef in &api_def.entries {
        let mut functions = Vec::new();
        sdef.get_callback_functions(&mut functions);

        for func in &functions {
            generate_bind_info(info, &func);
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
    let ordered: BTreeMap<_, _> = info.iter().collect();

    for (_, func) in ordered {
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

        func.write_c_func_def(f, |index, arg| {
            if index == 0 {
                ("".to_owned(), "".to_owned())
            } else {
                (arg.get_c_type(), arg.name.to_owned())
            }
        })?;
            
        f.write_all(b" {\n        m_func(")?;

        func.write_c_func_def(f, |index, arg| {
            if index == 0 {
                ("m_data".to_owned(), "".to_owned())
            } else {
                (arg.name.to_owned(), "".to_owned())
            }
        })?;

        f.write_all(b";\n")?;
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
    let ret_value = func.return_val
        .as_ref()
        .map_or("void".to_owned(), |v| v.get_c_type());

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
    f.write_fmt(format_args!("    Q{}* qt_data = (Q{}*)self_c;\n",
                                struct_qt_name,
                                struct_qt_name))?;

    // TODO: Handle return functions

    // qt_data->func(params);
    f.write_fmt(format_args!("    qt_data->{}(", func.name.to_mixed_case()))?;

    func.write_c_func_def(f, |index, arg| {
        if index == 0 {
            ("".to_owned(), "".to_owned())
        } else if arg.vtype == "String" {
            (format!("QString::fromLatin1({})", &arg.name), "".to_owned())
        } else if arg.vtype == "Rect" {
            (format!("QRect({}->x, {}->y, {}->width, {}->height)", &arg.name, &arg.name, &arg.name, &arg.name), "".to_owned())
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

    func.write_c_func_def(f, |index, arg| { 
        if index == 0 {
            ("".to_owned(), "".to_owned())
        } else {
            (arg.get_c_type(), "".to_owned())
        }
    })?;

    f.write_all(b"), wrap, SLOT(method(")?;

    //func.write_c_func_def(f, |_, arg| (arg.get_c_type(), "".to_owned()))?;

    func.write_c_func_def(f, |index, arg| { 
        if index == 0 {
            ("".to_owned(), "".to_owned())
        } else {
            (arg.get_c_type(), "".to_owned())
        }
    })?;


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
    for sdef in api_def.entries.iter().filter(|v| !v.is_pod()) {
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
    for sdef in api_def.entries.iter().filter(|s| !s.is_pod()) {
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


// struct PUWidget* create_widget(void* priv_data) {
//    PrivData* data = (PrivData*)priv_data;
//    QWidget* qt_obj = new QWidget(data->parent);
//    PUWidget* ctl = new PUWidget;
//    memcpy(ctl, s_widget, sizeof(s_widget);
//    ctl->priv_data = qt_object;
//    return ctl;
// }

///
/// Generates the create functions
///
fn generate_create_functions(f: &mut File,
                             struct_name_map: &HashMap<&str, &str>,
                             api_def: &ApiDef)
                             -> io::Result<()> {
    f.write_all(b"///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n")?;
    f.write_all(CREATE_TEMPLATE)?;

    for sdef in api_def.entries.iter().filter(|s| !s.is_pod()) {
        let struct_name = sdef.name.as_str();

        f.write_all(b"///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n\n")?;

        f.write_fmt(format_args!("static struct PU{}* create_{}(void* priv_data) {{\n",
                                    sdef.name,
                                    sdef.name.to_snake_case()))?;

        let struct_qt_name = struct_name_map
            .get(struct_name)
            .unwrap_or_else(|| &struct_name);

        f.write_fmt(format_args!("    return create_func<struct PU{}, Q{}>(&s_{}, priv_data);\n}}\n", struct_name, struct_qt_name, struct_name.to_snake_case()))?;
    }

    Ok(())
}

///
/// Generate the PU structure
///
fn generate_pu_struct(f: &mut File, api_def: &ApiDef) -> io::Result<()> {
    f.write_all(b"///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n")?;
    f.write_all(b"static struct PU s_pu = {\n")?;

    for sdef in api_def.entries.iter().filter(|s| !s.is_pod()) {
        f.write_fmt(format_args!("    create_{},\n", sdef.name.to_snake_case()))?;
    }

    f.write_all(b"};\n\n")
}

///
/// This is the main entry for generating the C/C++ buindings for Qt. The current API mimics Qt
/// fairly closely when it comes to names but at the start there is a map setup that used used
/// to translate between some struct names to fit the Qt version. If more structs gets added that
/// needs to be translated into another Qt name they needs to be added here as well
///
pub fn generate_qt_bindings(filename: &str,
                            header_filename: &str,
                            api_def: &ApiDef)
                            -> io::Result<()> {
    let mut f = File::create(filename)?;
    let mut header_file = File::create(header_filename)?;

    let mut signals_info = HashMap::new();

    let mut struct_name_map = HashMap::new();

    struct_name_map.insert("Button", "AbstractButton");

    f.write_all(b"#include \"c_api.h\"\n")?;
    f.write_all(b"#include \"qt_api_gen.h\"\n")?;

    generate_includes(&mut f, &struct_name_map, api_def)?;

    f.write_all(HEADER)?;

    build_signal_wrappers_info(&mut signals_info, api_def);

    header_file
        .write_all(b"#pragma once\n#include <QObject>\n\n")?;
    generate_signal_wrappers(&mut header_file, &signals_info)?;

    // generate wrapper functions

    for sdef in &api_def.entries {
        generate_struct_body_recursive(&mut f, &sdef.name, &sdef, &struct_name_map, api_def)?;
    }

    generate_struct_defs(&mut f, api_def)?;
    generate_create_functions(&mut f, &struct_name_map, api_def)?;
    generate_pu_struct(&mut f, api_def)?;

    f.write_all(FOOTER)
}
