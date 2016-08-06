#[macro_use]
extern crate json;
pub mod error;

use json::JsonValue;
use std::fs::File;
use std::io::Read;
use std::io::Write;

use error::ProjectError;

pub struct Project {
    pub backend_name: String,
    pub backend_data: Option<Vec<String>>,
    pub layout_data: String,
}

impl Project {
    pub fn new() -> Project {
        Project {
            backend_name: "Dummy Backend".to_owned(), // dummy as default
            backend_data: None,
            layout_data: "".to_owned(),
        }
    }

    pub fn load(filename: &str) -> Result<Project, ProjectError> {
        let mut file = try!(File::open(filename));

        let mut buffer = String::new();
        try!(file.read_to_string(&mut buffer));

        let json_data = try!(json::parse(&buffer));

        Ok(Project {
            backend_name: Self::get_string_value(&json_data, "backend_name"),
            backend_data: Self::get_array_value(&json_data, "backend_data"),
            layout_data: Self::get_string_value(&json_data, "layout_data"),
        })
    }

    pub fn save(&self, filename: &str) -> Result<(), ProjectError> {
        let mut file = try!(File::create(filename));

        let data = object!{
            "backend_name" => self.backend_name.as_str(),
            "backend_data" => self.get_backend_data(),
            "layout_data" => self.layout_data.as_str()
        };

        let json_data = data.dump();

        try!(file.write_all(json_data.as_bytes()));

        Ok(())
    }

    fn get_backend_data(&self) -> json::JsonValue {
        if self.backend_data.is_none() {
            json::JsonValue::Null
        } else {
            let mut data = json::JsonValue::new_array();

            for t in self.backend_data.as_ref().unwrap() {
                data.push(t.to_owned()).unwrap();
            }

            data
        }
    }

    fn get_string_value(json_data: &JsonValue, id: &str) -> String {
        let ref v = json_data[id];
        if v.is_null() {
            "".to_owned()
        } else {
            v.as_str().unwrap().to_owned()
        }
    }

    fn get_array_value(json_data: &JsonValue, id: &str) -> Option<Vec<String>> {
        let ref v = json_data[id];
        if v.is_null() {
            None
        } else {
            let mut data = Vec::with_capacity(v.len());
            for t in v.members() {
                data.push(t.as_str().unwrap().to_owned());
            }
            Some(data)
        }
    }
}
