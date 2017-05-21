use std::fs::File;
use std::path::Path;
use std::io::prelude::*;
use pest::prelude::*;

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
                structs
            },
        }

        _chunk(&self) -> Vec<Struct<'input>> {
            (_: structdef, sdef: _structdef(), mut tail: _chunk()) => {
                println!("push push");
                tail.push(sdef);
                tail
            },

            () => Vec::new(),
        }

        _structdef(&self) -> Struct<'input> {
            (&name: name, inherit: _derive(), list: _fieldlist()) => {
                let t = Struct {
                    name: name,
                    inherit: inherit,
                    entries: list,
                };

                println!("Created struct {:?}", t);

                t
            },
        }

        _derive(&self) -> Option<&'input str> {
            (_: derive, &name: name) => Some(name),
            () => None,
        }

        _fieldlist(&self) -> Vec<StructEntry<'input>> {
            (_: fieldlist, entries: _fieldentries()) => {
                entries
            },

            () => Vec::new(),
        }

        _fieldentries(&self) -> Vec<StructEntry<'input>> {
            (_: field, entry: _fieldentry(), mut tail: _fieldentries()) => {
                println!("_fieldentries");
                tail.push(entry);
                tail
            },

            () => {
                println!("_fieldentries::new");
                Vec::new()
            }
        }

        _fieldentry(&self) -> StructEntry<'input> {
            (_: function, func: _funcentry()) => {
                func
            },

            (_: var, &vtype: name, &name: name) => {
                StructEntry::Var(Variable {
                    name: name,
                    vtype: vtype,
                })
            },
        }

        _funcentry(&self) -> StructEntry<'input> {
            (&name: name, callback: _callback(), func_args: _func_args_list(), ret_value: _returnvalue()) => {
                StructEntry::Function(Function {
                    name: name,
                    function_args: func_args,
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

        _func_args_list(&self) -> Vec<Variable<'input>> {
            (_: var, &name: name, &atype: name, mut tail: _func_args_list()) => {
                tail.push(Variable {
                    name: name,
                    vtype: atype,
                });

                tail
            },

            (_: varlist, tail: _func_args_list()) => {
                tail
            },

            () => {
                println!("_func_args_list::new");
                Vec::new()
            }
        }
    }
}

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

impl<'a> Struct<'a> {
    fn new() -> Struct<'a> {
        Struct {
            name: "",
            inherit: None,
            entries: Vec::new(),
        }
    }
}


#[derive(Debug)]
pub struct ApiDef<'a> {
    pub text: String,
    pub entries: Vec<Struct<'a>>,
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

        //let mut struct_def = Struct::new();
        //let mut state = State::StructDef;

        // We could do this using process! instead the parser instead but as the rules
        // are quite simple we just do a basic state machine istead

        parser.process();

        for token in parser.queue() {
            println!("{:?}", token);
        }

        /*
        let mut queue = parser.queue().iter();

        loop {
            match queue.next() {
                Some(token) => {
                    match token.rule {
                        Rule::structdef => {
                            let mut struct_def = Struct::new();
                            let t: i32 = queue;
                        }

                        _ => (),

                    }
                }

                None => { break },
            }
        }
        */

        /*

        for token in queue {
            println!("{:?}", token);

            match token.rule {
                Rule::name => {
                    match state {
                        State::StructDef => struct_def.name = &self.text[token.start..token.end],
                        State::Derive => struct_def.inherit = Some(&self.text[token.start..token.end]),
                        _ => (),
                    }
                }

                Rule::structdef => {
                    println!("{:?}", struct_def);
                    struct_def = Struct::new();
                    state = State::StructDef;
                },


                Rule::derive => state = State::Derive,

                _ => (),
            }
        */

            /*
            match token.rule {
                Rule::structdef => {
                    let t = parser.process_struct();
                    println!("{:?}", t);
                }

                _ => (),
            }

            let s = Struct {
                name: &self.text[0..3],
                inharit: None,
                entries: Vec::new(),
            };

            self.entries.push(s);
            */

            /*
            match token.rule {
            }
            */
        //}

        // build up tokens here
    }
}
