use std::fs::File;
use std::path::Path;
use std::io::prelude::*;
use pest::prelude::*;

impl_rdp! {
    grammar! {
        chunk = { block* ~ eoi }

        block = {
            whitespace |
            comment |
            structdef
        }

        structdef   =  { name ~ derive? ~ ["{"] ~ fieldlist? ~ ["}"] }
        fieldlist   =  { field ~ (fieldsep ~ field)* ~ fieldsep* }
        field       =  { var | function }
        fieldsep    = _{ [","] }

        ftype       = { name }
        rettype     = { name }
        derive      = { [":"] ~ name }
        callback    = { ["[callback]"] }
        retexp      = { ["->"] ~ name }
        var         = { ftype ~ name }
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
        process(&self) -> () {
            (_: chunk) => {
                println!("chunk");
            },

            (_: block) => {
                println!("block");
            },

            (_: structdef, &name: name) => {
                println!("struct name {}", name);
                //(name, None)
            },

            (_: structdef, &name: name, &derive: derive) => {
                println!("struct name {}", name);
                println!("struct name {}", derive);
                //(name, Some(derive))
            },

            (&name: name) => {
                println!("{}", name);
                //(name, None)
            },

            () => (), //("", None)
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
    pub return_val: Option<Variable<'a>>,
}

#[derive(Debug)]
pub enum StructEntry<'a> {
    Var(Variable<'a>),
    Function(Function<'a>),
}

#[derive(Debug)]
pub struct Struct<'a> {
    pub name: &'a str,
    pub inharit: Option<&'a str>,
    pub entries: Vec<StructEntry<'a>>,
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

        parser.process();

        for token in parser.queue() {
            println!("{:?}", token);

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
        }

        // build up tokens here
    }
}
