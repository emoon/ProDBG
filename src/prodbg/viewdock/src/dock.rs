use rect::Rect;

/// Handle to a dock
#[derive(Debug, PartialEq, Clone, Copy)]
pub struct DockHandle(pub u64);

/// Holds information about the plugin view, data and handle
#[derive(Debug, Clone)]
pub struct Dock {
    pub handle: DockHandle,
    pub plugin_name: String,
    pub plugin_data: Option<Vec<String>>,
    pub rect: Rect
}

impl Dock {
    pub fn new(dock_handle: DockHandle) -> Dock {
        Dock {
            handle: dock_handle,
            plugin_name: "".to_owned(),
            plugin_data: None,
            rect: Rect::default(),
        }
    }
}
