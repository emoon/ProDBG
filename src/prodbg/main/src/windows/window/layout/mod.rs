extern crate serde;
extern crate serde_json;

#[macro_use]
mod serialize_helper;

use super::super::viewdock::Workspace;
use core::view_plugins::{ViewInstance, ViewPlugins, ViewHandle};
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

    pub fn from_current_state(workspace: Workspace, view_plugins: &mut ViewPlugins) -> WindowLayout {
        let infos = workspace.get_docks()
            .iter()
            .map(|dock| PluginInstanceInfo::new(
                view_plugins
                    .get_view(ViewHandle(dock.0))
                    .expect("View exists in viewdock but not in view_plugins")
            ))
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

mod window_layout_serialization {
    use super::{serde, WindowLayout};
    // Serialization

    impl serde::ser::Serialize for WindowLayout {
        fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
            serializer.serialize_struct("WindowLayout", WindowLayoutMapVisitor { value: self }).map(|_| ())
        }
    }

    struct WindowLayoutMapVisitor<'a> {
        value: &'a WindowLayout
    }

    impl<'a> serde::ser::MapVisitor for WindowLayoutMapVisitor<'a> {
        fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error> where S: serde::Serializer {
            try!(serializer.serialize_struct_elt("workspace", &self.value.workspace));
            try!(serializer.serialize_struct_elt("infos", &self.value.infos));
            Ok(None)
        }
    }

    // Deserialization

    gen_struct_deserializer!(WindowLayout;
        workspace => "workspace", Workspace,
        infos => "infos", Infos;
    );
}

mod plugin_instance_info_serialization {
    use super::{serde, PluginInstanceInfo};
    // Serialization

    impl serde::ser::Serialize for PluginInstanceInfo {
        fn serialize<S>(&self, serializer: &mut S) -> Result<(), S::Error> where S: serde::ser::Serializer {
            serializer.serialize_struct("PluginInstanceInfo", PluginInstanceInfoMapVisitor { value: self }).map(|_| ())
        }
    }

    struct PluginInstanceInfoMapVisitor<'a> {
        value: &'a PluginInstanceInfo
    }

    impl<'a> serde::ser::MapVisitor for PluginInstanceInfoMapVisitor<'a> {
        fn visit<S>(&mut self, serializer: &mut S) -> Result<Option<()>, S::Error> where S: serde::Serializer {
            try!(serializer.serialize_struct_elt("handle", &self.value.handle));
            try!(serializer.serialize_struct_elt("name", &self.value.name));
            try!(serializer.serialize_struct_elt("plugin_name", &self.value.plugin_name));
            try!(serializer.serialize_struct_elt("plugin_data", &self.value.plugin_data));
            Ok(None)
        }
    }

    // Deserialization

    gen_struct_deserializer!(PluginInstanceInfo;
        handle => "handle", Handle,
        name => "name", Name,
        plugin_name => "plugin_name", PluginName,
        plugin_data => "plugin_data", PluginData;
    );
}