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
        let mut source_line = 0u32;
        let mut was_over = false;

        for line in lines {
            if line.offset == offset {
                println!("Matching source {} line {}", filename, line.line);
                return Some((filename.to_owned(), line.line));
            } if line.offset <= offset {
                source_line = line.line;
            } else if line.offset > offset {
                was_over = true;
            }
        }

        if was_over {
            println!("Partial Matching source {} line {}", filename, source_line);
            Some((filename.to_owned(), source_line))
        } else {
            None
        }
    }

    pub fn resolve_file_line(&self, offset: u32, seg_id: u32) -> Option<(String, u32)> {
        if seg_id >= self.hunks.len() as u32 {
            return None; 
        }

        let hunk = &self.hunks[seg_id as usize];

        if let Some(ref source_files) = hunk.line_debug_info {
            for src_file in source_files {
                //if offset > src_file.base_offset {
                //    continue;
                //}

                if let Some(data) = Self::try_find_line(&src_file.name, &src_file.lines, offset) {
                    return Some(data);
                }
            }
        }

        None
    }

    pub fn get_address_seg(&self, filename: &str, file_line: u32) -> Option<(u32, u32)> {
        for (i, hunk) in self.hunks.iter().enumerate() {
            if let Some(ref source_files) = hunk.line_debug_info {
                for src_file in source_files {
                    if src_file.name != filename {
                        continue;
                    }

                    for line in &src_file.lines {
                        if line.line == file_line {
                            return Some((i as u32, line.offset));
                        }
                    }
                }
            }
        }

        None
    }
}
