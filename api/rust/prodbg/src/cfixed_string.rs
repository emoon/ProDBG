use std::ffi::CString;
use std::mem;
use std::ptr;

const STRING_SIZE: usize = 512;

pub struct CFixedString {
    pub local_string: [i8; STRING_SIZE],
    pub heap_string: Option<CString>,
}

impl CFixedString {
    pub fn from_str(name: &str) -> CFixedString {
        unsafe {
            let name_len = name.len();
            if name_len <= STRING_SIZE - 1 {
                let mut handler = CFixedString {
                    local_string: mem::uninitialized(),
                    heap_string: None,
                };

                ptr::copy(name.as_ptr(),
                          handler.local_string.as_mut_ptr() as *mut u8,
                          name_len);
                handler.local_string[name_len] = 0;

                handler
            } else {
                CFixedString {
                    local_string: mem::uninitialized(),
                    heap_string: Some(CString::new(name).unwrap()),
                }
            }
        }
    }

    pub fn as_ptr(&mut self) -> *const i8 {
        if self.heap_string == None {
            self.local_string.as_ptr()
        } else {
            self.heap_string.as_mut().unwrap().as_ptr()
        }
    }
}

#[cfg(test)]
mod tests {
    use std::ffi::CStr;
    use super::*;

    fn get_string(handler: &mut CFixedString) -> String {
        unsafe { CStr::from_ptr(handler.as_ptr()).to_string_lossy().into_owned() }
    }

    #[test]
    fn test_empty_handler() {
        let short_string = "";
        let mut t = CFixedString::from_str(short_string);
        assert_eq!(t.heap_string.is_none(), true);
        assert_eq!(get_string(&mut t), short_string);
    }


    #[test]
    fn test_short_1() {
        let short_string = "test_local";
        let mut t = CFixedString::from_str(short_string);
        assert_eq!(t.heap_string.is_none(), true);
        assert_eq!(get_string(&mut t), short_string);
    }

    #[test]
    fn test_short_2() {
        let short_string = "test_local stoheusthsotheost";
        let mut t = CFixedString::from_str(short_string);
        assert_eq!(t.heap_string.is_none(), true);
        assert_eq!(get_string(&mut t), short_string);
    }

    #[test]
    fn test_511() {
        // this string (with 511) buffer should just fit
        let test_511_string = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeuuuuuuuuuuuuu\
                                uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu\
                                uuuueeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeaaaaaaaaaaaa\
                                aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaoooooooooooooooooooooooooooooo\
                                oooooooooooooooooooooooooooooooooeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\
                                eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeuuuuuuuuuuuuuuuuuuuuuuuuuu\
                                uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuoooooooooooooooooooooooooooooo\
                                oooooooooooooooooooooooooooooooooooooooacd";
        let mut t = CFixedString::from_str(test_511_string);
        assert_eq!(t.heap_string.is_none(), true);
        assert_eq!(get_string(&mut t), test_511_string);
    }

    #[test]
    fn test_512() {
        // this string (with 512) buffer should no fit
        let test_512_string = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeuuuuuuuuuuuuu\
                                uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu\
                                uuuueeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeaaaaaaaaaaaa\
                                aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaoooooooooooooooooooooooooooooo\
                                oooooooooooooooooooooooooooooooooeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\
                                eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeuuuuuuuuuuuuuuuuuuuuuuuuuu\
                                uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuoooooooooooooooooooooooooooooo\
                                oooooooooooooooooooooooooooooooooooooooabcd";
        let mut t = CFixedString::from_str(test_512_string);
        assert_eq!(t.heap_string.is_some(), true);
        assert_eq!(get_string(&mut t), test_512_string);
    }

    #[test]
    fn test_513() {
        // this string (with 513) buffer should no fit
        let test_513_string = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeuuuuuuuuuuuuu\
                                uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu\
                                uuuueeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeaaaaaaaaaaaa\
                                aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaoooooooooooooooooooooooooooooo\
                                oooooooooooooooooooooooooooooooooeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\
                                eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeuuuuuuuuuuuuuuuuuuuuuuuuuu\
                                uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuoooooooooooooooooooooooooooooo\
                                oooooooooooooooooooooooooooooooooooooooabcd";
        let mut t = CFixedString::from_str(test_513_string);
        assert_eq!(t.heap_string.is_some(), true);
        assert_eq!(get_string(&mut t), test_513_string);
    }
}


