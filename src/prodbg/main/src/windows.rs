extern crate minifb;

use libc::{c_void, c_int};
use minifb::{Scale, WindowOptions, MouseMode, MouseButton};
use core::view_plugins::ViewHandle;

const WIDTH: usize = 1280;
const HEIGHT: usize = 800;

pub struct Window {
    /// minifb window
    pub win: minifb::Window,

    /// Views in this window
    pub views: Vec<ViewHandle>,
}

// struct MenuInfo<'a> {
// name: &'a str,
// menu: Vec<Menu>,
// }
//

///! Windows keeps track of all different windows that are present with in the application
///! There are several ways windows can be created:
///!
///! 1. User opens a new window using a shortcut or menu selection.
///! 2. User "undocks" a view from an existing window giving it it's own floating window.
///! 3. etc

pub struct Windows {
    /// All the windows being tracked
    windows: Vec<Window>,
    current: usize,
}

impl Windows {
    pub fn new() -> Windows {
        Windows { 
            windows: Vec::new(),
            current: 0,
        }
    }

    /// Create a default window which will only be created if there are no other
    pub fn create_default(&mut self) {
        if self.windows.len() > 0 {
            return;
        }

        let window = Self::create_window(WIDTH, HEIGHT).expect("Unable to create window");

        self.windows.push(window)
    }

    pub fn create_window(width: usize, height: usize) -> minifb::Result<Window> {
        let res = minifb::Window::new("ProDBG",
                                      width,
                                      height,
                                      WindowOptions {
                                          resize: true,
                                          scale: Scale::X1,
                                          ..WindowOptions::default()
                                      });
        match res {
            Ok(win) => {
                unsafe {
                    println!("{:?} {} {}", win.get_window_handle() as *const c_void, 
                             width, height);
                    bgfx_create_window(win.get_window_handle() as *const c_void,
                                       width as c_int,
                                       height as c_int);
                }
                Ok(Window {
                    win: win,
                    views: Vec::new(),
                })
            }
            Err(err) => Err(err),
        }
    }

    pub fn create_window_with_menus(&mut self) -> minifb::Result<Window> {
        const WIDTH: usize = 1280;
        const HEIGHT: usize = 1024;

        let window = try!(Self::create_window(WIDTH, HEIGHT));

        // for menu in &menus {
        // window.add_menu(m.name, m.menu);
        // }
        //

        Ok(window)
    }

    fn update_window(window: &mut Window) {
        window.update();

        window.win.get_mouse_pos(MouseMode::Clamp).map(|mouse| {
            unsafe {
                prodbg_set_mouse_pos(mouse.0, mouse.1);
                prodbg_set_mouse_state(0, window.win.get_mouse_down(MouseButton::Left) as c_int);
            }
        });
    }

    pub fn update(&mut self) {
        for i in (0..self.windows.len()).rev() {
            Self::update_window(&mut self.windows[i]);
        
            if !self.windows[i].win.is_open() {
                self.windows.swap_remove(i);
            }
        }  
    }

    pub fn get_current(&mut self) -> &mut Window {
        let current = self.current;
        &mut self.windows[current]
    }
       
    /// Checks if application should exit (all window instances closed)
    pub fn should_exit(&self) -> bool {
        self.windows.len() == 0
    }

    /// Save the state of the windows (usually done when exiting the application)
    pub fn save(_filename: &str) {}

    /// Load the state of all the views from a previous run
    pub fn load(_filename: &str) {}
}

impl Window {
    // fn update(&mut self, docking_plugins: &DockingPlugin) {
    // let dock = docking_plugins.get_handle(self.docking);
    //
    // let plugin_funcs = view.plugin_type.plugin_funcs as *mut CViewCallbacks;
    // ((*plugin_funcs).update.unwrap())(instance.user_data,
    // bgfx_get_ui_funcs(),
    // Send in reader/writer
    // ptr::null_mut(),
    // ptr::null_mut());
    // }
    //
    pub fn update(&mut self) {
        self.win.update();
    }

    pub fn add_view(&mut self, view: ViewHandle) {
        self.views.push(view);
    }
}


extern "C" {
     fn prodbg_set_mouse_pos(x: f32, y: f32);
     fn prodbg_set_mouse_state(mouse: c_int, state: c_int);
    fn bgfx_create_window(window: *const c_void, width: c_int, height: c_int);
}
