use std::os::raw::{c_void};
use std::rc::Rc;
use plugin::Plugin;
use plugins::PluginHandler;
use prodbg_api::backend::CBackendCallbacks;
use menus;
use services;
use Lib;
use minifb::Menu;
use std::mem::transmute;
use std::ptr;

#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct BackendHandle(pub usize);

pub struct BackendInstance {
    pub plugin_data: *mut c_void,
    pub handle: BackendHandle,
    pub plugin_type: Rc<Plugin>,
    pub menu_id_offset: u32,
}

#[derive(Clone)]
struct ReloadState {
    name: String,
    handle: BackendHandle,
}

pub struct BackendPlugins {
    pub instances: Vec<BackendInstance>,
    plugin_types: Vec<Rc<Plugin>>,
    reload_state: Vec<ReloadState>,
    handle_counter: BackendHandle,
}

impl PluginHandler for BackendPlugins {
    fn is_correct_plugin_type(&self, plugin: &Plugin) -> bool {
        plugin.type_name.contains("Backend")
    }

    fn add_plugin(&mut self, plugin: &Rc<Plugin>) {
        println!("added plugin type {}", plugin.type_name);
        self.plugin_types.push(plugin.clone())
    }

    fn unload_plugin(&mut self, lib: &Rc<Lib>) {
        self.reload_state.clear();
        for i in (0..self.instances.len()).rev() {
            if &self.instances[i].plugin_type.lib == lib {
                let state = ReloadState {
                    name: self.instances[i].plugin_type.name.clone(),
                    handle: self.instances[i].handle,
                };

                self.reload_state.push(state);
                self.instances.swap_remove(i);
            }
        }

        for i in (0..self.plugin_types.len()).rev() {
            if &self.plugin_types[i].lib == lib {
                self.plugin_types.swap_remove(i);
            }
        }
    }

    fn reload_plugin(&mut self) {
        let t = self.reload_state.clone();
        for reload_plugin in &t {
            Self::create_instance(self, &reload_plugin.name);
        }
    }

    fn reload_failed(&mut self) {}
}

impl BackendPlugins {
    pub fn new() -> BackendPlugins {
        BackendPlugins {
            instances: Vec::new(),
            plugin_types: Vec::new(),
            reload_state: Vec::new(),
            handle_counter: BackendHandle(0),
        }
    }

    fn create_instance_from_type(&mut self, index: usize) -> Option<BackendHandle> {
        let user_data = unsafe {
            let callbacks = self.plugin_types[index].plugin_funcs as *mut CBackendCallbacks;
            (*callbacks).create_instance.unwrap()(services::get_services)
        };

        let handle = self.handle_counter;

        let instance = BackendInstance {
            plugin_data: user_data,
            handle: handle,
            plugin_type: self.plugin_types[index].clone(),
            menu_id_offset: 0,
        };

        self.handle_counter.0 += 1;

        self.instances.push(instance);

        Some(handle)
    }

    pub fn create_instance_from_index(mut self, index: usize) -> Option<BackendHandle> {
        Self::create_instance_from_type(&mut self, index)
    }

    pub fn create_instance(&mut self, plugin_type: &String) -> Option<BackendHandle> {
        for i in 0..self.plugin_types.len() {
            if self.plugin_types[i].name != *plugin_type {
                continue;
            }

            return self.create_instance_from_type(i);
        }

        None
    }

    pub fn get_backend(&mut self, backend_handle: Option<BackendHandle>) -> Option<&mut BackendInstance> {
        if let Some(handle) = backend_handle {
            for i in 0..self.instances.len() {
                if self.instances[i].handle == handle {
                    return Some(&mut self.instances[i]);
                }
            }
        }

        None
    }

    pub fn get_menu(&mut self, handle: BackendHandle, menu_id_offset: u32) -> Option<Box<Menu>> {
        if let Some(backend) = self.get_backend(Some(handle)) {
            unsafe {
                let plugin_funcs = backend.plugin_type.plugin_funcs as *mut CBackendCallbacks;

                if let Some(register_menu) = (*plugin_funcs).register_menu { 
                    let mut menus_funcs = menus::get_menu_funcs(menu_id_offset); 
                    let funcs: *mut c_void = transmute(&mut menus_funcs);
                    let menu = register_menu(backend.plugin_data, funcs);
                    if menu == ptr::null_mut() {
                        return None;
                    }

                    backend.menu_id_offset = menu_id_offset;

                    let menu: Box<Menu> = transmute(menu);
                    return Some(menu)
                }
            }
        }

        None
    }
}
