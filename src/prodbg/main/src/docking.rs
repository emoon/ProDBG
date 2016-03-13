//use libc::{c_void, c_uchar};
use std::rc::Rc;
use core::plugin::Plugin;
use core::plugins::PluginHandler;
use core::Lib;
//use std::ptr;

pub struct DockingPlugin {
    plugin: Option<Rc<Plugin>>,
    reload: bool,
}

impl PluginHandler for DockingPlugin {
    fn is_correct_plugin_type(&self, plugin: &Plugin) -> bool {
        plugin.type_name.contains("Docking")
    }

    fn add_plugin(&mut self, plugin: &Rc<Plugin>) {
        println!("Added docking plugin");
        self.plugin = Some(plugin.clone())
    }

    fn unload_plugin(&mut self, lib: &Rc<Lib>) {
        if let Some(ref p) = self.plugin {
            if &p.lib == lib {
                self.reload = true;
            }
        }

        if self.reload {
            Self::unload(self)
        }
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

impl DockingPlugin {
    pub fn new() -> DockingPlugin {
        DockingPlugin {
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
}
