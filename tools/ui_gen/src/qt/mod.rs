//mod signal_wrappers;

use std::io;
use std::fs::File;
use std::io::Write;
use std::collections::HashMap;
use ::api_parser::*;
use ::c_api_gen::get_type_name;
use ::c_api_gen::generate_c_function_args;

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

    input_args.push_str("void*");   // user_data

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

        f.write_fmt(format_args!("typedef void (*{})({});\n\n", signal_type_name, generate_c_function_args(func)))?;

        f.write_fmt(format_args!("class QSlotWrapper{} : public QObject {{\n    Q_OBJECT\npublic:\n", signal_type_name))?;
        f.write_fmt(format_args!("    QSlotWrapper{}(void* data, {} func) {{\n", signal_type_name, signal_type_name))?;
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

            f.write_fmt(format_args!("{} {}) {{\n", get_type_name(&func.function_args[args_count - 1]), func.function_args[args_count - 1].name))?;
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

        f.write_all(b"////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n\n")?;
    }

    Ok(())
}

pub fn generate_qt_bindings(filename: &str, api_def: &ApiDef) -> io::Result<()> {
    let mut f = File::create(filename)?;
    let mut signals_info = HashMap::new();

    f.write_all(b"#include <QObject>\n\n")?;

    build_signal_wrappers_info(&mut signals_info, api_def);

    generate_signal_wrappers(&mut f, &signals_info)
}

