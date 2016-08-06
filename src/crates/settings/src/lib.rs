#[macro_use]
extern crate json;

use json::JsonValue;
use std::fs::File;
use std::io::Read;

pub mod error;
use error::SettingsError;

#[derive(Debug)]
pub struct Settings {
    default_values: JsonValue,
    user_values: JsonValue,
}

impl Settings {
    pub fn new() -> Settings {
        Settings {
            default_values: JsonValue::new_object(),
            user_values: JsonValue::new_object(),
        }
    }

    fn load_file(filename: &str) -> Result<JsonValue, SettingsError> {
        let mut file = try!(File::open(filename));

        let mut buffer = String::new();
        try!(file.read_to_string(&mut buffer));

        let json_data = try!(json::parse(&buffer));

        Ok(json_data)
    }

    pub fn load_default_settings(&mut self, filename: &str) -> Result<(), SettingsError> {
        let data = try!(Self::load_file(filename));

        self.default_values = data;

        Ok(())
    }

    pub fn load_user_settings(&mut self, filename: &str) -> Result<(), SettingsError> {
        let data = try!(Self::load_file(filename));

        self.user_values = data;

        Ok(())
    }

    pub fn get_int(&self, category: &str, key: &str) -> Option<i32> {
        let ref v = self.user_values[category][key];

        if !v.is_null() {
            return v.as_i32();
        }

        let ref v = self.default_values[category][key];

        if !v.is_null() {
            return v.as_i32();
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
        assert_eq!(Settings::load_file("../../../test_data/settings.json").is_ok(),
                   true);
    }

    #[test]
    fn test_json_get_int() {
        let mut settings = Settings::new();

        settings.load_default_settings("../../../test_data/settings.json").unwrap();

        assert_eq!(settings.get_int("window_size", "width"), Some(1024));
        assert_eq!(settings.get_int("window_size", "some_new_data"), None);
    }

    #[test]
    fn test_json_get_int_updated() {
        let mut settings = Settings::new();

        settings.load_default_settings("../../../test_data/settings.json").unwrap();
        settings.load_user_settings("../../../test_data/user_settings.json").unwrap();

        assert_eq!(settings.get_int("window_size", "width"), Some(1920));
        assert_eq!(settings.get_int("window_size", "hsoeu"), None);
        assert_eq!(settings.get_int("window_size", "some_new_data"), Some(42));
    }
}
