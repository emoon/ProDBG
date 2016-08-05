extern crate minifb;

use minifb::{Key, MENU_KEY_CTRL};
use minifb::Menu as MinifbMenu;

pub const MENU_FILE_OPEN_AND_RUN_EXE: usize = 1;
pub const MENU_FILE_OPEN_SOURCE: usize = 2;
pub const MENU_FILE_START_NEW_BACKEND: usize = 3;
pub const MENU_FILE_BACKEND_START: usize = 4;
pub const MENU_FILE_BACKEND_END: usize = 99;
pub const MENU_DEBUG_ATTACH_TO_REMOTE: usize = 100;
pub const MENU_DEBUG_START: usize = 101;
pub const MENU_DEBUG_STEP_IN: usize = 102;
pub const MENU_DEBUG_STEP_OVER: usize = 103;
pub const MENU_DEBUG_TOGGLE_BREAKPOINT: usize = 104;

pub struct Menu {
    pub file_menu: MinifbMenu,
    pub debug_menu: MinifbMenu,
}

impl Menu {
    pub fn new() -> Menu {
        Menu {
            file_menu: Self::create_file_menu(),
            debug_menu: Self::create_debug_menu(),
        }
    }

    pub fn create_file_menu() -> MinifbMenu {
        let mut menu = MinifbMenu::new("File").unwrap();

        menu.add_item("Open and run executable...", MENU_FILE_OPEN_AND_RUN_EXE)
            .shortcut(Key::D, MENU_KEY_CTRL)
            .build();

        menu.add_item("Open Source...", MENU_FILE_OPEN_SOURCE)
            .shortcut(Key::O, MENU_KEY_CTRL)
            .build();

        menu.add_item("Start new backend...", MENU_FILE_START_NEW_BACKEND)
            .shortcut(Key::N, MENU_KEY_CTRL)
            .build();

        menu
    }

    pub fn create_debug_menu() -> MinifbMenu {
        let mut menu = MinifbMenu::new("Debug").unwrap();

        menu.add_item("Attach to Remote...", MENU_DEBUG_ATTACH_TO_REMOTE)
            .shortcut(Key::F6, 0)
            .build();

        menu.add_item("Start", MENU_DEBUG_START)
            .shortcut(Key::F5, 0)
            .build();

        menu.add_item("Step In", MENU_DEBUG_STEP_IN)
            .shortcut(Key::F11, 0)
            .build();

        menu.add_item("Step Over", MENU_DEBUG_STEP_OVER)
            .shortcut(Key::F10, 0)
            .build();

        menu.add_item("Toggel Breakpoint", MENU_DEBUG_TOGGLE_BREAKPOINT)
            .shortcut(Key::F10, 0)
            .build();

        menu
    }
}

