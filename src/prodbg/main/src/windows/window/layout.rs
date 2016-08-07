extern crate serde;
extern crate serde_json;


use super::super::viewdock::Workspace;
use core::view_plugins::{ViewHandle, ViewInstance, ViewPlugins};
use core::session::SessionHandle;
use imgui_sys::Imgui;


/// Holds information about `ViewPluginInstance` that is needed to restore it.
pub struct PluginInstanceInfo {
    pub handle: u64,
    pub name: String,
    pub plugin_name: String,
    pub plugin_data: Option<Vec<String>>,
}

impl PluginInstanceInfo {
    pub fn new(instance: &ViewInstance) -> PluginInstanceInfo {
        PluginInstanceInfo {
            handle: instance.handle.0,
            name: instance.name.clone(),
            plugin_name: instance.plugin_type.name.clone(),
            plugin_data: instance.get_plugin_data().1,
        }
    }

    pub fn restore(&self, view_plugins: &mut ViewPlugins) -> Option<ViewHandle> {
        let ui = Imgui::create_ui_instance();
        view_plugins.create_instance(ui,
                                     &self.plugin_name,
                                     self.plugin_data.as_ref(),
                                     Some(&self.name),
                                     SessionHandle(0),
                                     Some(ViewHandle(self.handle)))
    }
}

/// Holds information needed to save and load window layout. After restoring layout call
/// `restore_view_plugins` to instantiate stored view plugins.
pub struct WindowLayout {
    pub workspace: Workspace,
    pub infos: Vec<PluginInstanceInfo>,
}

impl WindowLayout {
    pub fn to_string(&self) -> String {
        serde_json::to_string(self).unwrap()
    }

    pub fn from_string(s: &str) -> Result<WindowLayout, serde_json::Error> {
        serde_json::from_str(s)
    }

    pub fn from_current_state(workspace: Workspace,
                              view_plugins: &mut ViewPlugins)
                              -> WindowLayout {
        let infos = workspace.get_docks()
            .iter()
            .map(|dock| {
                PluginInstanceInfo::new(view_plugins.get_view(ViewHandle(dock.0))
                    .expect("View exists in viewdock but not in view_plugins"))
            })
            .collect();
        WindowLayout {
            workspace: workspace,
            infos: infos,
        }
    }

    pub fn restore_view_plugins(view_plugins: &mut ViewPlugins, infos: &Vec<PluginInstanceInfo>) {
        for info in infos {
            if info.restore(view_plugins).is_none() {
                panic!("Could not restore view");
            }
        }
    }
}

gen_struct_code!(WindowLayout, workspace, infos;);
gen_struct_code!(PluginInstanceInfo, handle, name, plugin_name, plugin_data;);
