use std::fs::File;
use std::io::Write;
use std::io::prelude::*;
use std::io;
use std::path::Path;
use std::collections::VecDeque;
use pest::prelude::*;

#[derive(Debug, Clone)]
pub struct Variable {
    pub name: String,
    pub vtype: String,
    pub primitive: bool,
}

#[derive(Debug, Clone)]
pub struct Function {
    pub name: String,
    pub function_args: Vec<Variable>,
    pub return_val: Option<Variable>,
    pub callback: bool,
}

#[derive(Debug)]
pub enum StructEntry {
    Var(Variable),
    Function(Function),
}

#[derive(Debug)]
pub struct Struct {
    pub name: String,
    pub inherit: Option<String>,
    pub entries: Vec<StructEntry>,
}

#[derive(Debug)]
pub struct ApiDef {
    pub entries: Vec<Struct>,
}

fn is_primitve(name: &str) -> bool {
    if name == "u8" || name == "u8" || name == "i16" || name == "u16" ||
       name == "i32" || name == "u32" || name == "i64" || name == "u64" ||
       name == "f32" || name == "f64" || name == "bool" {
        true
    } else {
        false
    }
}

impl Struct {
    pub fn is_pod(&self) -> bool {
        self.entries
            .iter()
            .all(|e| match *e {
                     StructEntry::Var(ref _var) => true,
                     _ => false,
                 })
    }

    pub fn get_functions<F>(&self, funcs: &mut Vec<Function>, filter: F) 
        where F: Fn(&Func) -> bool {
        for entry in &self.entries {
            match *entry {
                StructEntry::Function(ref func) => 
                    if filter(func) {
                        funcs.push(func.clone());
                    },
                _ => (),
            }
        }
    }

    pub fn get_callback_functions(&self, funcs: &mut Vec<Function>) {
        Self::get_functions(self, funcs, |func| func.callback)
    }

    pub fn get_regular_functions(&self, funcs: &mut Vec<Function>) {
        Self::get_functions(self, funcs, |func| !func.callback)
    }

    pub fn get_all_functions(&self, funcs: &mut Vec<Function>) {
        Self::get_functions(self, funcs, |_| true)
    }
}

impl Variable {
    pub fn get_c_type(&self) -> String {
        let tname = &self.vtype;
        let primitve = self.primitive;

        if tname == "String" {
            return "const char*".to_owned();
        }

        if tname == "self" {
            return "void*".to_owned();
        }

        if primitve {
            if tname == "f32" {
                return "float".to_owned();
            } else if tname == "bool" {
                return "bool".to_owned();
            } else if tname == "f64" {
                return "double".to_owned();
            } else if tname == "i32" {
                return "int".to_owned();
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
}

impl Function {
    pub fn write_c_func_def<F>(&self, f: &mut File, filter: F) -> io::Result<()>
        where F: Fn(usize, &Variable) -> (String, String)
    {
        let arg_count = self.function_args.len();

        for (i, arg) in self.function_args.iter().enumerate() {
            let filter_arg = filter(i, &arg);

            if filter_arg.1 == "" {
                f.write_fmt(format_args!("{}", filter_arg.0))?;
            } else {
                f.write_fmt(format_args!("{} {}", filter_arg.0, filter_arg.1))?;
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

    pub fn write_func_def<F>(&self, f: &mut File, filter: F) -> io::Result<()>
        where F: Fn(usize, &Variable) -> (String, String)
    {
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

        //f.write_all(b")")?;

        if let Some(ref ret_var) = self.return_val {
            let filter_arg = filter(arg_count, &ret_var);
            if filter_arg.1 != "" {
                f.write_fmt(format_args!(" -> {}", filter_arg.1))?;
            }
        }

        Ok(())
    }
}


impl ApiDef {
    fn collect_recursive<F>(funcs: &mut Vec<Function>, api_def: &ApiDef, sdef: &Struct, filter: F)
        where F: Fn(&Func) -> bool {
        if let Some(ref inherit_name) = sdef.inherit {
            for sdef in &api_def.entries {
                if &sdef.name == inherit_name {
                    Self::collect_recursive(funcs, api_def, &sdef);
                }
            }
        }

        sdef.get_functions(funcs, f);
    }

    pub fn collect_functions<F>(&self, sdef: &Struct, filter) 
        where F: Fn(&Func) -> Vec<Function> {

        let mut funcs = Vec::new();

        Self::collect_recursive(&mut funcs, &self, sdef, f);

        funcs
    }

    pub fn collect_all_functions(&self, sdef: &Struct) -> Vec<Function> {
        Self::collect_functions(&self, sdef, |_| true)
    }

    pub fn collect_callback_functions(&self, sdef: &Struct) -> Vec<Function> {
        Self::collect_functions(&self, sdef, |f| f.callback )
    }

    pub fn collect_regular_functions(&self, sdef: &Struct) -> Vec<Function> {
        Self::collect_functions(&self, sdef, |f| !f.callback)
    }
}

impl_rdp! {
    grammar! {
        chunk = _{ soi ~ structdef* ~ eoi }

        structdef   =  { name ~ derive? ~ ["{"] ~ fieldlist? ~ ["}"] }
        fieldlist   =  { field ~ (fieldsep ~ field)* ~ fieldsep* }
        field       =  { var | function }
        fieldsep    = _{ [","] }

        rettype     = { name }
        derive      = { [":"] ~ name }
        callback    = { ["[callback]"] }
        retexp      = { ["->"] ~ name }
        var         = { name ~ name }
        varlist     = { var ~ ([","] ~ var)* }
        function    = { callback? ~ name ~ ["("] ~ varlist? ~ [")"] ~ retexp? }

        name = @{
            (['a'..'z'] | ['A'..'Z'] | ["_"]) ~ (['a'..'z'] | ['A'..'Z'] | ["_"] | ['0'..'9'])*
        }

       comment = _{
            ["//"] ~ (!(["\r"] | ["\n"]) ~ any)* ~ (["\n"] | ["\r\n"] | ["\r"] | eoi)
        }

        whitespace = _{ [" "] | ["\t"] | ["\u{000C}"] | ["\r"] | ["\n"] }
    }

    process! {
        process(&self) -> Vec<Struct> {
            (structs: _chunk()) => {
                structs.into_iter().collect::<Vec<_>>()
            },
        }

        _chunk(&self) -> VecDeque<Struct> {
            (_: structdef, sdef: _structdef(), mut tail: _chunk()) => {
                tail.push_front(sdef);
                tail
            },

            () => VecDeque::new(),
        }

        _structdef(&self) -> Struct {
            (&name: name, inherit: _derive(), list: _fieldentries()) => {
                Struct {
                    name: name.to_owned(),
                    inherit: inherit,
                    entries: list.into_iter().collect::<Vec<_>>(),
                }
            },
        }

        _derive(&self) -> Option<String> {
            (_: derive, &name: name) => Some(name.to_owned()),
            () => None,
        }

        _fieldentries(&self) -> VecDeque<StructEntry> {
            (_: field, entry: _fieldentry(), mut tail: _fieldentries()) => {
                tail.push_front(entry);
                tail
            },

            (_: fieldlist, entries: _fieldentries()) => entries,
            () => VecDeque::new(),
        }

        _fieldentry(&self) -> StructEntry {
            (_: function, func: _funcentry()) => func,

            (_: var, &vtype: name, &name: name) => {
                StructEntry::Var(Variable {
                    name: name.to_owned(),
                    vtype: vtype.to_owned(),
                    primitive: is_primitve(vtype),
                })
            },
        }

        _funcentry(&self) -> StructEntry {
            (callback: _callback(), &name: name, mut func_args: _func_args_list(), ret_value: _returnvalue()) => {
                func_args.push_front(Variable {
                    name: "self_c".to_owned(),
                    vtype: "self".to_owned(),
                    primitive: false,
                });
                // we always have self as first parameter so add it here
                StructEntry::Function(Function {
                    name: name.to_owned(),
                    function_args: func_args.into_iter().collect::<Vec<_>>(),
                    return_val: ret_value,
                    callback: callback,
                })
            }
        }

        _callback(&self) -> bool {
            (_: callback) => true,
            () => false,
        }

        _returnvalue(&self) -> Option<Variable> {
            (_: retexp, &name: name) => {
                Some(Variable {
                    name: "".to_owned(),
                    vtype: name.to_owned(),
                    primitive: is_primitve(name),
                })
            },
            () => None,
        }

        _func_args_list(&self) -> VecDeque<Variable> {
            (_: var, &name: name, &atype: name, mut tail: _func_args_list()) => {
                tail.push_front(Variable {
                    name: atype.to_owned(),
                    vtype: name.to_owned(),
                    primitive: is_primitve(name),
                });

                tail
            },

            (_: varlist, tail: _func_args_list()) => tail,
            () => VecDeque::new(),
        }
    }
}

impl ApiDef {
    pub fn new<P: AsRef<Path>>(path: P) -> ApiDef {
        let mut text = String::new();

        let mut file = File::open(path).unwrap();
        file.read_to_string(&mut text).unwrap();

        let mut parser = Rdp::new(StringInput::new(&text));

        parser.chunk();

        if !parser.end() {
            let expected = parser.expected();
            panic!("Failed to parse file - {:?} - {}",
                   parser.expected(),
                   &text[expected.1..]);
        }

        ApiDef { entries: parser.process() }
    }
}
