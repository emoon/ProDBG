use std::error::Error as StdError;
use std::fmt;

/// Errors that can be return from various operatiors
///
#[derive(Debug)]
pub enum Error {
    /// Size of Workspace is incorrect
    IllegalSize(String),
}

impl StdError for Error {
    fn description(&self) -> &str {
        match *self {
            Error::IllegalSize(_) => "Illegal Size",
        }
    }

    fn cause(&self) -> Option<&StdError> {
        match *self {
            Error::IllegalSize(_) => None,
        }
    }
}

impl fmt::Display for Error {
    fn fmt(&self, fmt: &mut fmt::Formatter) -> fmt::Result {
        match *self {
            Error::IllegalSize(ref name) => write!(fmt, "{} {}", self.description(), name),
        }
    }
}

