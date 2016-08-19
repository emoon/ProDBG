use std::path::Path;
use amiga_hunk_parser::{Hunk, HunkParser, SourceLine};

pub struct DebugInfo {
    pub hunks: Vec<Hunk>,
}

impl DebugInfo {
    pub fn new() -> DebugInfo {
        DebugInfo {
            hunks: Vec::new(),
        }
    }

    pub fn load_info(&mut self, uae_path: &str, amiga_exe: &str) {
        // TODO: Not assume dhx: path
        let path = Path::new(uae_path).join(&amiga_exe[4..]);
        println!("Trying debug data from {:?}", path);
        if let Ok(hunks) = HunkParser::parse_file(path.to_str().unwrap()) {
            println!("Loading ok!");
            self.hunks = hunks;
        }
    }

    fn try_find_line(filename: &str, 
                     lines: &Vec<SourceLine>, 
                     offset: u32) -> Option<(String, u32)> {
        for line in lines {
            if line.offset == offset {
                return Some((filename.to_owned(), line.line));
            }
        }

        None
    }

    pub fn resolve_file_line(&self, offset: u32, seg_id: u32) -> Option<(String, u32)> {
        if seg_id >= self.hunks.len() as u32 {
            return None; 
        }

        let hunk = &self.hunks[seg_id as usize];

        if let Some(ref source_files) = hunk.line_debug_info {
            for src_file in source_files {
                if let Some(data) = Self::try_find_line(&src_file.name, &src_file.lines, offset) {
                    return Some(data);
                }
            }
        }

        None
    }
}
