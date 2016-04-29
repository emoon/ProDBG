extern crate minifb;

use minifb::{Key, MENU_KEY_CTRL, MENU_KEY_COMMAND};
use minifb::Menu as MinifbMenu;

const MENU_FILE_OPEN_AND_RUN_EXE: usize = 1;
const MENU_FILE_OPEN_SOURCE: usize = 1;
const MENU_DEBUG_ATTACH_TO_REMOTE: usize = 50;
const MENU_DEBUG_START: usize = 51;
const MENU_DEBUG_STEP_IN: usize = 52;
const MENU_DEBUG_STEP_OVER: usize = 53;
const MENU_DEBUG_TOGGLE_BREAKPOINT: usize = 54;

pub struct Menu<'a> {
    pub file_menu: Vec<MinifbMenu<'a>>,
    pub debug_menu: Vec<MinifbMenu<'a>>,
}

impl<'a> Menu<'a> {
    pub fn new() -> Menu<'a> {
        Menu {
            file_menu: Self::create_file_menu(),
            debug_menu: Self::create_debug_menu(),
        }
    }

    pub fn create_file_menu() -> Vec<MinifbMenu<'a>> {
        vec![
            MinifbMenu {
                name: "Open and run executable...",
                key: Key::D,
                id: MENU_FILE_OPEN_AND_RUN_EXE,
                modifier: MENU_KEY_CTRL,
                mac_mod: MENU_KEY_COMMAND,
                ..MinifbMenu::default()
            },
            MinifbMenu {
                name: "Open Source...",
                key: Key::O,
                id: MENU_FILE_OPEN_SOURCE,
                modifier: MENU_KEY_CTRL,
                mac_mod: MENU_KEY_COMMAND,
                ..MinifbMenu::default()
            }
        ]
    }

    pub fn create_debug_menu() -> Vec<MinifbMenu<'a>> {
        vec![
            MinifbMenu {
                name: "Attach to Remote...",
                key: Key::F6,
                id: MENU_DEBUG_ATTACH_TO_REMOTE ,
                ..MinifbMenu::default()
            },
            MinifbMenu  {
                name: "Start",
                key: Key::S,
                id: MENU_DEBUG_START,
                ..MinifbMenu::default()
            },
            MinifbMenu {
                name: "Step In",
                key: Key::F11,
                id: MENU_DEBUG_STEP_IN,
                ..MinifbMenu::default()
            },
            MinifbMenu {
                name: "Step Over",
                key: Key::F10,
                id: MENU_DEBUG_STEP_OVER,
                ..MinifbMenu::default()
            },
            MinifbMenu {
                name: "Toggle Breakpoint",
                key: Key::F9,
                id: MENU_DEBUG_TOGGLE_BREAKPOINT,
                ..MinifbMenu::default()
            }
        ]
    }
}

