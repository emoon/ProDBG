// Copyright (c) 2015-2016, Johan Sköld.
// License: http://opensource.org/licenses/ISC

//! Rust wrapper around [bgfx].
//!
//! Before using this crate, ensure that you fullfill the build requirements for bgfx, as outlined
//! in its [documentation][bgfx building]. If you are compiling for an `msvc` target, make sure to
//! build this crate in a developer command prompt.
//!
//! ## Limitations
//!
//! - So far, only Windows, Linux, and OSX are supported.
//! - Far from all bgfx functionality is exposed. As more examples get ported, more functionality
//!   will be as well.
//!
//! *This API is still unstable, and very likely to change.*
//!
//! ## Basic Usage
//!
//! Before this crate can be used, some platform data must be initialized. See [`PlatformData`].
//!
//! ```ignore
//! bgfx::PlatformData::new()
//!     .context(std::ptr::null_mut())
//!     .display(std::ptr::null_mut())
//!     .window(std::ptr::null_mut())
//!     .apply()
//!     .expect("Could not set platform data");
//! ```
//!
//! Once the platform data has been initialized, a new thread should be spawned to act as the main
//! thread. This thread should call [`bgfx::init`] to initialize bgfx. The object returned by that
//! function should be used to access bgfx API calls.
//!
//! ```ignore
//! std::thread::spawn(|| {
//!     let bgfx = bgfx::init(bgfx::RendererType::Default, None, None)
//!         .expect("Failed to initialize bgfx");
//!     // ...
//! });
//! ```
//!
//! Finally, the real main thread should act as the render thread, and repeatedly be calling
//! [`bgfx::render_frame`].
//!
//! ```ignore
//! loop {
//!     // This is probably also where you will want to pump the window event queue.
//!     bgfx::render_frame();
//! }
//! ```
//!
//! See the examples for more in-depth usage.
//!
//! [bgfx]: https://github.com/bkaradzic/bgfx
//! [bgfx building]: https://bkaradzic.github.io/bgfx/build.html
//! [`bgfx::init`]: fn.init.html
//! [`bgfx::render_frame`]: fn.render_frame.html
//! [`PlatformData`]: struct.PlatformData.html

#[macro_use]
extern crate bgfx_sys;
#[macro_use]
extern crate bitflags;
extern crate libc;


// use std::marker::PhantomData;
use std::mem;
use std::ptr;
use std::ffi::CString;

pub mod flags;

pub use flags::*;

/// Autoselect adapter.
pub const PCI_ID_NONE: u16 = bgfx_sys::BGFX_PCI_ID_NONE;

/// Software rasterizer.
pub const PCI_ID_SOFTWARE_RASTERIZER: u16 = bgfx_sys::BGFX_PCI_ID_SOFTWARE_RASTERIZER;

/// AMD adapter.
pub const PCI_ID_AMD: u16 = bgfx_sys::BGFX_PCI_ID_AMD;

/// Intel adapter.
pub const PCI_ID_INTEL: u16 = bgfx_sys::BGFX_PCI_ID_INTEL;

/// nVidia adapter.
pub const PCI_ID_NVIDIA: u16 = bgfx_sys::BGFX_PCI_ID_NVIDIA;

/// Renderer backend type.
#[repr(u32)]
#[derive(PartialEq, Eq, Debug, Copy, Clone)]
pub enum RendererType {
    /// No rendering.
    Null = bgfx_sys::BGFX_RENDERER_TYPE_NULL,

    /// Direct3D 9.0.
    Direct3D9 = bgfx_sys::BGFX_RENDERER_TYPE_DIRECT3D9,

    /// Direct3D 11.0.
    Direct3D11 = bgfx_sys::BGFX_RENDERER_TYPE_DIRECT3D11,

    /// Direct3D 12.0.
    Direct3D12 = bgfx_sys::BGFX_RENDERER_TYPE_DIRECT3D12,

    /// Metal.
    Metal = bgfx_sys::BGFX_RENDERER_TYPE_METAL,

    /// OpenGLES.
    OpenGLES = bgfx_sys::BGFX_RENDERER_TYPE_OPENGLES,

    /// OpenGL.
    OpenGL = bgfx_sys::BGFX_RENDERER_TYPE_OPENGL,

    /// Vulkan.
    Vulkan = bgfx_sys::BGFX_RENDERER_TYPE_VULKAN,

    /// Use the most platform appropriate renderer.
    Default = bgfx_sys::BGFX_RENDERER_TYPE_COUNT,
}

impl RendererType {
    fn from_u32(n: u32) -> Option<RendererType> {
        if n <= bgfx_sys::BGFX_RENDERER_TYPE_COUNT {
            Some(unsafe { mem::transmute(n) })
        } else {
            None
        }
    }
}

/// `render_frame()` results.
#[repr(u32)]
#[derive(PartialEq, Eq, Debug, Copy, Clone)]
pub enum RenderFrame {
    /// No context is available. This usually means the main thread has exited.
    NoContext = bgfx_sys::BGFX_RENDER_FRAME_NO_CONTEXT,

    /// The render was performed.
    Render = bgfx_sys::BGFX_RENDER_FRAME_RENDER,

    /// The renderer is exiting.
    Exiting = bgfx_sys::BGFX_RENDER_FRAME_EXITING,
}

impl RenderFrame {
    fn from_u32(n: u32) -> Option<RenderFrame> {
        if n < bgfx_sys::BGFX_RENDER_FRAME_COUNT {
            Some(unsafe { mem::transmute(n) })
        } else {
            None
        }
    }
}

/// Vertex attribute.
#[repr(u32)]
#[derive(PartialEq, Eq, Debug, Copy, Clone)]
pub enum Attrib {
    /// Position.
    Position = bgfx_sys::BGFX_ATTRIB_POSITION,

    /// Normal.
    Normal = bgfx_sys::BGFX_ATTRIB_NORMAL,

    /// Tangent.
    Tangent = bgfx_sys::BGFX_ATTRIB_TANGENT,

    /// Bitangent.
    Bitangent = bgfx_sys::BGFX_ATTRIB_BITANGENT,

    /// Color 0.
    Color0 = bgfx_sys::BGFX_ATTRIB_COLOR0,

    /// Color 1.
    Color1 = bgfx_sys::BGFX_ATTRIB_COLOR1,

    /// Index list.
    Indices = bgfx_sys::BGFX_ATTRIB_INDICES,

    /// Bone weight.
    Weight = bgfx_sys::BGFX_ATTRIB_WEIGHT,

    /// Texture coordinate 0.
    TexCoord0 = bgfx_sys::BGFX_ATTRIB_TEXCOORD0,

    /// Texture coordinate 1.
    TexCoord1 = bgfx_sys::BGFX_ATTRIB_TEXCOORD1,

    /// Texture coordinate 2.
    TexCoord2 = bgfx_sys::BGFX_ATTRIB_TEXCOORD2,

    /// Texture coordinate 3.
    TexCoord3 = bgfx_sys::BGFX_ATTRIB_TEXCOORD3,

    /// Texture coordinate 4.
    TexCoord4 = bgfx_sys::BGFX_ATTRIB_TEXCOORD4,

    /// Texture coordinate 5.
    TexCoord5 = bgfx_sys::BGFX_ATTRIB_TEXCOORD5,

    /// Texture coordinate 6.
    TexCoord6 = bgfx_sys::BGFX_ATTRIB_TEXCOORD6,

    /// Texture coordinate 7.
    TexCoord7 = bgfx_sys::BGFX_ATTRIB_TEXCOORD7,
}

/// Vertex attribute type.
#[derive(PartialEq, Eq, Debug, Copy, Clone)]
pub enum AttribType {
    /// Unsigned 8-bit integer.
    ///
    /// If the parameter is `true`, the value will be normalized between 0 and 1.
    Uint8(bool),

    /// Signed 8-bit integer.
    ///
    /// If the parameter is `true`, the value will be normalized between 0 and 1.
    Int8(bool),

    /// Unsigned 10-bit integer.
    ///
    /// If the parameter is `true`, the value will be normalized between 0 and 1.
    Uint10(bool),

    /// Signed 10-bit integer.
    ///
    /// If the parameter is `true`, the value will be normalized between 0 and 1.
    Int10(bool),

    /// Unsigned 16-bit integer.
    ///
    /// If the parameter is `true`, the value will be normalized between 0 and 1.
    Uint16(bool),

    /// Signed 16-bit integer.
    ///
    /// If the parameter is `true`, the value will be normalized between 0 and 1.
    Int16(bool),

    /// 16-bit float.
    Half,

    /// 32-bit float.
    Float,
}

/// bgfx error.
#[derive(Debug)]
pub enum BgfxError {
    /// An invalid display was provided in the platform data.
    InvalidDisplay,

    /// An invalid window was provided in the platform data.
    InvalidWindow,

    /// Initialization failed.
    InitFailed,
}

/// bgfx-managed buffer of memory.
///
/// It can be created by either copying existing data through [`copy(...)`], or by referencing
/// existing memory directly through [`reference(...)`].
///
/// [`copy(...)`]: #method.copy
/// [`reference(...)`]: #method.reference
pub struct Memory {
    handle: *const bgfx_sys::bgfx_memory_t,
}

impl Memory {
    /// Copies the source data into a new bgfx-managed buffer.
    ///
    /// **IMPORTANT:** If this buffer is never passed into a bgfx call, the memory will never be
    /// freed, and will leak.
    #[inline]
    pub fn copy<T>(data: &[T]) -> Memory {
        unsafe {
            let handle = bgfx_sys::bgfx_copy(data.as_ptr() as *const libc::c_void,
                                             mem::size_of_val(data) as u32);
            Memory { handle: handle }
        }
    }

    /// Creates a reference to the source data for passing into bgfx. When using this constructor
    /// over the `copy` call, no copy will be created. bgfx will read the source memory directly.
    ///
    /// *Note that this is only valid for memory that will live for longer than the bgfx object,
    /// as it's the only way we can guarantee that the memory will still be valid until bgfx has
    /// finished using it.*
    #[inline]
    pub fn reference<T>(data: &[T]) -> Memory {
        unsafe {
            let handle = bgfx_sys::bgfx_make_ref(data.as_ptr() as *const libc::c_void,
                                                 mem::size_of_val(data) as u32);
            Memory { handle: handle }
        }
    }
}



/// Vertex attribute type.
#[repr(u32)]
#[derive(PartialEq, Eq, Debug, Copy, Clone)]
pub enum TextureFormat {
    Bc1 = bgfx_sys::BGFX_TEXTURE_FORMAT_BC1,
    Bc2 = bgfx_sys::BGFX_TEXTURE_FORMAT_BC2,
    Bc3 = bgfx_sys::BGFX_TEXTURE_FORMAT_BC3,
    Bc4 = bgfx_sys::BGFX_TEXTURE_FORMAT_BC4,
    Bc5 = bgfx_sys::BGFX_TEXTURE_FORMAT_BC5,
    Bc6h = bgfx_sys::BGFX_TEXTURE_FORMAT_BC6H,
    Bc7 = bgfx_sys::BGFX_TEXTURE_FORMAT_BC7,
    Etc1 = bgfx_sys::BGFX_TEXTURE_FORMAT_ETC1,
    Etc2 = bgfx_sys::BGFX_TEXTURE_FORMAT_ETC2,
    Etc2a = bgfx_sys::BGFX_TEXTURE_FORMAT_ETC2A,
    Etc2a1 = bgfx_sys::BGFX_TEXTURE_FORMAT_ETC2A1,
    Ptc12 = bgfx_sys::BGFX_TEXTURE_FORMAT_PTC12,
    Ptc14 = bgfx_sys::BGFX_TEXTURE_FORMAT_PTC14,
    Ptc12a = bgfx_sys::BGFX_TEXTURE_FORMAT_PTC12A,
    Ptc14a = bgfx_sys::BGFX_TEXTURE_FORMAT_PTC14A,
    Ptc22 = bgfx_sys::BGFX_TEXTURE_FORMAT_PTC22,
    Ptc24 = bgfx_sys::BGFX_TEXTURE_FORMAT_PTC24,

    Unknown = bgfx_sys::BGFX_TEXTURE_FORMAT_UNKNOWN,

    R1 = bgfx_sys::BGFX_TEXTURE_FORMAT_R1,
    A8 = bgfx_sys::BGFX_TEXTURE_FORMAT_A8,
    R8 = bgfx_sys::BGFX_TEXTURE_FORMAT_R8,
    R8i = bgfx_sys::BGFX_TEXTURE_FORMAT_R8I,
    R8u = bgfx_sys::BGFX_TEXTURE_FORMAT_R8U,
    R8s = bgfx_sys::BGFX_TEXTURE_FORMAT_R8S,
    R16 = bgfx_sys::BGFX_TEXTURE_FORMAT_R16,
    R16i = bgfx_sys::BGFX_TEXTURE_FORMAT_R16I,
    R16u = bgfx_sys::BGFX_TEXTURE_FORMAT_R16U,
    R16f = bgfx_sys::BGFX_TEXTURE_FORMAT_R16F,
    R16s = bgfx_sys::BGFX_TEXTURE_FORMAT_R16S,
    R32i = bgfx_sys::BGFX_TEXTURE_FORMAT_R32I,
    R32u = bgfx_sys::BGFX_TEXTURE_FORMAT_R32U,
    R32f = bgfx_sys::BGFX_TEXTURE_FORMAT_R32F,
    Rg8 = bgfx_sys::BGFX_TEXTURE_FORMAT_RG8,
    Rg8i = bgfx_sys::BGFX_TEXTURE_FORMAT_RG8I,
    Rg8u = bgfx_sys::BGFX_TEXTURE_FORMAT_RG8U,
    Rg8s = bgfx_sys::BGFX_TEXTURE_FORMAT_RG8S,
    Rg16 = bgfx_sys::BGFX_TEXTURE_FORMAT_RG16,
    Rg16i = bgfx_sys::BGFX_TEXTURE_FORMAT_RG16I,
    Rg16u = bgfx_sys::BGFX_TEXTURE_FORMAT_RG16U,
    Rg16f = bgfx_sys::BGFX_TEXTURE_FORMAT_RG16F,
    Rg16s = bgfx_sys::BGFX_TEXTURE_FORMAT_RG16S,
    Rg32i = bgfx_sys::BGFX_TEXTURE_FORMAT_RG32I,
    Rg32u = bgfx_sys::BGFX_TEXTURE_FORMAT_RG32U,
    Rg32f = bgfx_sys::BGFX_TEXTURE_FORMAT_RG32F,
    Rgb8 = bgfx_sys::BGFX_TEXTURE_FORMAT_RGB8,
    Rgb8i = bgfx_sys::BGFX_TEXTURE_FORMAT_RGB8I,
    Rgb8u = bgfx_sys::BGFX_TEXTURE_FORMAT_RGB8U,
    Rgb8s = bgfx_sys::BGFX_TEXTURE_FORMAT_RGB8S,
    Rgb9e5f = bgfx_sys::BGFX_TEXTURE_FORMAT_RGB9E5F,
    Bgra8 = bgfx_sys::BGFX_TEXTURE_FORMAT_BGRA8,
    Rgba8 = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA8,
    Rgba8i = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA8I,
    Rgba8u = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA8U,
    Rgba8s = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA8S,
    Rgba16 = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA16,
    Rgba16i = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA16I,
    Rgba16u = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA16U,
    Rgba16f = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA16F,
    Rgba16s = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA16S,
    Rgba32i = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA32I,
    Rgba32u = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA32U,
    Rgba32f = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA32F,
    R5g6b5 = bgfx_sys::BGFX_TEXTURE_FORMAT_R5G6B5,
    Rgba4 = bgfx_sys::BGFX_TEXTURE_FORMAT_RGBA4,
    Rgb5a1 = bgfx_sys::BGFX_TEXTURE_FORMAT_RGB5A1,
    Rgb10a2 = bgfx_sys::BGFX_TEXTURE_FORMAT_RGB10A2,
    R11g11b10f = bgfx_sys::BGFX_TEXTURE_FORMAT_R11G11B10F,

    UnknownDepth = bgfx_sys::BGFX_TEXTURE_FORMAT_UNKNOWN_DEPTH,

    D16 = bgfx_sys::BGFX_TEXTURE_FORMAT_D16,
    D24 = bgfx_sys::BGFX_TEXTURE_FORMAT_D24,
    D24s8 = bgfx_sys::BGFX_TEXTURE_FORMAT_D24S8,
    D32 = bgfx_sys::BGFX_TEXTURE_FORMAT_D32,
    D16f = bgfx_sys::BGFX_TEXTURE_FORMAT_D16F,
    D24f = bgfx_sys::BGFX_TEXTURE_FORMAT_D24F,
    D32f = bgfx_sys::BGFX_TEXTURE_FORMAT_D32F,
    D0s8 = bgfx_sys::BGFX_TEXTURE_FORMAT_D0S8,
}

/// Texture
///
/// Holds a texture
pub struct Texture {
    pub handle: bgfx_sys::bgfx_texture_handle_t,
}

impl Texture {
    #[inline]
    pub fn create_2d(width: u16,
                     height: u16,
                     mip_count: u8,
                     format: TextureFormat,
                     flags: TextureFlags,
                     mem: Option<&Memory>)
                     -> Texture {
        unsafe {
            let handle;

            if let Some(t) = mem {
                handle = bgfx_sys::bgfx_create_texture_2d(width,
                                                          height,
                                                          mip_count,
                                                          format as u32,
                                                          flags.bits(),
                                                          t.handle);
            } else {
                handle = bgfx_sys::bgfx_create_texture_2d(width,
                                                          height,
                                                          mip_count,
                                                          format as u32,
                                                          flags.bits(),
                                                          std::ptr::null_mut());
            }

            Texture { handle: handle }
        }
    }

    pub fn new_from_handle_ptr(handle: *mut std::os::raw::c_void) -> Texture {
        let t: u64 = unsafe { std::mem::transmute(handle) };
        let t: bgfx_sys::bgfx_texture_handle_t = unsafe { std::mem::transmute(t as u16) };
        Texture { handle: t }
    }

    // pub fn update(mip: u8, x: u16, y: u16, width: u16, height: u16, mem: &Memory, pitch: Option<u16>) {
    //
    // }
    //
}

pub struct TransientVertexBuffer {
    data: bgfx_sys::bgfx_transient_vertex_buffer_t,
}

impl TransientVertexBuffer {
    // TODO: Make this more safe (check available memory, and return result?)
    pub fn new(count: usize, decl: &VertexDecl) -> TransientVertexBuffer {
        unsafe {
            let mut tvb: TransientVertexBuffer = {
                mem::zeroed()
            };
            bgfx_sys::bgfx_alloc_transient_vertex_buffer(&mut tvb.data, count as u32, &decl.decl);
            tvb
        }
    }

    pub fn update<T>(&mut self, data: &[T]) {
        unsafe {
            std::ptr::copy_nonoverlapping(data.as_ptr() as *const libc::c_void,
                                          self.data.data as *mut libc::c_void,
                                          mem::size_of_val(data));
        }
    }
}

pub struct TransientIndexBuffer {
    data: bgfx_sys::bgfx_transient_index_buffer_t,
}

impl TransientIndexBuffer {
    // TODO: Make this more safe (check available memory, and return result?)
    pub fn new(count: usize) -> TransientIndexBuffer {
        unsafe {
            let mut tib: TransientIndexBuffer = {
                mem::zeroed()
            };
            bgfx_sys::bgfx_alloc_transient_index_buffer(&mut tib.data, count as u32);
            tib
        }
    }

    pub fn update<T>(&mut self, data: &[T]) {
        unsafe {
            std::ptr::copy_nonoverlapping(data.as_ptr() as *const libc::c_void,
                                          self.data.data as *mut libc::c_void,
                                          mem::size_of_val(data));
        }
    }
}


/// Uniform types.
#[repr(u32)]
#[derive(PartialEq, Eq, Debug, Copy, Clone)]
pub enum UniformType {
    Int1 = bgfx_sys::BGFX_UNIFORM_TYPE_INT1,
    End = bgfx_sys::BGFX_UNIFORM_TYPE_END,

    Vec4 = bgfx_sys::BGFX_UNIFORM_TYPE_VEC4,
    Mat3 = bgfx_sys::BGFX_UNIFORM_TYPE_MAT3,
    Mat4 = bgfx_sys::BGFX_UNIFORM_TYPE_MAT4,
}

/// Uniform
///
/// Holds a ref to a uniform
pub struct Uniform {
    pub handle: bgfx_sys::bgfx_uniform_handle_t,
}

impl Uniform {
    #[inline]
    pub fn new(name: &str, utype: UniformType, count: u16) -> Uniform {
        let n = CString::new(name).unwrap();
        unsafe {
            Uniform { handle: bgfx_sys::bgfx_create_uniform(n.as_ptr(), utype as u32, count) }
        }
    }
}

/// Shader program.
///
/// The program holds a vertex shader and a fragment shader.
pub struct Program {
    handle: bgfx_sys::bgfx_program_handle_t,
    _vsh: Shader,
    _fsh: Shader,
}

impl Program {
    /// Creates a new program from a vertex shader and a fragment shader. Ownerships of the shaders
    /// are moved to the program.
    #[inline]
    pub fn new(vsh: Shader, fsh: Shader) -> Program {
        unsafe {
            let handle = bgfx_sys::bgfx_create_program(vsh.handle, fsh.handle, 0);
            Program { handle: handle, _vsh: vsh, _fsh: fsh }
        }
    }
}

impl Drop for Program {
    #[inline]
    fn drop(&mut self) {
        unsafe { bgfx_sys::bgfx_destroy_program(self.handle) }
    }
}

/// Shader.
pub struct Shader {
    handle: bgfx_sys::bgfx_shader_handle_t,
}

impl Shader {
    /// Creates a new shader from bgfx-managed memory.
    #[inline]
    pub fn new(data: Memory) -> Shader {
        unsafe {
            let handle = bgfx_sys::bgfx_create_shader(data.handle);
            Shader { handle: handle }
        }
    }
}

impl Drop for Shader {
    #[inline]
    fn drop(&mut self) {
        println!("Shader Drop");
        unsafe { bgfx_sys::bgfx_destroy_shader(self.handle) }
    }
}

/// Vertex index buffer.
pub struct IndexBuffer {
    handle: bgfx_sys::bgfx_index_buffer_handle_t,
}

impl IndexBuffer {
    /// Creates a new index buffer from bgfx-managed memory.
    #[inline]
    pub fn new(indices: Memory, flags: BufferFlags) -> IndexBuffer {
        unsafe {
            let handle = bgfx_sys::bgfx_create_index_buffer(indices.handle, flags.bits());
            IndexBuffer { handle: handle }
        }
    }
}

impl Drop for IndexBuffer {
    #[inline]
    fn drop(&mut self) {
        unsafe { bgfx_sys::bgfx_destroy_index_buffer(self.handle) }
    }
}

/// Vertex data buffer.
pub struct VertexBuffer {
    handle: bgfx_sys::bgfx_vertex_buffer_handle_t,
}

impl VertexBuffer {
    /// Creates a new vertex buffer from bgfx-managed memory.
    #[inline]
    pub fn new(verts: &Memory, decl: &VertexDecl, flags: BufferFlags) -> VertexBuffer {
        unsafe {
            let handle =
                bgfx_sys::bgfx_create_vertex_buffer(verts.handle, &decl.decl, flags.bits());
            VertexBuffer { handle: handle }
        }
    }
}

impl Drop for VertexBuffer {
    #[inline]
    fn drop(&mut self) {
        unsafe { bgfx_sys::bgfx_destroy_vertex_buffer(self.handle) }
    }
}

/// Describes the structure of a vertex.
pub struct VertexDecl {
    decl: bgfx_sys::bgfx_vertex_decl_t,
}

impl VertexDecl {
    /// Creates a new vertex declaration using a [`VertexDeclBuilder`].
    ///
    /// # Example
    ///
    /// ```ignore
    /// let decl = bgfx::VertexDecl::new(None)
    ///                .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
    ///                .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8(true))
    ///                .end();
    /// ```
    ///
    /// [`VertexDeclBuilder`]: struct.VertexDeclBuilder.html
    #[inline]
    pub fn new(renderer: Option<RendererType>) -> VertexDeclBuilder {
        let renderer = renderer.unwrap_or(RendererType::Null) as bgfx_sys::bgfx_renderer_type_t;

        unsafe {
            let mut descr = VertexDeclBuilder { decl: mem::uninitialized() };
            bgfx_sys::bgfx_vertex_decl_begin(&mut descr.decl, renderer);
            descr
        }
    }
}

/// Builder for `VertexDecl` instances.
pub struct VertexDeclBuilder {
    decl: bgfx_sys::bgfx_vertex_decl_t,
}

impl VertexDeclBuilder {
    /// Adds a field to the structure descriptor. See [`VertexDecl::new`] for an example.
    ///
    /// [`VertexDecl::new`]: struct.VertexDecl.html#method.new
    pub fn add(&mut self, attrib: Attrib, count: u8, kind: AttribType) -> &mut Self {
        let mut normalized = false;
        let mut as_int = false;

        let kind = match kind {
            AttribType::Uint8(n) => {
                normalized = n;
                bgfx_sys::BGFX_ATTRIB_TYPE_UINT8
            }
            AttribType::Int8(n) => {
                normalized = n;
                as_int = true;
                bgfx_sys::BGFX_ATTRIB_TYPE_UINT8
            }
            AttribType::Uint10(n) => {
                normalized = n;
                bgfx_sys::BGFX_ATTRIB_TYPE_UINT10
            }
            AttribType::Int10(n) => {
                normalized = n;
                as_int = true;
                bgfx_sys::BGFX_ATTRIB_TYPE_UINT10
            }
            AttribType::Uint16(n) => {
                normalized = n;
                bgfx_sys::BGFX_ATTRIB_TYPE_INT16
            }
            AttribType::Int16(n) => {
                normalized = n;
                as_int = true;
                bgfx_sys::BGFX_ATTRIB_TYPE_INT16
            }
            AttribType::Half => bgfx_sys::BGFX_ATTRIB_TYPE_HALF,
            AttribType::Float => bgfx_sys::BGFX_ATTRIB_TYPE_FLOAT,
        };

        unsafe {
            bgfx_sys::bgfx_vertex_decl_add(&mut self.decl,
                                           attrib as bgfx_sys::bgfx_attrib_t,
                                           count,
                                           kind,
                                           if normalized { 1 } else { 0 },
                                           if as_int { 1 } else { 0 });
        }

        self
    }

    /// Finalizes the construction of the [`VertexDecl`].
    ///
    /// [`VertexDecl`]: struct.VertexDecl.html
    #[inline]
    pub fn end(&mut self) -> VertexDecl {
        unsafe {
            bgfx_sys::bgfx_vertex_decl_end(&mut self.decl);
        }

        VertexDecl { decl: self.decl }
    }

    /// Indicates a gap in the vertex structure.
    #[inline]
    pub fn skip(&mut self, bytes: u8) -> &mut Self {
        unsafe {
            bgfx_sys::bgfx_vertex_decl_skip(&mut self.decl, bytes);
        }

        self
    }
}

/// Acts as the library wrapper for bgfx. Any calls intended to be run on the main thread are
/// exposed as functions on this object.
///
/// It is created through a call to [`bgfx::init`], and will shut down bgfx when dropped.
///
/// [`bgfx::init`]: fn.init.html
pub struct Bgfx {
    _dummy: u32,
}

impl Bgfx {
    #[inline]
    pub fn new() -> Bgfx {
        Bgfx { _dummy: 0 }
    }

    /// Finish the frame, syncing up with the render thread. Returns an incrementing frame counter.
    #[inline]
    pub fn frame() -> u32 {
        unsafe { bgfx_sys::bgfx_frame() }
    }

    /// Gets the type of the renderer in use.
    #[inline]
    pub fn get_renderer_type() -> RendererType {
        unsafe { RendererType::from_u32(bgfx_sys::bgfx_get_renderer_type()).unwrap() }
    }

    /// Resets the graphics device to the given size, with the given flags.
    #[inline]
    pub fn reset(width: u16, height: u16, reset: ResetFlags) {
        unsafe { bgfx_sys::bgfx_reset(width as u32, height as u32, reset.bits()) }
    }

    /// Sets the debug flags to use.
    #[inline]
    pub fn set_debug(debug: DebugFlags) {
        unsafe { bgfx_sys::bgfx_set_debug(debug.bits()) }
    }

    /// Sets the index buffer to use for rendering.
    #[inline]
    pub fn set_index_buffer(ibh: &IndexBuffer) {
        // TODO: How to solve lifetimes...
        unsafe { bgfx_sys::bgfx_set_index_buffer(ibh.handle, 0, std::u32::MAX) }
    }

    /// Sets the render state.
    #[inline]
    pub fn set_state(state: StateFlags, rgba: Option<u32>) {
        unsafe { bgfx_sys::bgfx_set_state(state.bits(), rgba.unwrap_or(0)) }
    }

    /// Sets the model transform for rendering. If not called before submitting a draw, an identity
    /// matrix will be used.
    #[inline]
    pub fn set_transform(mtx: &[f32; 16]) {
        unsafe {
            bgfx_sys::bgfx_set_transform(mtx.as_ptr() as *const libc::c_void, 1);
        }
    }

    /// Set texture stage for draw primitive
    #[inline]
    pub fn set_texture(stage: u8,
                       sampler: &Uniform,
                       texture: &Texture,
                       flags: Option<TextureFlags>) {
        let flags = flags.unwrap_or(TEXTURE_U32_MAX).bits();
        unsafe {
            bgfx_sys::bgfx_set_texture(stage, sampler.handle, texture.handle, flags);
        }
    }

    /// Sets the vertex buffer to use for rendering.
    #[inline]
    pub fn set_vertex_buffer(vbh: &VertexBuffer) {
        unsafe { bgfx_sys::bgfx_set_vertex_buffer(vbh.handle, 0, std::u32::MAX) }
    }

    /// Sets the transient vertex buffer to use for rendering.
    #[inline]
    pub fn set_transient_vertex_buffer(tvb: &TransientVertexBuffer, offset: u32, count: u32) {
        unsafe {
            bgfx_sys::bgfx_set_transient_vertex_buffer(&tvb.data, offset, count);
        }
    }

    /// Sets the transient vertex buffer to use for rendering.
    #[inline]
    pub fn set_transient_index_buffer(tib: &TransientIndexBuffer, offset: u32, count: u32) {
        unsafe {
            bgfx_sys::bgfx_set_transient_index_buffer(&tib.data, offset, count);
        }
    }

    /// Sets the options to use when clearing the given view.
    #[inline]
    pub fn set_view_clear(id: u8, flags: ClearFlags, rgba: u32, depth: f32, stencil: u8) {
        unsafe { bgfx_sys::bgfx_set_view_clear(id, flags.bits(), rgba, depth, stencil) }
    }

    /// Sets the rectangle to display the given view in.
    #[inline]
    pub fn set_view_rect(id: u8, x: u16, y: u16, width: u16, height: u16) {
        unsafe { bgfx_sys::bgfx_set_view_rect(id, x, y, width, height) }
    }

    /// Sets the view and projection matrices for the given view.
    #[inline]
    // pub fn set_view_transform(id: u8, _view: &[f32; 16], proj: &[f32; 16]) {
    pub fn set_view_transform(id: u8, proj: &[f32; 16]) {
        unsafe {
            bgfx_sys::bgfx_set_view_transform(id,
                                              // view.as_ptr() as *const libc::c_void,
                                              std::ptr::null(),
                                              proj.as_ptr() as *const libc::c_void)
        }
    }

    /// Submit a primitive for rendering. Returns the number of draw calls used.
    #[inline]
    pub fn submit(view: u8, program: &Program) -> u32 {
        unsafe { bgfx_sys::bgfx_submit(view, program.handle, 0) }
    }

    /// Touches a view.
    #[inline]
    pub fn touch(id: u8) {
        unsafe {
            bgfx_sys::bgfx_touch(id);
        }
    }

    /// Set view into sequential mode. Draw calls will be sorted in the same order in which submit calls were called.
    #[inline]
    pub fn set_view_seq(view: u8, enabled: bool) {
        let e = if enabled { 1 } else { 0 };
        unsafe { bgfx_sys::bgfx_set_view_seq(view, e) }
    }

    /// Initializes bgfx.
    ///
    /// This must be called on the main thread after setting the platform data. See [`PlatformData`].
    ///
    /// [`PlatformData`]: struct.PlatformData.html
    pub fn init(renderer: RendererType,
                vendor_id: Option<u16>,
                device_id: Option<u16>)
                -> Result<Bgfx, BgfxError> {
        let renderer = renderer as bgfx_sys::bgfx_renderer_type_t;
        let vendor = vendor_id.unwrap_or(PCI_ID_NONE);
        let device = device_id.unwrap_or(0);

        unsafe {
            let success =
                bgfx_sys::bgfx_init(renderer, vendor, device, ptr::null_mut(), ptr::null_mut());

            if success != 0 { Ok(Bgfx::new()) } else { Err(BgfxError::InitFailed) }
        }
    }
}

// impl Drop for Bgfx {
// fn drop(&mut self) {
// unsafe { bgfx_sys::bgfx_shutdown() }
// }
// }
//

/// Pump the render thread.
///
/// This should be called repeatedly on the render thread.
#[inline]
pub fn render_frame() -> RenderFrame {
    unsafe { RenderFrame::from_u32(bgfx_sys::bgfx_render_frame()).unwrap() }
}

/// Platform data initializer.
///
/// This should be applied *only once*, before bgfx is used.
///
/// # Example
///
/// ```ignore
/// // Note: The default value for all of these options is null. If that is what you want, you may
/// // choose not to call said setter.
/// bgfx::PlatformData::new()
///     .context(std::ptr::null_mut())
///     .display(std::ptr::null_mut()) // Must be non-null on unix platforms
///     .window(std::ptr::null_mut()) // Must be non-null
///     .apply()
///     .expect("Could not set platform data");
/// ```
pub struct PlatformData {
    data: bgfx_sys::Struct_bgfx_platform_data,
}

impl PlatformData {
    /// Creates an empty PlatformData instance.
    #[inline]
    pub fn new() -> PlatformData {
        PlatformData {
            data: bgfx_sys::Struct_bgfx_platform_data {
                ndt: ptr::null_mut(),
                nwh: ptr::null_mut(),
                context: ptr::null_mut(),
                backBuffer: ptr::null_mut(),
                backBufferDS: ptr::null_mut(),
            },
        }
    }

    /// Apply the platform configuration.
    pub fn apply(&mut self) -> Result<(), BgfxError> {
        if self.data.ndt == ptr::null_mut() && cfg!(target_os = "linux") {
            Err(BgfxError::InvalidDisplay)
        } else if self.data.nwh == ptr::null_mut() {
            Err(BgfxError::InvalidWindow)
        } else {
            unsafe {
                bgfx_sys::bgfx_set_platform_data(&mut self.data);
            }
            Ok(())
        }
    }

    /// Sets the GL context to use.
    #[inline]
    pub fn context(&mut self, context: *mut libc::c_void) -> &mut Self {
        self.data.context = context;
        self
    }

    /// Sets the X11 display to use on unix systems.
    #[inline]
    pub fn display(&mut self, display: *mut std::os::raw::c_void) -> &mut Self {
        self.data.ndt = display as *mut libc::c_void;
        self
    }

    /// Sets the handle to the window to use.
    #[inline]
    pub fn window(&mut self, window: *mut std::os::raw::c_void) -> &mut Self {
        self.data.nwh = window as *mut libc::c_void;
        self
    }
}
