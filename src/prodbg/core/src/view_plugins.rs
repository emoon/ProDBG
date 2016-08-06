use prodbg_api::view::CViewCallbacks;
use std::rc::Rc;
use plugin::Plugin;
use plugins::PluginHandler;
use dynamic_reload::Lib;
use session::SessionHandle;
use std::os::raw::c_void;
use prodbg_api::ui::Ui;
use services;
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
    plugin_type: String,
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
                    plugin_type: self.instances[i].plugin_type.name.clone(),
                    name: self.instances[i].name.clone(),
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
            self.create_instance(reload_plugin.ui.clone(),
                                 &reload_plugin.plugin_type,
                                 None, // TODO: Include saved data here
                                 Some(&reload_plugin.name),
                                 reload_plugin.session_handle,
                                 Some(reload_plugin.handle));
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

    pub fn get_view(&mut self, handle: ViewHandle) -> Option<&mut ViewInstance> {
        self.instances.iter_mut().find(|i| i.handle == handle)
    }

    fn name_is_unique(&self, name: &str) -> bool {
        !self.instances.iter().any(|i| i.name == name)
    }

    fn get_unique_name(&self, plugin_type_name: &str) -> String {
        let mut res = plugin_type_name.to_owned();
        let mut counter = 2;
        while !self.name_is_unique(&res) {
            res = format!("{} {}", plugin_type_name, counter);
            counter += 1;
        }
        res
    }

    pub fn create_instance_from_index(&mut self,
                                      ui: Ui,
                                      index: usize,
                                      session_handle: SessionHandle,
                                      view_handle: Option<ViewHandle>,
                                      name: Option<&str>)
                                      -> Option<ViewHandle> {
        let plugin_data = unsafe {
            let callbacks = self.plugin_types[index].plugin_funcs as *mut CViewCallbacks;
            (*callbacks).create_instance.unwrap()(ui.api as *mut c_void, services::get_services)
        };

        let handle = match view_handle {
            Some(h) => {
                if h.0 >= self.handle_counter.0 {
                    self.handle_counter.0 = h.0 + 1
                }
                h
            }
            _ => {
                let counter = self.handle_counter;
                self.handle_counter.0 += 1;
                counter
            }
        };

        let name = name.map(|n| n.to_owned())
            .unwrap_or_else(|| self.get_unique_name(&self.plugin_types[index].name));

        let instance = ViewInstance {
            plugin_data: plugin_data,
            name: name,
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

    /// Creates new plugin instance. If `name` is not specified, unique name will be generated.
    pub fn create_instance(&mut self,
                           ui: Ui,
                           plugin_type: &str,
                           plugin_data: Option<&Vec<String>>,
                           name: Option<&str>,
                           session_handle: SessionHandle,
                           handle: Option<ViewHandle>)
                           -> Option<ViewHandle> {
        self.plugin_types
            .iter()
            .position(|pt| plugin_type == pt.name)
            .and_then(|pos| {
                let res = self.create_instance_from_index(ui, pos, session_handle, handle, name);

                if let Some(handle) = res {
                    if let Some(data) = plugin_data {
                        self.get_view(handle).map(|instance| {
                            instance.load_plugin_data(data);
                        });
                    }
                }

                res
            })
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
