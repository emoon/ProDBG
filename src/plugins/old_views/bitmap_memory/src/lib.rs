#[macro_use]
extern crate prodbg_api;

use prodbg_api::{View, Image, Ui, Reader, Writer, PluginHandler, Service, CViewCallbacks, Color};

struct BitmapView {
    dummy: i32,
    image: Option<Image>,
    image_data: Box<[Color]>,
}

impl View for BitmapView {
    fn new(ui: &Ui, _: &Service) -> Self {
        BitmapView {
            dummy: 0,
            image: ui.image_create_rgba(256, 256),
            image_data: vec![Color::from_u32(0); 256*256].into_boxed_slice(),
        }
    }

    fn update(&mut self, ui: &mut Ui, _: &mut Reader, _: &mut Writer) {
        for i in 0..self.image_data.len() {
            self.image_data[i] = Color::from_rgba((i & 0xff) as u32, 0, 0, 127);
        }

        self.image.as_mut().unwrap().update(&self.image_data);
        ui.image(self.image.as_ref().unwrap()).show();

        if ui.button("sthteeoh", None) {
            println!("yah");
        }

        self.dummy += 1;
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(PLUGIN, b"Bitmap View\0", BitmapView);
    plugin_handler.register_view(&PLUGIN);
}
