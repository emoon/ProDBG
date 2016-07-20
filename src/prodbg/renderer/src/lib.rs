extern crate bgfx;
extern crate imgui_sys;

use std::io;
use std::io::Read;
use std::fs::File;
use std::os::raw::c_void;
use bgfx::{Bgfx, BgfxError, Program, Uniform, TransientIndexBuffer, TransientVertexBuffer };
use imgui_sys::Imgui;

#[repr(C)]
struct UiVertex {
    _x: f32,
    _y: f32,
    _u: f32,
    _v: f32,
    _c: u32,
}

#[inline]
fn mtx_ortho(left: f32, right: f32, bottom: f32, top: f32, near: f32, far: f32) -> [f32; 16] {
    let aa = 2.0 / (right - left);
    let bb = 2.0 / (top - bottom);
    let cc = 1.0 / (far - near);
    let dd = (left + right) / (left - right);
    let ee = (top + bottom) / (bottom - top);
    let ff = near / (near - far);

    [aa, 0.0, 0.0, 0.0,
     0.0, bb, 0.0, 0.0,
     0.0, 0.0, cc, 0.0,
     dd, ee, ff, 1.0]
}


impl UiVertex  {
    fn build_decl() -> bgfx::VertexDecl {
        bgfx::VertexDecl::new(None)
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8(true))
            .end()
    }
}

pub struct Renderer {
    pub bgfx: Box<Bgfx>,
    // TODO: I guess these needs to be moved when we support more than one window.
    pub width: u16,
    pub height: u16,
    tex_uniform: Option<bgfx::Uniform>,
    ui_shaders: Option<bgfx::Program>,
    texture: Option<bgfx::Texture>,
    ui_vertex_decl: bgfx::VertexDecl,
}

impl Renderer {
    pub fn new() -> Renderer {
        Renderer {
            bgfx: Box::new(Bgfx::new()),
            width: 0,
            height: 0,
            tex_uniform: None,
            ui_shaders: None,
            texture: None,
            ui_vertex_decl: UiVertex::build_decl(),
        }
    }

    pub fn setup_window(&mut self, handle: *mut c_void, width: u16, height: u16) -> Result<(), BgfxError> {
        try!(bgfx::PlatformData::new()
            .context(std::ptr::null_mut())
            .display(get_display_server())
            .window(handle)
            .apply());

        try!(Bgfx::init(bgfx::RendererType::Default, None, None));

        Imgui::setup(Some("data/font/source_code_pro/SourceCodePro-Medium.ttf"), 20.0, width as u32, height as u32);

        let font = Imgui::get_font_tex_data();

        // Create vertex stream declaration

        self.tex_uniform = Some(Uniform::new("s_tex", bgfx::UniformType::Int1, 1));

        let memory = bgfx::Memory::copy(font.data);

        self.texture = Some(bgfx::Texture::create_2d(font.width, font.height, 1, bgfx::TextureFormat::Rgba8, bgfx::TEXTURE_NONE, Some(&memory)));
        self.ui_shaders = Some(Self::load_shaders("imgui").unwrap());

        Bgfx::reset(width, height, bgfx::RESET_NONE);
        Bgfx::set_view_seq(0, true);

        self.width = width;
        self.height = height;

        Ok(())
    }

    pub fn pre_update(&self) {
        Bgfx::set_view_rect(0, 0, 0, self.width, self.height);
        Bgfx::set_view_clear(0, bgfx::CLEAR_COLOR | bgfx::CLEAR_DEPTH, 0x000101010, 1.0, 0);
        Bgfx::touch(0);
        // TODO: Correct delta time
        Imgui::pre_update(1.0 / 60.0);
    }

    pub fn update_size(&mut self, size: (usize, usize)) {
        if self.width != size.0 as u16 || self.height != size.1 as u16 {
            self.width = size.0 as u16;
            self.height = size.1 as u16;
            Bgfx::reset(self.width, self.height, bgfx::RESET_NONE);
        }
    }

    fn render_imgui(&self) {
        let draw_lists = Imgui::get_draw_data();
        let view = mtx_ortho(0.0, self.width as f32, self.height as f32, 0.0, -1.0, 0.0);

        Bgfx::set_view_transform(0, &view);
        Bgfx::set_texture(0, self.tex_uniform.as_ref().unwrap(), self.texture.as_ref().unwrap(), None);

        for &list in draw_lists {
            let cmd_buffer = unsafe { (*list).cmd_buffer.as_slice() };
            let idx_buffer = unsafe { (*list).idx_buffer.as_slice() };
            let vtx_buffer = unsafe { (*list).vtx_buffer.as_slice() };

            let mut tib = TransientIndexBuffer::new(idx_buffer.len());
            let mut tvb = TransientVertexBuffer::new(vtx_buffer.len(), &self.ui_vertex_decl);

            tib.update(&idx_buffer);
            tvb.update(&vtx_buffer);

            let mut offset = 0;

            for cmd in cmd_buffer {
                if cmd.texture_id != std::ptr::null_mut() {
                    let texture = bgfx::Texture::new_from_handle_ptr(cmd.texture_id);
                    Bgfx::set_texture(0, self.tex_uniform.as_ref().unwrap(), &texture, None);
                } else {
                    Bgfx::set_texture(0, self.tex_uniform.as_ref().unwrap(), self.texture.as_ref().unwrap(), None);
                }

                if cmd.elem_count > 0 {
                    Bgfx::set_transient_vertex_buffer(&tvb, 0, vtx_buffer.len() as u32);
                    Bgfx::set_transient_index_buffer(&tib, offset, cmd.elem_count);
                    Bgfx::set_state(bgfx::STATE_RGB_WRITE | bgfx::STATE_ALPHA_WRITE |
                                    bgfx::STATE_BLEND_ALPHA, None);
                    Bgfx::submit(0, self.ui_shaders.as_ref().unwrap());

                    offset += cmd.elem_count;
                }
            }
        }
    }

    pub fn post_update(&self) {
        Imgui::post_update();

        self.render_imgui();

        Bgfx::frame();
    }

    #[cfg(target_family="unix")]
    fn get_platform_path() -> &'static str {
        "data/shaders/pre_built/opengl"
    }

    #[cfg(target_family="windows")]
    fn get_platform_path() -> &'static str {
        "data/shaders/pre_built/d3d11"
    }

    fn load_file(name: &str) -> io::Result<Vec<u8>> {
        let mut data = Vec::new();
        let mut file = try!(File::open(name));
        try!(file.read_to_end(&mut data));
        data.push(0u8);  // make sure we end with 0 in case of strings
        Ok(data)
    }

    fn load_shaders(name: &str) -> io::Result<Program> {
        let fs_name = format!("{}/{}.fs", Self::get_platform_path(), name);
        let vs_name = format!("{}/{}.vs", Self::get_platform_path(), name);
        let fs_data = try!(Self::load_file(&fs_name));
        let vs_data = try!(Self::load_file(&vs_name));
        let fsh_mem = bgfx::Memory::copy(&fs_data);
        let vsh_mem = bgfx::Memory::copy(&vs_data);
        let vsh = bgfx::Shader::new(vsh_mem);
        let fsh = bgfx::Shader::new(fsh_mem);
        let program = bgfx::Program::new(vsh, fsh);
        Ok(program)
    }
}


// This is somewhat hacky but to depend on whole x11 bindings for just 1 function is a bit overkill
extern "C" {
    pub fn XOpenDisplay(arg0: *mut c_void) -> *mut c_void;
}

#[cfg(any(target_os="linux",
          target_os="freebsd",
          target_os="dragonfly",
          target_os="netbsd",
          target_os="openbsd"))]
fn get_display_server() -> *mut c_void {
    unsafe { XOpenDisplay(std::ptr::null_mut()) }
}

#[cfg(any(target_os="macos",
          target_os="windows"))]
fn get_display_server() -> *mut c_void {
    std::ptr::null_mut()
}


