use prodbg_api::view::CViewCallbacks;
use libc::{c_void, c_uchar};
use std::rc::Rc;
use plugin::Plugin;
use plugins::PluginHandler;
use dynamic_reload::Lib;
use std::ptr;

#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct ViewHandle(pub usize);

pub struct ViewInstance {
    pub user_data: *mut c_void,
    pub handle: ViewHandle,
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
    pub plugin_type: Rc<Plugin>,
}

#[derive(Clone)]
struct ReloadState {
    name: String,
    handle: ViewHandle,
}

pub struct ViewPlugins {
    pub instances: Vec<ViewInstance>,
    plugin_types: Vec<Rc<Plugin>>,
    reload_state: Vec<ReloadState>,
    handle_counter: ViewHandle,
}

impl PluginHandler for ViewPlugins {
    fn is_correct_plugin_type(&self, plugin: &Plugin) -> bool {
        plugin.type_name.contains("View")
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



impl ViewPlugins {
    pub fn new() -> ViewPlugins {
        ViewPlugins {
            instances: Vec::new(),
            plugin_types: Vec::new(),
            reload_state: Vec::new(),
            handle_counter: ViewHandle(0),
        }
    }

    pub extern "C" fn service_fun(_name: *const c_uchar) -> *mut c_void {
        ptr::null_mut()
    }

    pub fn get_view(&mut self, view_handle: ViewHandle) -> Option<&mut ViewInstance> {
        for i in 0..self.instances.len() {
            if self.instances[i].handle == view_handle {
                return Some(&mut self.instances[i]);
            }
        }

        None
    }

    pub fn create_instance_from_index(&mut self, index: usize) -> Option<ViewHandle> {
        let user_data = unsafe {
            let callbacks = self.plugin_types[index].plugin_funcs as *mut CViewCallbacks;
            (*callbacks).create_instance.unwrap()(ptr::null(), Self::service_fun)
        };

        let handle = self.handle_counter;

        let instance = ViewInstance {
            user_data: user_data,
            handle: handle,
            x: 0.0,
            y: 0.0,
            width: 0.0,
            height: 0.0,
            plugin_type: self.plugin_types[index].clone(),
        };

        self.handle_counter.0 += 1;
        self.instances.push(instance);

        Some(handle)
    }

    pub fn create_instance(&mut self, plugin_type: &String) -> Option<ViewHandle> {
        for i in 0..self.plugin_types.len() {
            if self.plugin_types[i].name != *plugin_type {
                continue;
            }

            return Self::create_instance_from_index(self, i);
        }

        None
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn test_search_paths_none() {
        assert_eq!(1);
    }
}


