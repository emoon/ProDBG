#[macro_use]
extern crate prodbg_api;

use prodbg_api::*;

pub struct DmaView {
    dummy: i32,
    id_amiga_uae_dma_time: i32,
}

impl View for DmaView {
    fn new(_: &Ui, service: &Service) -> Self {
        DmaView {
            dummy: 0,
            id_amiga_uae_dma_time: service.get_id_register().register_id("AmigaUAEDmaTime") as i32,
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, _: &mut Writer) {
        if ui.button("sthteeoh", None) {
            println!("yah");
        }

        for event in reader.get_event() {
            if event == self.id_amiga_uae_dma_time {
                println!("Got dma data");
            }

            /*
            match event {
                0 => {

                }
            }
            */
        }

        self.dummy += 1;
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(DMA_VIEW_PLUGIN, b"Amiga UAE Dma View\0", DmaView);
    plugin_handler.register_view(&DMA_VIEW_PLUGIN);
}

