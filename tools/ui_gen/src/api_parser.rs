use std::fs::File;
use std::path::Path;
use std::io::prelude::*;
use pest::prelude::*;
use std::collections::VecDeque;

#[derive(Debug)]
pub struct Variable {
    pub name: String,
    pub vtype: String,
    pub primitive: bool,
}

#[derive(Debug)]
pub struct Function {
    pub name: String,
    pub function_args: Vec<Variable>,
    pub return_val: Option<String>,
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
    if name == "u8" || name == "u8" || 
       name == "i16" || name == "u16" || 
       name == "i32" || name == "u32" ||
       name == "i64" || name == "u64" ||
       name == "f32" || name == "f64" {
        true
    } else {
        false
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
            (callback: _callback(), &name: name, func_args: _func_args_list(), ret_value: _returnvalue()) => {
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

        _returnvalue(&self) -> Option<String> {
            (_: retexp, &name: name) => Some(name.to_owned()),
            () => None,
        }

        _func_args_list(&self) -> VecDeque<Variable> {
            (_: var, &name: name, &atype: name, mut tail: _func_args_list()) => {
                tail.push_front(Variable {
                    name: name.to_owned(),
                    vtype: atype.to_owned(),
                    primitive: is_primitve(atype),
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
            panic!("Failed to parse file - {:?} - {}", parser.expected(), &text[expected.1..]);
        }

        ApiDef {
            entries: parser.process(),
        }
    }
}
