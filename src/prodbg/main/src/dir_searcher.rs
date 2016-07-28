//! Provides functions to search for working dir
use std::fs;
use std::ffi::OsStr;
use std::path::{Path, PathBuf};


fn is_data_dir(entry: &fs::DirEntry) -> bool {
    if entry.file_name() != OsStr::new("data") {
        return false;
    }
    entry.path().is_dir()
}

fn is_working_dir(dir: &Path) -> bool {
    match fs::read_dir(dir) {
        Ok(mut reader) => {
            reader.any(|el| match el {
                Ok(ref entry) => is_data_dir(entry),
                _ => false,
            })
        }
        _ => false,
    }
}

/// Finds directory that contains "data" directory. Starts with current working directory and
/// traverses up to the root.
pub fn find_working_dir() -> Option<PathBuf> {
    ::std::env::current_dir()
        .ok()
        .and_then(|dir| fs::canonicalize(dir).ok())
        .and_then(|cur_dir| {
            let mut cur_check_dir = cur_dir.as_path();
            loop {
                if is_working_dir(cur_check_dir) {
                    return Some(cur_check_dir.to_path_buf());
                }
                cur_check_dir = match cur_check_dir.parent() {
                    Some(dir) => dir,
                    None => return None,
                }
            }
        })
}
