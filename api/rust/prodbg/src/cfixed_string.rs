use std::borrow::{Borrow, Cow};
use std::ffi::{CStr, CString};
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
    Local([c_char; STRING_SIZE]),
    Heap(CString),
}

impl CFixedString {
    pub fn from_str(s: &str) -> Self {
        Self::from(s)
    }

    pub fn as_ptr(&self) -> *const c_char {
        match *self {
            CFixedString::Local(ref s) => s.as_ptr(),
            CFixedString::Heap(ref s) => s.as_ptr(),
        }
    }

    /// Returns true if the string has been heap allocated
    pub fn is_allocated(&self) -> bool {
        match *self {
            CFixedString::Local(_) => false,
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
}

impl<'a> From<&'a str> for CFixedString {
    fn from(s: &'a str) -> Self {
        match s.len() {
            len if len <= STRING_SIZE - 1 => unsafe {
                let mut ret = CFixedString::Local(mem::uninitialized());

                match ret {
                    CFixedString::Local(ref mut l) => {
                        let ptr = l.as_mut_ptr() as *mut u8;
                        ptr::copy(s.as_ptr(), ptr, len);
                        *ptr.offset(len as isize) = 0;
                    },
                    _ => unreachable!(),
                }

                ret
            },
            _ => CFixedString::Heap(CString::new(s).unwrap()),
        }
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
        match *self {
            CFixedString::Local(ref s) => unsafe { CStr::from_ptr(s.as_ptr()) },
            CFixedString::Heap(ref s) => s,
        }
    }
}

impl Borrow<CStr> for CFixedString {
    fn borrow(&self) -> &CStr { self }
}

impl AsRef<CStr> for CFixedString {
    fn as_ref(&self) -> &CStr { self }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn gen_string(len: usize) -> String {
        use std::fmt::Write;

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
}