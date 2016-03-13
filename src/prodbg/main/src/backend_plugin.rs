//use libc::{c_void, c_uchar};
/*

use std::rc::Rc;
use core::plugin::Plugin;
use core::plugins::PluginHandler;
use core::view_plugins::ViewInstance;
use core::Lib;
use prodbg_api::backend::CBackendCallbacks;
//use std::ptr;

#[derive(PartialEq, Eq, Clone, Copy, Debug)]
pub struct BackendHandle(pub usize);

pub struct BackendPlugin {
    plugin: Rc<Plugin>,
    reload: bool,
}

impl PluginHandler for BackendPlugin {
    fn is_correct_plugin_type(&self, plugin: &Plugin) -> bool {
        plugin.type_name.contains("Backend")
    }

    fn add_plugin(&mut self, plugin: &Rc<Plugin>) {
        self.plugin = plugin.clone()
    }

    fn unload_plugin(&mut self, _lib: &Rc<Lib>) {
        /*
        if let Some(ref p) = self.plugin {
            if &p.lib == lib {
                self.reload = true;
            }
        }

        if self.reload {
            Self::unload(self)
        }
        */
    }

    fn reload_plugin(&mut self) {
        if self.reload {
            Self::reload(self);
        }

        self.reload = false
    }

    fn reload_failed(&mut self) {
        //Self::reload(self)
    }
}

impl BackendPlugin {
    pub fn new() -> BackendPlugin {
        BackendPlugin {
            plugin: None,
            reload: false,
        }
    }

    fn unload(&mut self) {
        // TODO: Save state here
    }

    fn reload(&mut self) {
        // Restore state here
    }

    /*

    fn create_instance_from_type(t: &Plugin) -> Option<BackendHandle> {
        let user_data = unsafe {
            let callbacks = t.plugin_funcs as *mut CBackendCallbacks;
            (*callbacks).create_instance.unwrap()(Self::service_fun)
        };

        let instance = ViewInstance {
            user_data: user_data,
            handle: self.handle_counter,
            plugin_type: t.clone(),
        };

        self.handle_counter.0 += 1;

        let index = self.instances.len();
        self.instances.push(instance);

        Some(instance.handle)
    }

    pub fn create_instance_from_index(mut self, index: usize) -> Option<BackendHandle> {
        create_instance_from_type(self.plugin_types[i], session, window)
    }

    pub fn create_instance(&mut self, plugin_type: &String) -> Option<BackendHandle> {
        for t in self.plugin_types.iter() {
            if t.name != *plugin_type {
                continue;
            }

            return create_instance_from_type(&t, session, window);
        }

        None
    }
    */

}
*/
