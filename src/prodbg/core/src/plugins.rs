extern crate libloading;
extern crate dynamic_reload;
extern crate libc;

use self::dynamic_reload::{DynamicReload, Lib, PlatformName, UpdateState};
use self::libloading::Result as LibRes;
use self::libloading::Symbol;
use std::rc::Rc;
use self::libc::{c_char, c_void};
use std::mem::transmute;
use plugin::Plugin;
use std::cell::RefCell;

pub struct Plugins {
    pub plugin_types: Vec<Rc<Lib>>,
    pub plugin_handlers: Vec<Rc<RefCell<PluginHandler>>>,
}

struct CallbackData<'a> {
    handler: &'a mut Plugins,
    lib: &'a Rc<Lib>,
}

pub struct ReloadHandler<'a> {
    pub plugins: &'a mut Plugins,
    pub instance_count: i32,
    pub name: String,
}

pub trait PluginHandler {
    fn is_correct_plugin_type(&self, plugin: &Plugin) -> bool;
    fn add_plugin(&mut self, plugin: &Rc<Plugin>);
    fn unload_plugin(&mut self, lib: &Rc<Lib>);
    fn reload_plugin(&mut self);
    fn reload_failed(&mut self);
}

type RegisterPlugin = unsafe fn(pt: *const c_char,
                                plugin: *mut c_void,
                                data: *mut CallbackData);

unsafe fn register_plugin_callback(plugin_type: *const c_char,
                                   plugin: *mut c_void,
                                   ph: *mut CallbackData) {
    let t = &mut (*ph);

    let p = Plugin::new(t.lib, plugin_type, plugin);

    t.handler.plugin_types.push(t.lib.clone());

    for handler in t.handler.plugin_handlers.iter_mut() {
        if handler.borrow().is_correct_plugin_type(&p) {
            handler.borrow_mut().add_plugin(&Rc::new(Plugin::new(t.lib, plugin_type, plugin)));
        }
    }
}

impl<'a> ReloadHandler<'a> {
    fn new(plugins: &'a mut Plugins) -> ReloadHandler {
        ReloadHandler {
            plugins: plugins,
            instance_count: 0,
            name: "".to_string(),
        }
    }

    fn unload_plugins(&mut self, lib: &Rc<Lib>) {
        for handler in self.plugins.plugin_handlers.iter_mut() {
            handler.borrow_mut().unload_plugin(lib);
        }

        for i in (0..self.plugins.plugin_types.len()).rev() {
            if &self.plugins.plugin_types[i] == lib {
                self.plugins.plugin_types.swap_remove(i);
            }
        }
    }

    fn reload_plugins(&mut self, lib: &Rc<Lib>) {
        unsafe { self.plugins.add_p(lib) }

        for handler in self.plugins.plugin_handlers.iter_mut() {
            handler.borrow_mut().reload_plugin();
        }
    }

    fn reload_failed(&mut self) {
        for handler in self.plugins.plugin_handlers.iter_mut() {
            handler.borrow_mut().reload_failed();
        }
    }

    fn callback(&mut self, state: UpdateState, lib: Option<&Rc<Lib>>) {
        match state {
            UpdateState::Before => Self::unload_plugins(self, lib.unwrap()),
            UpdateState::After => Self::reload_plugins(self, lib.unwrap()),
            UpdateState::ReloadFalied(_) => Self::reload_failed(self),
        }
    }
}

impl Plugins {
    pub fn new() -> Plugins {
        Plugins {
            plugin_types: Vec::new(),
            plugin_handlers: Vec::new(),
        }
    }

    pub fn add_handler<T: PluginHandler + 'static>(&mut self, handler: &Rc<RefCell<T>>) {
        self.plugin_handlers.push(handler.clone());
    }

    pub fn add_plugin(&mut self, lib_handler: &mut DynamicReload, name: &str) {
        match lib_handler.add_library(name, PlatformName::Yes) {
            Ok(lib) => unsafe {
                Self::add_p(self, &lib);
            },
            Err(e) => {
                println!("Unable to add {} err {:?}", name, e);
            }
        }
    }

    pub fn update(&mut self, lib_handler: &mut DynamicReload) {
        let mut handler = ReloadHandler::new(self);
        lib_handler.update(ReloadHandler::callback, &mut handler);
    }

    unsafe fn add_p(&mut self, library: &Rc<Lib>) {
        let init_plugin: LibRes<Symbol<extern "C" fn(RegisterPlugin, *mut CallbackData)>> =
            library.lib.get(b"InitPlugin");

        match init_plugin {
            Ok(init_fun) => {
                let mut callback_data = CallbackData {
                    handler: transmute(self),
                    lib: library,
                };

                init_fun(register_plugin_callback, &mut callback_data);
            }

            Err(e) => println!("Unable to load {:?} err {:?}", library.original_path, e),
        }
    }
}
