use std::fs::File;
use std::path::Path;
use std::io::prelude::*;
use pest::prelude::*;
use std::collections::VecDeque;

#[derive(Debug)]
pub struct Variable<'a> {
    pub name: &'a str,
    pub vtype: &'a str,
}

#[derive(Debug)]
pub struct Function<'a> {
    pub name: &'a str,
    pub function_args: Vec<Variable<'a>>,
    pub return_val: Option<&'a str>,
    pub callback: bool,
}

#[derive(Debug)]
pub enum StructEntry<'a> {
    Var(Variable<'a>),
    Function(Function<'a>),
}

#[derive(Debug)]
pub struct Struct<'a> {
    pub name: &'a str,
    pub inherit: Option<&'a str>,
    pub entries: Vec<StructEntry<'a>>,
}

#[derive(Debug)]
pub struct ApiDef<'a> {
    pub text: String,
    pub entries: Vec<Struct<'a>>,
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
        process(&self) -> Vec<Struct<'input>> {
            (structs: _chunk()) => {
                structs.into_iter().collect::<Vec<_>>()
            },
        }

        _chunk(&self) -> VecDeque<Struct<'input>> {
            (_: structdef, sdef: _structdef(), mut tail: _chunk()) => {
                tail.push_front(sdef);
                tail
            },

            () => VecDeque::new(),
        }

        _structdef(&self) -> Struct<'input> {
            (&name: name, inherit: _derive(), list: _fieldentries()) => {
                Struct {
                    name: name,
                    inherit: inherit,
                    entries: list.into_iter().collect::<Vec<_>>(),
                }
            },
        }

        _derive(&self) -> Option<&'input str> {
            (_: derive, &name: name) => Some(name),
            () => None,
        }

        _fieldentries(&self) -> VecDeque<StructEntry<'input>> {
            (_: field, entry: _fieldentry(), mut tail: _fieldentries()) => {
                tail.push_front(entry);
                tail
            },

            (_: fieldlist, entries: _fieldentries()) => entries,
            () => VecDeque::new(),
        }

        _fieldentry(&self) -> StructEntry<'input> {
            (_: function, func: _funcentry()) => func,

            (_: var, &vtype: name, &name: name) => {
                StructEntry::Var(Variable {
                    name: name,
                    vtype: vtype,
                })
            },
        }

        _funcentry(&self) -> StructEntry<'input> {
            (callback: _callback(), &name: name, func_args: _func_args_list(), ret_value: _returnvalue()) => {
                StructEntry::Function(Function {
                    name: name,
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

        _returnvalue(&self) -> Option<&'input str> {
            (_: retexp, &name: name) => Some(name),
            () => None,
        }

        _func_args_list(&self) -> VecDeque<Variable<'input>> {
            (_: var, &name: name, &atype: name, mut tail: _func_args_list()) => {
                tail.push_front(Variable {
                    name: name,
                    vtype: atype,
                });

                tail
            },

            (_: varlist, tail: _func_args_list()) => tail,
            () => VecDeque::new(),
        }
    }
}

impl<'a> ApiDef<'a> {
    pub fn parse_file<P: AsRef<Path>>(&'a mut self, path: P) {
        let mut file = File::open(path).unwrap();
        file.read_to_string(&mut self.text).unwrap();

        let mut parser = Rdp::new(StringInput::new(&self.text));

        parser.chunk();

        if !parser.end() {
            let expected = parser.expected();
            panic!("Failed to parse {:?} - {}", parser.expected(), &self.text[expected.1..]);
        }

        self.entries = parser.process();

        println!("API {:?}", self.entries);
    }
}
