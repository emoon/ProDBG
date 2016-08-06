#[macro_use]
extern crate prodbg_api;

use prodbg_api::*;

pub struct DmaView {
    image_data: Box<[Color]>,
    image: Option<Image>,
    id_amiga_uae_dma_time: i32,
}

impl DmaView {
    fn update_dma_view(&mut self, ui: &Ui, reader: &mut Reader) {
        let lines = reader.find_u16("line").ok().unwrap() as usize;
        let xcount = reader.find_u16("xcount").ok().unwrap() as usize;
        let data = reader.find_data("data").ok().unwrap();

        let colors = [Color::from_u32(0x2222227f),
                      Color::from_u32(0x4444447f), // DMARECORD_REFRESH
                      Color::from_u32(0x8888887f), // DMARECORD_CPU
                      Color::from_u32(0xeeee007f), // DMARECORD_COPPER
                      Color::from_u32(0xff00007f), // DMARECORD_AUDIO
                      Color::from_u32(0x0088887f), // DMARECORD_BLITTER
                      Color::from_u32(0x0088ff7f), // DMARECORD_BLITTER_FILL 6
                      Color::from_u32(0x00ff007f), // DMARECORD_BLITTER_LINE 7
                      Color::from_u32(0x0000ff7f), // DMARECORD_BITPLANE 8
                      Color::from_u32(0xff00ff7f), // DMARECORD_SPRITE 9
                      Color::from_u32(0xffffff7f) /* DMARECORD_DISK 10 */];

        let mut index = 1;

        for line in 0..lines - 1 {
            for x in 0..xcount {
                self.image_data[(line * 256) + x] = colors[data[index] as usize];
                index += 2;
            }
        }

        let image = self.image.as_mut().unwrap();

        image.update(&self.image_data);
        ui.image(image).show();
    }
}

impl View for DmaView {
    fn new(ui: &Ui, service: &Service) -> Self {
        DmaView {
            image: ui.image_create_rgba(256, 512),
            image_data: vec![Color::from_u32(0); 256*512].into_boxed_slice(),
            id_amiga_uae_dma_time: service.get_id_register().register_id("AmigaUAEDmaTime") as i32,
        }
    }

    fn update(&mut self, ui: &mut Ui, reader: &mut Reader, _: &mut Writer) {
        for event in reader.get_event() {
            if event == self.id_amiga_uae_dma_time {
                println!("Got dma data");
                self.update_dma_view(ui, reader);
            }
        }
    }
}

#[no_mangle]
pub fn init_plugin(plugin_handler: &mut PluginHandler) {
    define_view_plugin!(DMA_VIEW_PLUGIN, b"Amiga UAE Dma View\0", DmaView);
    plugin_handler.register_view(&DMA_VIEW_PLUGIN);
}
