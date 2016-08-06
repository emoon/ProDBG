use std::io;
use std::fmt;
use std::error::Error;
use json;

#[derive(Debug)]
pub enum ProjectError {
    Io(io::Error),
    Json(json::JsonError),
}

impl fmt::Display for ProjectError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            ProjectError::Io(ref err) => err.fmt(f),
            ProjectError::Json(ref err) => err.fmt(f),
        }
    }
}

impl Error for ProjectError {
    fn description(&self) -> &str {
        match *self {
            ProjectError::Io(ref err) => err.description(),
            ProjectError::Json(ref err) => err.description(),
        }
    }
}

impl From<json::JsonError> for ProjectError {
    fn from(err: json::JsonError) -> ProjectError {
        ProjectError::Json(err)
    }
}

impl From<io::Error> for ProjectError {
    fn from(err: io::Error) -> ProjectError {
        ProjectError::Io(err)
    }
}

