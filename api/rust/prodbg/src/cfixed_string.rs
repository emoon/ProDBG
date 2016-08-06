use std::borrow::{Borrow, Cow};
use std::ffi::{CStr, CString};
use std::fmt;
use std::mem;
use std::ops;
use std::os::raw::c_char;
use std::ptr;

const STRING_SIZE: usize = 512;

/// This is a C String abstractions that presents a CStr like
/// interface for interop purposes but tries to be little nicer
/// by avoiding heap allocations if the string is within the
/// generous bounds (512 bytes) of the statically sized buffer.
/// Strings over this limit will be heap allocated, but the
/// interface outside of this abstraction remains the same.
pub enum CFixedString {
    Local {
        s: [c_char; STRING_SIZE],
        len: usize,
    },
    Heap {
        s: CString,
        len: usize,
    },
}

impl CFixedString {
    /// Creates an empty CFixedString, this is intended to be
    /// used with write! or the `fmt::Write` trait
    pub fn new() -> Self {
        unsafe {
            CFixedString::Local {
                s: mem::uninitialized(),
                len: 0,
            }
        }
    }

    pub fn from_str<S: AsRef<str>>(s: S) -> Self {
        Self::from(s.as_ref())
    }

    pub fn as_ptr(&self) -> *const c_char {
        match *self {
            CFixedString::Local { ref s, .. } => s.as_ptr(),
            CFixedString::Heap { ref s, .. } => s.as_ptr(),
        }
    }

    /// Returns true if the string has been heap allocated
    pub fn is_allocated(&self) -> bool {
        match *self {
            CFixedString::Local { .. } => false,
            _ => true,
        }
    }

    /// Converts a `CFixedString` into a `Cow<str>`.
    ///
    /// This function will calculate the length of this string (which normally
    /// requires a linear amount of work to be done) and then return the
    /// resulting slice as a `Cow<str>`, replacing any invalid UTF-8 sequences
    /// with `U+FFFD REPLACEMENT CHARACTER`. If there are no invalid UTF-8
    /// sequences, this will merely return a borrowed slice.
    pub fn to_string(&self) -> Cow<str> {
        String::from_utf8_lossy(&self.to_bytes())
    }

    pub unsafe fn as_str(&self) -> &str {
        use std::slice;
        use std::str;

        match *self {
            CFixedString::Local { ref s, len } => {
                str::from_utf8_unchecked(slice::from_raw_parts(s.as_ptr() as *const u8, len))
            }
            CFixedString::Heap { ref s, len } => {
                str::from_utf8_unchecked(slice::from_raw_parts(s.as_ptr() as *const u8, len))
            }
        }
    }
}

impl<'a> From<&'a str> for CFixedString {
    fn from(s: &'a str) -> Self {
        use std::fmt::Write;

        let mut string = CFixedString::new();

        string.write_str(s).unwrap();

        string
    }
}

impl fmt::Write for CFixedString {
    fn write_str(&mut self, s: &str) -> Result<(), fmt::Error> {
        // use std::fmt::Write;

        unsafe {
            let cur_len = self.as_str().len();

            match cur_len + s.len() {
                len if len <= STRING_SIZE - 1 => {
                    match *self {
                        CFixedString::Local { s: ref mut ls, len: ref mut lslen } => {
                            let ptr = ls.as_mut_ptr() as *mut u8;
                            ptr::copy(s.as_ptr(), ptr.offset(cur_len as isize), s.len());
                            *ptr.offset(len as isize) = 0;
                            *lslen = len;
                        }
                        _ => unreachable!(),
                    }
                }
                len => {
                    let mut heapstring = String::with_capacity(len + 1);

                    heapstring.write_str(self.as_str()).unwrap();
                    heapstring.write_str(s).unwrap();

                    *self = CFixedString::Heap {
                        s: CString::new(heapstring).unwrap(),
                        len: len,
                    };
                }
            }
        }

        // Yah....we should do error handling
        Ok(())
    }
}

impl From<CFixedString> for String {
    fn from(s: CFixedString) -> Self {
        String::from_utf8_lossy(&s.to_bytes()).into_owned()
    }
}

impl ops::Deref for CFixedString {
    type Target = CStr;

    fn deref(&self) -> &CStr {
        use std::slice;

        match *self {
            CFixedString::Local { ref s, len } => unsafe {
                mem::transmute(slice::from_raw_parts(s.as_ptr(), len + 1))
            },
            CFixedString::Heap { ref s, .. } => s,
        }
    }
}

impl Borrow<CStr> for CFixedString {
    fn borrow(&self) -> &CStr {
        self
    }
}

impl AsRef<CStr> for CFixedString {
    fn as_ref(&self) -> &CStr {
        self
    }
}

impl Borrow<str> for CFixedString {
    fn borrow(&self) -> &str {
        unsafe { self.as_str() }
    }
}

impl AsRef<str> for CFixedString {
    fn as_ref(&self) -> &str {
        unsafe { self.as_str() }
    }
}

macro_rules! format_c {
    // This does not work on stable, to change the * to a + and
    // have this arm be used when there are no arguments :(
    // ($fmt:expr) => {
    //     use std::fmt::Write;

    //     let mut fixed = CFixedString::new();
    //     write!(&mut fixed, $fmt).unwrap();
    //     fixed
    // }

    ($fmt:expr, $($args:tt)*) => ({
        use std::fmt::Write;

        let mut fixed = CFixedString::new();
        write!(&mut fixed, $fmt, $($args)*).unwrap();
        fixed
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fmt::Write;

    fn gen_string(len: usize) -> String {
        let mut out = String::with_capacity(len);

        for _ in 0..len / 16 {
            out.write_str("zyxvutabcdef9876").unwrap();
        }

        for i in 0..len % 16 {
            out.write_char((i as u8 + 'A' as u8) as char).unwrap();
        }

        assert_eq!(out.len(), len);
        out
    }

    #[test]
    fn test_empty_handler() {
        let short_string = "";

        let t = CFixedString::from_str(short_string);

        assert!(!t.is_allocated());
        assert_eq!(&t.to_string(), short_string);
    }

    #[test]
    fn test_short_1() {
        let short_string = "test_local";

        let t = CFixedString::from_str(short_string);

        assert!(!t.is_allocated());
        assert_eq!(&t.to_string(), short_string);
    }

    #[test]
    fn test_short_2() {
        let short_string = "test_local stoheusthsotheost";

        let t = CFixedString::from_str(short_string);

        assert!(!t.is_allocated());
        assert_eq!(&t.to_string(), short_string);
    }

    #[test]
    fn test_511() {
        // this string (width 511) buffer should just fit
        let test_511_string = gen_string(511);

        let t = CFixedString::from_str(&test_511_string);

        assert!(!t.is_allocated());
        assert_eq!(&t.to_string(), &test_511_string);
    }

    #[test]
    fn test_512() {
        // this string (width 512) buffer should not fit
        let test_512_string = gen_string(512);

        let t = CFixedString::from_str(&test_512_string);

        assert!(t.is_allocated());
        assert_eq!(&t.to_string(), &test_512_string);
    }

    #[test]
    fn test_513() {
        // this string (width 513) buffer should not fit
        let test_513_string = gen_string(513);

        let t = CFixedString::from_str(&test_513_string);

        assert!(t.is_allocated());
        assert_eq!(&t.to_string(), &test_513_string);
    }

    #[test]
    fn test_to_owned() {
        let short = "this is an amazing string";

        let t = CFixedString::from_str(short);

        assert!(!t.is_allocated());
        assert_eq!(&String::from(t), short);

        let long = gen_string(1025);

        let t = CFixedString::from_str(&long);

        assert!(t.is_allocated());
        assert_eq!(&String::from(t), &long);
    }

    #[test]
    fn test_short_format() {
        let mut fixed = CFixedString::new();

        write!(&mut fixed, "one_{}", 1).unwrap();
        write!(&mut fixed, "_two_{}", "two").unwrap();
        write!(&mut fixed,
               "_three_{}-{}-{:.3}",
               23,
               "some string data",
               56.789)
            .unwrap();

        assert!(!fixed.is_allocated());
        assert_eq!(&fixed.to_string(),
                   "one_1_two_two_three_23-some string data-56.789");
    }

    #[test]
    fn test_long_format() {
        let mut fixed = CFixedString::new();
        let mut string = String::new();

        for i in 1..30 {
            let genned = gen_string(i * i);

            write!(&mut fixed, "{}_{}", i, genned).unwrap();
            write!(&mut string, "{}_{}", i, genned).unwrap();
        }

        assert!(fixed.is_allocated());
        assert_eq!(&fixed.to_string(), &string);
    }

    // TODO: Reenable this test once the empty match arm is allowed
    // by the compiler
    // #[test]
    // fn test_empty_fmt_macro() {
    // let empty = format_c!("");
    // let no_args = format_c!("there are no format args");
    //
    // assert!(!empty.is_allocated());
    // assert_eq!(&empty.to_string(), "");
    //
    // assert!(!no_args.is_allocated());
    // assert_eq!(&no_args.to_string(), "there are no format args");
    // }

    #[test]
    fn test_short_fmt_macro() {
        let first = 23;
        let second = "#@!*()&^%_-+={}[]|\\/?><,.:;~`";
        let third = u32::max_value();
        let fourth = gen_string(512 - 45);

        let fixed = format_c!("{}_{}_0x{:x}_{}", first, second, third, fourth);
        let heaped = format!("{}_{}_0x{:x}_{}", first, second, third, fourth);

        assert!(!fixed.is_allocated());
        assert_eq!(&fixed.to_string(), &heaped);
    }

    #[test]
    fn test_long_fmt_macro() {
        let first = "";
        let second = gen_string(510);
        let third = 3;
        let fourth = gen_string(513 * 8);

        let fixed = format_c!("{}_{}{}{}", first, second, third, fourth);
        let heaped = format!("{}_{}{}{}", first, second, third, fourth);

        assert!(fixed.is_allocated());
        assert_eq!(&fixed.to_string(), &heaped);
    }
}
