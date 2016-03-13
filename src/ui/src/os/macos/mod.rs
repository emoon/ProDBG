#![cfg(target_os = "macos")]

use {Key, KeyRepeat};
use key_handler::KeyHandler;

use libc::{c_void, c_char};
use std::ffi::{CString};
use std::ptr;
use std::mem;

// Table taken from GLFW and slightly modified

static KEY_MAPPINGS: [Key; 128] = [ 
    /* 00 */ Key::A,
    /* 01 */ Key::S,
    /* 02 */ Key::D,
    /* 03 */ Key::F,
    /* 04 */ Key::H,
    /* 05 */ Key::G,
    /* 06 */ Key::Z,
    /* 07 */ Key::X,
    /* 08 */ Key::C,
    /* 09 */ Key::V,
    /* 0a */ Key::Unknown, // GraveAccent
    /* 0b */ Key::B,
    /* 0c */ Key::Q,
    /* 0d */ Key::W,
    /* 0e */ Key::E,
    /* 0f */ Key::R,
    /* 10 */ Key::Y,
    /* 11 */ Key::T,
    /* 12 */ Key::Key1,
    /* 13 */ Key::Key2,
    /* 14 */ Key::Key3,
    /* 15 */ Key::Key4,
    /* 16 */ Key::Key6,
    /* 17 */ Key::Key5,
    /* 18 */ Key::Equal,
    /* 19 */ Key::Key9,
    /* 1a */ Key::Key7,
    /* 1b */ Key::Minus,
    /* 1c */ Key::Key8,
    /* 1d */ Key::Key0,
    /* 1e */ Key::RightBracket,
    /* 1f */ Key::O,
    /* 20 */ Key::U,
    /* 21 */ Key::LeftBracket,
    /* 22 */ Key::I,
    /* 23 */ Key::P,
    /* 24 */ Key::Enter,
    /* 25 */ Key::L,
    /* 26 */ Key::J,
    /* 27 */ Key::Apostrophe,
    /* 28 */ Key::K,
    /* 29 */ Key::Semicolon,
    /* 2a */ Key::Backslash,
    /* 2b */ Key::Comma,
    /* 2c */ Key::Slash,
    /* 2d */ Key::N,
    /* 2e */ Key::M,
    /* 2f */ Key::Period,
    /* 30 */ Key::Tab,
    /* 31 */ Key::Space,
    /* 32 */ Key::Unknown,  // World1
    /* 33 */ Key::Backspace,
    /* 34 */ Key::Unknown,
    /* 35 */ Key::Escape,
    /* 36 */ Key::RightSuper,
    /* 37 */ Key::LeftSuper,
    /* 38 */ Key::LeftShift,
    /* 39 */ Key::CapsLock,
    /* 3a */ Key::LeftAlt,
    /* 3b */ Key::LeftCtrl,
    /* 3c */ Key::RightShift,
    /* 3d */ Key::RightAlt,
    /* 3e */ Key::RightCtrl,
    /* 3f */ Key::Unknown, // Function
    /* 40 */ Key::Unknown, // F17
    /* 41 */ Key::Unknown, // Decimal
    /* 42 */ Key::Unknown, 
    /* 43 */ Key::Unknown, // Multiply
    /* 44 */ Key::Unknown,
    /* 45 */ Key::Unknown, // Add
    /* 46 */ Key::Unknown,
    /* 47 */ Key::NumLock, // Really KeypadClear...
    /* 48 */ Key::Unknown, // VolumeUp
    /* 49 */ Key::Unknown, // VolumeDown
    /* 4a */ Key::Unknown, // Mute
    /* 4b */ Key::Unknown, 
    /* 4c */ Key::Enter,
    /* 4d */ Key::Unknown,
    /* 4e */ Key::Unknown, // Subtrackt
    /* 4f */ Key::Unknown, // F18
    /* 50 */ Key::Unknown, // F19
    /* 51 */ Key::Equal,
    /* 52 */ Key::NumPad0,
    /* 53 */ Key::NumPad1,
    /* 54 */ Key::NumPad2,
    /* 55 */ Key::NumPad3,
    /* 56 */ Key::NumPad4,
    /* 57 */ Key::NumPad5,
    /* 58 */ Key::NumPad6,
    /* 59 */ Key::NumPad7,
    /* 5a */ Key::Unknown, // F20
    /* 5b */ Key::NumPad8,
    /* 5c */ Key::NumPad9,
    /* 5d */ Key::Unknown,
    /* 5e */ Key::Unknown,
    /* 5f */ Key::Unknown,
    /* 60 */ Key::F5,
    /* 61 */ Key::F6,
    /* 62 */ Key::F7,
    /* 63 */ Key::F3,
    /* 64 */ Key::F8,
    /* 65 */ Key::F9,
    /* 66 */ Key::Unknown,
    /* 67 */ Key::F11,
    /* 68 */ Key::Unknown,
    /* 69 */ Key::Unknown, // PrintScreen
    /* 6a */ Key::Unknown, // F16
    /* 6b */ Key::F14,
    /* 6c */ Key::Unknown,
    /* 6d */ Key::F10,
    /* 6e */ Key::Unknown,
    /* 6f */ Key::F12,
    /* 70 */ Key::Unknown,
    /* 71 */ Key::F15,
    /* 72 */ Key::Insert, /* Really Help... */
    /* 73 */ Key::Home,
    /* 74 */ Key::PageUp,
    /* 75 */ Key::Delete,
    /* 76 */ Key::F4,
    /* 77 */ Key::End,
    /* 78 */ Key::F2,
    /* 79 */ Key::PageDown,
    /* 7a */ Key::F1,
    /* 7b */ Key::Left,
    /* 7c */ Key::Right,
    /* 7d */ Key::Down,
    /* 7e */ Key::Up,
    /* 7f */ Key::Unknown,
];

#[link(name = "Cocoa", kind = "framework")]
extern {
    fn mfb_open(name: *const c_char, width: u32, height: u32) -> *mut c_void;
    fn mfb_close(window: *mut c_void);
    fn mfb_update(window: *mut c_void);
    fn mfb_set_position(window: *mut c_void, x: i32, y: i32);
    fn mfb_set_data_key_callback(window: *mut c_void, target: *mut c_void, cb: unsafe extern fn(*mut c_void, i32, i32));
    fn mfb_should_close(window: *mut c_void) -> i32;
}

// This needs to match C version in OSXWIndow
#[derive(Default)]
pub struct MouseData {
    pub pos_x: f32,
    pub pos_y: f32,
    pub scroll_x: f32,
    pub scroll_y: f32,
    pub state: [u8; 5],
}

// This needs to match C version in OSXWIndow
#[repr(C)]
struct SharedData {
    window_handle: *mut c_void,
    mouse_data: MouseData,
}

// This needs to match C version in OSXWIndow
pub struct Window {
    shared_data: SharedData, 
    key_handler: KeyHandler,
}

unsafe extern "C" fn key_callback(window: *mut c_void, key: i32, state: i32) {
    let win: *mut Window = mem::transmute(window);

    let s = state == 1;

    if key > 128 {
        (*win).key_handler.set_key_state(Key::Unknown, s);
    } else {
        (*win).key_handler.set_key_state(KEY_MAPPINGS[key as usize], s);
    }
}

impl Window {
    pub fn new(name: &str, width: usize, height: usize) -> Result<Window, &str> {
        let n = match CString::new(name) {
            Err(_) => { 
                println!("Unable to convert {} to c_string", name);
                return Err("Unable to set correct name"); 
            }
            Ok(n) => n,
        };

        unsafe {
            let handle = mfb_open(n.as_ptr(), width as u32, height as u32);

            if handle == ptr::null_mut() {
                return Err("Unable to open Window");
            }

            Ok(Window { 
                shared_data: SharedData { 
                    window_handle: handle,
                    mouse_data: MouseData::default(),
                },
                key_handler: KeyHandler::new(),
            })
        }
    }

    pub fn init(&mut self) {
        unsafe {
            mfb_set_data_key_callback(self.shared_data.window_handle, mem::transmute(&mut self.shared_data), key_callback);
        }
    }

    pub fn update(&mut self) {
        self.key_handler.update();

        unsafe {
            mfb_update(self.shared_data.window_handle);
        }
    }

    #[inline]
    pub fn get_mouse_pos(&self) -> (f32, f32) {
        (self.shared_data.mouse_data.pos_x, self.shared_data.mouse_data.pos_y)
    }

    #[inline]
    pub fn get_mouse_state(&self, index: usize) -> u8 {
        self.shared_data.mouse_data.state[index]
    }

    #[inline]
    pub fn set_position(&mut self, x: isize, y: isize) {
        unsafe { mfb_set_position(self.shared_data.window_handle, x as i32, y as i32) }
    }

    #[inline]
    pub fn get_keys(&self) -> Option<Vec<Key>> {
        self.key_handler.get_keys()
    }

    #[inline]
    pub fn get_keys_pressed(&self, repeat: KeyRepeat) -> Option<Vec<Key>> {
        self.key_handler.get_keys_pressed(repeat)
    }

    #[inline]
    pub fn is_key_down(&self, key: Key) -> bool {
        self.key_handler.is_key_down(key)
    }

    #[inline]
    pub fn set_key_repeat_delay(&mut self, delay: f32) {
        self.key_handler.set_key_repeat_delay(delay)
    }

    #[inline]
    pub fn set_key_repeat_rate(&mut self, rate: f32) {
        self.key_handler.set_key_repeat_rate(rate)
    }

    #[inline]
    pub fn is_key_pressed(&self, key: Key, repeat: KeyRepeat) -> bool {
        self.key_handler.is_key_pressed(key, repeat)
    }

    #[inline]
    pub fn is_open(&self) -> bool {
        unsafe { mfb_should_close(self.shared_data.window_handle) == 0 }
    }

    #[inline]
    pub fn get_native_handle(&self) -> *mut c_void {
        self.shared_data.window_handle
    }
}

impl Drop for Window {
    fn drop(&mut self) {
        unsafe {
            mfb_close(self.shared_data.window_handle);
        }
    }
}

