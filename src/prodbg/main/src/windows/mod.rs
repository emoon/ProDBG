extern crate minifb;
extern crate bgfx;
extern crate viewdock;
extern crate renderer;

mod window;
mod keys;

use renderer::Renderer;
use core::view_plugins::ViewPlugins;
use core::backend_plugin::{BackendHandle, BackendPlugins};
use core::session::Sessions;
use settings::Settings;
use self::window::Window;
use self::keys::KeyCharCallback;
use menu::{MENU_FILE_BACKEND_START, MENU_FILE_BACKEND_END};
use project::Project;

const WIDTH: i32 = 1280;
const HEIGHT: i32 = 800;

/// ! Windows keeps track of all different windows that are present with in the application
/// ! There are several ways windows can be created:
/// !
/// ! 1. User opens a new window using a shortcut or menu selection.
/// ! 2. User "undocks" a view from an existing window giving it it's own floating window.
/// ! 3. etc

pub struct Windows {
    /// All the windows being tracked
    windows: Vec<Window>,
    current: usize,
    renderer: Renderer,
}

impl Windows {
    pub fn new() -> Windows {
        Windows {
            windows: Vec::new(),
            renderer: Renderer::new(),
            current: 0,
        }
    }

    /// Create a default window which will only be created if there are no other
    pub fn create_default(&mut self, settings: &Settings, backend_plugins: Vec<String>) {
        if self.windows.len() > 0 {
            return;
        }

        let window = self.create_window_with_menus(settings, backend_plugins)
            .expect("Unable to create window");

        keys::setup_imgui_key_mappings();

        self.windows.push(window)
    }

    pub fn create_window(&mut self, width: usize, height: usize) -> minifb::Result<Window> {
        let win = try!(Window::new(width, height));
        // TODO: Return correctly
        self.renderer
            .setup_window(win.win.get_window_handle(), width as u16, height as u16)
            .unwrap();
        Ok(win)
    }

    pub fn create_window_with_menus(&mut self,
                                    settings: &Settings,
                                    backend_plugins: Vec<String>)
                                    -> minifb::Result<Window> {
        let width = settings.get_int("window_size", "width").unwrap_or(WIDTH) as usize;
        let height = settings.get_int("window_size", "height").unwrap_or(HEIGHT) as usize;

        let mut window = try!(self.create_window(width, height));

        window.win.set_input_callback(Box::new(KeyCharCallback {}));

        Self::add_backend_menus(&mut window, backend_plugins);

        // TODO: Figure check result of add_menu
        window.win.add_menu(&window.menu.file_menu);
        window.win.add_menu(&window.menu.debug_menu);

        Ok(window)
    }

    pub fn update(&mut self,
                  sessions: &mut Sessions,
                  view_plugins: &mut ViewPlugins,
                  backend_plugins: &mut BackendPlugins) {
        for win in &mut self.windows {
            win.pre_update();
        }

        self.renderer.pre_update();

        for i in (0..self.windows.len()).rev() {
            self.windows[i].update(sessions, view_plugins, backend_plugins);
            self.renderer.update_size(self.windows[i].win.get_size());

            if !self.windows[i].win.is_open() {
                // TODO: Support more than one window
                self.save_project("data/current_project.json",
                                  sessions,
                                  backend_plugins,
                                  view_plugins);
                self.windows.swap_remove(i);
            }
        }

        self.renderer.post_update();
    }

    pub fn get_current(&mut self) -> &mut Window {
        let current = self.current;
        &mut self.windows[current]
    }

    fn add_backend_menus(window: &mut Window, backend_plugins: Vec<String>) {
        // Build menu with all the backend plugins
        let mut backend_menus = minifb::Menu::new("").unwrap();

        for (i, name) in backend_plugins.iter().enumerate() {
            let id = MENU_FILE_BACKEND_START + i;

            if id >= MENU_FILE_BACKEND_END {
                panic!("Maximum number of backends reached {}",
                       MENU_FILE_BACKEND_END - MENU_FILE_BACKEND_START);
            }

            backend_menus.add_item(&name, id).build();
        }

        window.menu.file_menu.add_sub_menu("New Project...", &backend_menus);
    }

    /// Checks if application should exit (all window instances closed)
    pub fn should_exit(&self) -> bool {
        self.windows.is_empty()
    }

    /// Save the state of the windows (usually done when exiting the application)
    pub fn save_project(&mut self,
                        filename: &str,
                        sessions: &mut Sessions,
                        backend_plugins: &mut BackendPlugins,
                        view_plugins: &mut ViewPlugins) {
        // TODO: This only supports one window for now
        if self.windows.len() == 1 {
            println!("save layout");
            let layout = self.windows[0].layout_to_string(view_plugins);
            let mut backend_name = "".to_owned();
            let mut backend_data = None;

            if let Some(backend) = backend_plugins.get_backend(sessions.get_current().backend) {
                let t = backend.get_plugin_data();
                backend_name = t.0;
                backend_data = t.1;
            }

            let p = Project {
                backend_name: backend_name,
                backend_data: backend_data,
                layout_data: layout,
            };

            // TODO: Proper error handling
            p.save(filename).unwrap();
        }
    }

    /// Load the state of all the views from a previous run
    pub fn init(&mut self,
                layout_data: &str,
                view_plugins: &mut ViewPlugins,
                backend: Option<BackendHandle>,
                backend_plugins: &mut BackendPlugins) {
        // TODO: This only supports one window for now
        if self.windows.len() == 1 {
            // TODO: Proper error handling here (loading is ok to fail though)
            let _ = self.windows[0].init_layout(layout_data, view_plugins);

            if let Some(backend) = backend {
                if let Some(menu) =
                       backend_plugins.get_menu(backend, self.windows[0].menu_id_offset) {
                    self.windows[0].win.add_menu(&(*menu));
                    self.windows[0].menu_id_offset += 1000;
                }
            }
        }
    }
}
