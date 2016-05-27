use prodbg_api::view::CViewCallbacks;
use libc::{c_void, c_uchar};
use std::rc::Rc;
use plugin::Plugin;
use plugins::PluginHandler;
use dynamic_reload::Lib;
use session::SessionHandle;
use std::ptr;
use prodbg_api::ui::Ui;
use plugin_io;

#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct ViewHandle(pub u64);

pub struct ViewInstance {
    pub plugin_data: *mut c_void,
    pub ui: Ui,
    pub name: String,
    pub handle: ViewHandle,
    pub session_handle: SessionHandle,
    pub x: f32,
    pub y: f32,
    pub width: f32,
    pub height: f32,
    pub plugin_type: Rc<Plugin>,
}

#[derive(Clone)]
struct ReloadState {
    name: String,
    ui: Ui,
    handle: ViewHandle,
    session_handle: SessionHandle,
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
                    ui: self.instances[i].ui.clone(),
                    name: self.instances[i].plugin_type.name.clone(),
                    handle: self.instances[i].handle,
                    session_handle: self.instances[i].session_handle,
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
            Self::create_instance(self,
                                  reload_plugin.ui.clone(),
                                  &reload_plugin.name,
                                  reload_plugin.session_handle);
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

    pub fn create_instance_from_index(&mut self,
                                      ui: Ui,
                                      index: usize,
                                      session_handle: SessionHandle,
                                      view_handle: Option<ViewHandle>)
                                      -> Option<ViewHandle> {
        let plugin_data = unsafe {
            let callbacks = self.plugin_types[index].plugin_funcs as *mut CViewCallbacks;
            (*callbacks).create_instance.unwrap()(ui.api as *mut c_void, Self::service_fun)
        };

        let handle = match view_handle {
            Some(h) => {
                if h.0 >= self.handle_counter.0 {
                    self.handle_counter.0 = h.0 + 1
                }
                h
            },
            _ => {
                let counter = self.handle_counter;
                self.handle_counter.0 += 1;
                counter
            }
        };

        let instance = ViewInstance {
            plugin_data: plugin_data,
            name: format!("Plugin {}", handle.0),
            ui: ui,
            handle: handle,
            session_handle: session_handle,
            x: 0.0,
            y: 0.0,
            width: 0.0,
            height: 0.0,
            plugin_type: self.plugin_types[index].clone(),
        };

        self.instances.push(instance);

        Some(handle)
    }

    pub fn create_instance(&mut self,
                           ui: Ui,
                           plugin_type: &String,
                           session_handle: SessionHandle)
                           -> Option<ViewHandle> {
        for i in 0..self.plugin_types.len() {
            if self.plugin_types[i].name != *plugin_type {
                continue;
            }

            return Self::create_instance_from_index(self, ui, i, session_handle, None);
        }

        None
    }

    pub fn create_instance_with_handle(&mut self,
                                       ui: Ui,
                                       plugin_type: &String,
                                       plugin_data: &Option<Vec<String>>,
                                       session_handle: SessionHandle,
                                       view_handle: ViewHandle) -> Option<ViewHandle> {

        for i in 0..self.plugin_types.len() {
            if self.plugin_types[i].name != *plugin_type {
                continue;
            }

            let handle = Self::create_instance_from_index(self, ui, i, session_handle, Some(view_handle));

            if let Some(ref data) = *plugin_data {
                self.get_view(handle.unwrap()).map(|instance| {
                    instance.load_plugin_data(&data);
                });
            }

            return handle;
        }

        None
    }

    pub fn destroy_instance(&mut self, handle: ViewHandle) {
        for i in (0..self.instances.len()).rev() {
            if self.instances[i].handle.0 == handle.0 {
                self.instances.swap_remove(i);
                return;
            }
        }
    }

    // TODO: Would be nice to use something stack-base instead or return an iterator to interate
    // over the data instead
    pub fn get_plugin_names(&self) -> Vec<String> {
        let mut names = Vec::new();

        for i in &self.plugin_types {
            names.push(i.name.clone());
        }

        names
    }
}

impl ViewInstance {
    pub fn get_plugin_data(&self) -> (String, Option<Vec<String>>) {
        let mut plugin_data = None;
        unsafe {
            let callbacks = self.plugin_type.plugin_funcs as *mut CViewCallbacks;
            if let Some(save_state) = (*callbacks).save_state {
                let mut writer_funcs = plugin_io::get_writer_funcs();
                save_state(self.plugin_data, &mut writer_funcs);
                plugin_data = Some(plugin_io::get_data(&mut writer_funcs));
            }
        };
        (self.plugin_type.name.clone(), plugin_data)
    }

    pub fn load_plugin_data(&mut self, data: &Vec<String>) {
        unsafe {
            let callbacks = self.plugin_type.plugin_funcs as *mut CViewCallbacks;
            if let Some(load_state) = (*callbacks).load_state {
                let mut loader_funcs = plugin_io::get_loader_funcs(data);
                load_state(self.plugin_data, &mut loader_funcs);
            }
        }
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn test_search_paths_none() {}
}
