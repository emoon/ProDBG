#[macro_use]
extern crate lazy_static;
extern crate json;

use json::JsonValue;
use std::fs::File;
use std::io::Read;

pub mod error;
use error::SettingsError;

#[derive(Debug)]
pub struct Key {
    pub name: String,
    pub key_id: u32,
}

#[derive(Debug)]
pub struct Setting {
    pub name: String,
    pub value: JsonValue,
}

#[derive(Debug)]
pub struct Category {
    pub name: String,
    pub keys: Vec<Key>,
    pub values: Vec<Setting>,
}

#[derive(Debug)]
pub struct Settings {
    pub categories: Vec<Category>,
}

impl Setting {
    fn new(name: &String, json_data: &JsonValue) -> Setting {
        Setting {
            name: name.clone(),
            value: json_data.clone(),
        }
    }

    fn update(&mut self, json_data: &JsonValue) {
        self.value = json_data.clone();
    }
}

impl Category {
    pub fn new(name: &String, json_data: &JsonValue) -> Category {
        let mut cat = Category {
            name: name.clone(),
            keys: Vec::new(),
            values: Vec::new(),
        };

        cat.update(name, json_data);

        cat
    }

    fn update_entry(&mut self, entry: (&String, &JsonValue)) -> bool {
        if let Some(value) = self.values.iter_mut().find(|ref mut e| e.name == *entry.0) {
            value.update(entry.1);
            true
        } else {
            false
        }
    }

    fn update(&mut self, _name: &String, json_data: &JsonValue) {
        for entry in json_data.entries() {
            if !self.update_entry(entry) {
                self.values.push(Setting::new(entry.0, entry.1));
            }
        }
    }
}

impl Settings {
    pub fn new() -> Settings {
        Settings {
            categories: Vec::new(),
        }
    }

    fn load_file(filename: &str) -> Result<JsonValue, SettingsError> {
        let mut file = try!(File::open(filename));

        let mut buffer = String::new();
        try!(file.read_to_string(&mut buffer));

        let json_data = try!(json::parse(&buffer));

        Ok(json_data)
    }

    fn update_entry(&mut self, entry: (&String, &JsonValue)) -> bool {
        if let Some(value) = self.categories.iter_mut().find(|ref e| e.name == *entry.0) {
            value.update(entry.0, entry.1);
            true
        } else {
            false
        }
    }

    fn update_settings(&mut self, data: &JsonValue) {
        for entry in data.entries() {
            if !self.update_entry(entry) {
                self.categories.push(Category::new(entry.0, entry.1));
            }
        }
    }

    pub fn update(&mut self, filename: &str) -> Result<JsonValue, SettingsError> {
        let data = try!(Self::load_file(filename));

        self.update_settings(&data);

        Ok(data)
    }

    pub fn get_int(&self, category: &str, key: &str) -> Option<i32> {
        for cat in &self.categories {
            if cat.name == category {
                if let Some(ref t) = cat.values.iter().find(|ref s| s.name == key) {
                    return t.value.as_i32()
                }
            }
        }

        None
    }
}


#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_fail_load() {
        assert_eq!(Settings::load_file("uknown_file").is_err(), true);
    }

    #[test]
    fn test_json_parse_fail() {
        assert_eq!(Settings::load_file("Cargo.toml").is_err(), true);
    }

    #[test]
    fn test_json_parse_ok() {
        assert_eq!(Settings::load_file("../../../test_data/settings.json").is_ok(), true);
    }

    #[test]
    fn test_json_traverse_top() {
        let mut settings = Settings::new();

        settings.update("../../../test_data/settings.json").unwrap();

        let cats = [
            "Source Code View",
            "default_native_backend",
            "window_position",
            "window_size",
        ];

        for (i, cat) in settings.categories.iter().enumerate() {
            assert_eq!(cat.name, cats[i]);
        }
    }

    #[test]
    fn test_json_get_int() {
        let mut settings = Settings::new();

        settings.update("../../../test_data/settings.json").unwrap();

        assert_eq!(settings.get_int("window_size", "width"), Some(1024));
        assert_eq!(settings.get_int("window_size", "some_new_data"), None);
    }

    #[test]
    fn test_json_get_int_updated() {
        let mut settings = Settings::new();

        settings.update("../../../test_data/settings.json").unwrap();
        settings.update("../../../test_data/user_settings.json").unwrap();

        assert_eq!(settings.get_int("window_size", "width"), Some(1920));
        assert_eq!(settings.get_int("window_size", "hsoeu"), None);
        assert_eq!(settings.get_int("window_size", "some_new_data"), Some(42));
    }
}


