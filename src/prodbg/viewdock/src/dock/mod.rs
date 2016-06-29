mod serialize;

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

#[cfg(test)]
mod test {
    extern crate serde_json;
    use {Dock, DockHandle, Rect};

    #[test]
    fn test_dockhandle_serialize() {
        let handle_in = DockHandle(0x1337);
        let serialized = serde_json::to_string(&handle_in).unwrap();
        let handle_out: DockHandle = serde_json::from_str(&serialized).unwrap();

        assert_eq!(handle_in, handle_out);
    }

    #[test]
    fn test_dock_serialize_0() {
        let dock_in = Dock {
            handle: DockHandle(1),
            plugin_name: "disassembly".to_owned(),
            plugin_data: None,
            rect: Rect::new(1.0, 2.0, 3.0, 4.0)
        };

        let serialized = serde_json::to_string(&dock_in).unwrap();
        let dock_out: Dock = serde_json::from_str(&serialized).unwrap();

        assert_eq!(dock_in.handle, dock_out.handle);
        assert_eq!(dock_in.plugin_name, dock_out.plugin_name);
        assert_eq!(dock_in.plugin_data, dock_out.plugin_data);
        // expect that rect is not serialized and set to zero
        assert_eq!(dock_out.rect.x as i32, 0);
        assert_eq!(dock_out.rect.y as i32, 0);
        assert_eq!(dock_out.rect.width as i32, 0);
        assert_eq!(dock_out.rect.height as i32, 0);
    }

    #[test]
    fn test_dock_serialize_1() {
        let dock_in = Dock {
            handle: DockHandle(1),
            plugin_name: "registers".to_owned(),
            plugin_data: Some(vec!["some_data".to_owned(), "more_data".to_owned()]),
            rect: Rect::new(4.0, 5.0, 2.0, 8.0)
        };

        let serialized = serde_json::to_string(&dock_in).unwrap();
        let dock_out: Dock = serde_json::from_str(&serialized).unwrap();

        assert_eq!(dock_in.handle, dock_out.handle);
        assert_eq!(dock_in.plugin_name, dock_out.plugin_name);
        assert_eq!(dock_in.plugin_data, dock_out.plugin_data);

        // expect that rect is not serialized and set to zero
        assert_eq!(dock_out.rect.x as i32, 0);
        assert_eq!(dock_out.rect.y as i32, 0);
        assert_eq!(dock_out.rect.width as i32, 0);
        assert_eq!(dock_out.rect.height as i32, 0);

        let plugin_data = dock_out.plugin_data.as_ref().unwrap();

        assert_eq!(plugin_data.len(), 2);
        assert_eq!(plugin_data[0], "some_data");
        assert_eq!(plugin_data[1], "more_data");
    }

}