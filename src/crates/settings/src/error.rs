use std::io;
use std::fmt;
use std::error::Error;
use json;

#[derive(Debug)]
pub enum SettingsError {
    Io(io::Error),
    Json(json::JsonError),
}

impl fmt::Display for SettingsError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            SettingsError::Io(ref err) => err.fmt(f),
            SettingsError::Json(ref err) => err.fmt(f),
        }
    }
}

impl Error for SettingsError {
    fn description(&self) -> &str {
        match *self {
            SettingsError::Io(ref err) => err.description(),
            SettingsError::Json(ref err) => err.description(),
        }
    }
}

impl From<json::JsonError> for SettingsError {
    fn from(err: json::JsonError) -> SettingsError {
        SettingsError::Json(err)
    }
}

impl From<io::Error> for SettingsError {
    fn from(err: io::Error) -> SettingsError {
        SettingsError::Io(err)
    }
}
