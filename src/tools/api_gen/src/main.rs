//
// api_gen: An API file generator for ProDBG
//
extern crate codespawn;
use codespawn::fmt_code::{Lang};
use std::env;
use std::path::Path;
use std::io::Error as IoError;
use std::io::ErrorKind::Other as ParseError;
use std::error::Error;

macro_rules! api_gen {
    ($x:expr) => {
        if let Some(input_ext) = Path::new($x).extension() {
            let parsed_object = match input_ext.to_str() {
                Some("json") => codespawn::from_json($x),
                Some("xml")  => codespawn::from_xml($x),
                _  => Err(IoError::new(ParseError, format!("!!! api_gen: unrecognized input file extension: {}", input_ext.to_str().unwrap()))),
            };
            parsed_object
        }
        else {
            Err(IoError::new(ParseError, format!("!!! api_gen: unrecognized input file extension")))
        }
    };
}

fn main() {
    let exit_code = run_main();
    std::process::exit(exit_code);
}

fn run_main() -> i32 {
    let args: Vec<String> = env::args().collect();

    let mut input_file   = String::from("");
    let mut output_files = Vec::<(Lang, String)>::new();
    let num_args = args.len();
    
    for i in 0..num_args {
        if args[i] == "-i" && i < num_args - 1 {
            if '-' != args[i+1].as_bytes()[0] as char {
                input_file = args[i+1].clone();
            }
        }
        if args[i] == "-rs" && i < num_args - 1 {
            if '-' != args[i+1].as_bytes()[0] as char {
                output_files.push((Lang::Rust, args[i+1].clone()));
            }
        }
        if args[i] == "-cpp" && i < num_args - 1 {
            if '-' != args[i+1].as_bytes()[0] as char {
                output_files.push((Lang::Cpp, args[i+1].clone()));
            }
        }
    }

    if input_file.is_empty() || output_files.is_empty() {
        println!("Usage: api_gen -i <definition.xml/json> -rs <rust_output_file> -cpp <cpp_output_file>");
        return 1;
    }
    else {
        println!("* api_gen: using: {}", input_file);
        let raw_code = api_gen!(&input_file);
        match raw_code {
            Err(why) => {
                println!("{}", why.description());
                return 1;
            },
            _ => {},
        }

        let raw_code = raw_code.unwrap();
        for lang in output_files {
            println!("* api_gen: generating {} ({})", lang.1, lang.0);
            let file_saved = match lang.0 {
                Lang::Rust => raw_code.to_rust().to_file(&lang.1),
                Lang::Cpp  => raw_code.to_cpp().to_file(&lang.1),
            };

            match file_saved {
                Err(why) => {
                    println!("!!! api_gen: could not generate {}: {}", input_file, why.description());
                    return 1;
                },
                _ => {},
            }
        }
        println!("* api_gen: done!");
    }

    0
}
