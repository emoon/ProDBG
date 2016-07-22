// Copyright (c) 2015-2016, Johan Sköld.
// License: http://opensource.org/licenses/ISC

//! Raw FFI bgfx bindings.

#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

extern crate libc;

pub type va_list  = ::libc::c_void;
pub type size_t   = ::libc::size_t;
pub type int32_t  = i32;
pub type uint8_t  = u8;
pub type uint16_t = u16;
pub type uint32_t = u32;
pub type uint64_t = u64;

include!("ffi_bgfx.rs");
include!("ffi_bgfxplatform.rs");

pub const BGFX_PCI_ID_NONE:                 u16 = 0x0000;
pub const BGFX_PCI_ID_SOFTWARE_RASTERIZER:  u16 = 0x0001;
pub const BGFX_PCI_ID_AMD:                  u16 = 0x1002;
pub const BGFX_PCI_ID_INTEL:                u16 = 0x8086;
pub const BGFX_PCI_ID_NVIDIA:               u16 = 0x10de;

// Clear flags

pub const BGFX_CLEAR_NONE:                  u16 = 0x0000;
pub const BGFX_CLEAR_COLOR:                 u16 = 0x0001;
pub const BGFX_CLEAR_DEPTH:                 u16 = 0x0002;
pub const BGFX_CLEAR_STENCIL:               u16 = 0x0004;
pub const BGFX_CLEAR_DISCARD_COLOR_0:       u16 = 0x0008;
pub const BGFX_CLEAR_DISCARD_COLOR_1:       u16 = 0x0010;
pub const BGFX_CLEAR_DISCARD_COLOR_2:       u16 = 0x0020;
pub const BGFX_CLEAR_DISCARD_COLOR_3:       u16 = 0x0040;
pub const BGFX_CLEAR_DISCARD_COLOR_4:       u16 = 0x0080;
pub const BGFX_CLEAR_DISCARD_COLOR_5:       u16 = 0x0100;
pub const BGFX_CLEAR_DISCARD_COLOR_6:       u16 = 0x0200;
pub const BGFX_CLEAR_DISCARD_COLOR_7:       u16 = 0x0400;
pub const BGFX_CLEAR_DISCARD_DEPTH:         u16 = 0x0800;
pub const BGFX_CLEAR_DISCARD_STENCIL:       u16 = 0x1000;

pub const BGFX_CLEAR_DISCARD_COLOR_MASK:    u16 =
    (
        BGFX_CLEAR_DISCARD_COLOR_0 |
        BGFX_CLEAR_DISCARD_COLOR_1 |
        BGFX_CLEAR_DISCARD_COLOR_2 |
        BGFX_CLEAR_DISCARD_COLOR_3 |
        BGFX_CLEAR_DISCARD_COLOR_4 |
        BGFX_CLEAR_DISCARD_COLOR_5 |
        BGFX_CLEAR_DISCARD_COLOR_6 |
        BGFX_CLEAR_DISCARD_COLOR_7
    );

pub const BGFX_CLEAR_DISCARD_MASK:          u16 =
    (
        BGFX_CLEAR_DISCARD_COLOR_MASK |
        BGFX_CLEAR_DISCARD_DEPTH |
        BGFX_CLEAR_DISCARD_STENCIL
    );

// Debug flags

pub const BGFX_DEBUG_NONE:                  u32 = 0x00000000;
pub const BGFX_DEBUG_WIREFRAME:             u32 = 0x00000001;
pub const BGFX_DEBUG_IFH:                   u32 = 0x00000002;
pub const BGFX_DEBUG_STATS:                 u32 = 0x00000004;
pub const BGFX_DEBUG_TEXT:                  u32 = 0x00000008;

// Reset flags

pub const BGFX_RESET_NONE:                  u32 = 0x00000000;
pub const BGFX_RESET_FULLSCREEN:            u32 = 0x00000001;
pub const BGFX_RESET_FULLSCREEN_SHIFT:      u32 = 0;
pub const BGFX_RESET_FULLSCREEN_MASK:       u32 = 0x00000001;
pub const BGFX_RESET_MSAA_X2:               u32 = 0x00000010;
pub const BGFX_RESET_MSAA_X4:               u32 = 0x00000020;
pub const BGFX_RESET_MSAA_X8:               u32 = 0x00000030;
pub const BGFX_RESET_MSAA_X16:              u32 = 0x00000040;
pub const BGFX_RESET_MSAA_SHIFT:            u32 = 4;
pub const BGFX_RESET_MSAA_MASK:             u32 = 0x00000070;
pub const BGFX_RESET_VSYNC:                 u32 = 0x00000080;
pub const BGFX_RESET_MAXANISOTROPY:         u32 = 0x00000100;
pub const BGFX_RESET_CAPTURE:               u32 = 0x00000200;
pub const BGFX_RESET_HMD:                   u32 = 0x00000400;
pub const BGFX_RESET_HMD_DEBUG:             u32 = 0x00000800;
pub const BGFX_RESET_HMD_RECENTER:          u32 = 0x00001000;
pub const BGFX_RESET_FLUSH_AFTER_RENDER:    u32 = 0x00002000;
pub const BGFX_RESET_FLIP_AFTER_RENDER:     u32 = 0x00004000;
pub const BGFX_RESET_SRGB_BACKBUFFER:       u32 = 0x00008000;
pub const BGFX_RESET_HIDPI:                 u32 = 0x00010000;

// Buffer flags

pub const BGFX_BUFFER_NONE:                 u16 = 0x0000;
pub const BGFX_BUFFER_COMPUTE_FORMAT_8X1:   u16 = 0x0001;
pub const BGFX_BUFFER_COMPUTE_FORMAT_8X2:   u16 = 0x0002;
pub const BGFX_BUFFER_COMPUTE_FORMAT_8X4:   u16 = 0x0003;
pub const BGFX_BUFFER_COMPUTE_FORMAT_16X1:  u16 = 0x0004;
pub const BGFX_BUFFER_COMPUTE_FORMAT_16X2:  u16 = 0x0005;
pub const BGFX_BUFFER_COMPUTE_FORMAT_16X4:  u16 = 0x0006;
pub const BGFX_BUFFER_COMPUTE_FORMAT_32X1:  u16 = 0x0007;
pub const BGFX_BUFFER_COMPUTE_FORMAT_32X2:  u16 = 0x0008;
pub const BGFX_BUFFER_COMPUTE_FORMAT_32X4:  u16 = 0x0009;
pub const BGFX_BUFFER_COMPUTE_FORMAT_SHIFT: u16 = 0;
pub const BGFX_BUFFER_COMPUTE_FORMAT_MASK:  u16 = 0x000f;
pub const BGFX_BUFFER_COMPUTE_TYPE_UINT:    u16 = 0x0010;
pub const BGFX_BUFFER_COMPUTE_TYPE_INT:     u16 = 0x0020;
pub const BGFX_BUFFER_COMPUTE_TYPE_FLOAT:   u16 = 0x0030;
pub const BGFX_BUFFER_COMPUTE_TYPE_SHIFT:   u16 = 4;
pub const BGFX_BUFFER_COMPUTE_TYPE_MASK:    u16 = 0x0030;
pub const BGFX_BUFFER_COMPUTE_READ:         u16 = 0x0100;
pub const BGFX_BUFFER_COMPUTE_WRITE:        u16 = 0x0200;
pub const BGFX_BUFFER_DRAW_INDIRECT:        u16 = 0x0400;
pub const BGFX_BUFFER_ALLOW_RESIZE:         u16 = 0x0800;
pub const BGFX_BUFFER_INDEX32:              u16 = 0x1000;

pub const BGFX_BUFFER_COMPUTE_READ_WRITE:   u16 =
    (
        BGFX_BUFFER_COMPUTE_READ |
        BGFX_BUFFER_COMPUTE_WRITE
    );

// State flags

pub const BGFX_STATE_RGB_WRITE:             u64 = 0x0000000000000001_u64;
pub const BGFX_STATE_ALPHA_WRITE:           u64 = 0x0000000000000002_u64;
pub const BGFX_STATE_DEPTH_WRITE:           u64 = 0x0000000000000004_u64;
pub const BGFX_STATE_DEPTH_TEST_LESS:       u64 = 0x0000000000000010_u64;
pub const BGFX_STATE_DEPTH_TEST_LEQUAL:     u64 = 0x0000000000000020_u64;
pub const BGFX_STATE_DEPTH_TEST_EQUAL:      u64 = 0x0000000000000030_u64;
pub const BGFX_STATE_DEPTH_TEST_GEQUAL:     u64 = 0x0000000000000040_u64;
pub const BGFX_STATE_DEPTH_TEST_GREATER:    u64 = 0x0000000000000050_u64;
pub const BGFX_STATE_DEPTH_TEST_NOTEQUAL:   u64 = 0x0000000000000060_u64;
pub const BGFX_STATE_DEPTH_TEST_NEVER:      u64 = 0x0000000000000070_u64;
pub const BGFX_STATE_DEPTH_TEST_ALWAYS:     u64 = 0x0000000000000080_u64;
pub const BGFX_STATE_DEPTH_TEST_SHIFT:      u64 = 4_u64;
pub const BGFX_STATE_DEPTH_TEST_MASK:       u64 = 0x00000000000000f0_u64;
pub const BGFX_STATE_BLEND_ZERO:            u64 = 0x0000000000001000_u64;
pub const BGFX_STATE_BLEND_ONE:             u64 = 0x0000000000002000_u64;
pub const BGFX_STATE_BLEND_SRC_COLOR:       u64 = 0x0000000000003000_u64;
pub const BGFX_STATE_BLEND_INV_SRC_COLOR:   u64 = 0x0000000000004000_u64;
pub const BGFX_STATE_BLEND_SRC_ALPHA:       u64 = 0x0000000000005000_u64;
pub const BGFX_STATE_BLEND_INV_SRC_ALPHA:   u64 = 0x0000000000006000_u64;
pub const BGFX_STATE_BLEND_DST_ALPHA:       u64 = 0x0000000000007000_u64;
pub const BGFX_STATE_BLEND_INV_DST_ALPHA:   u64 = 0x0000000000008000_u64;
pub const BGFX_STATE_BLEND_DST_COLOR:       u64 = 0x0000000000009000_u64;
pub const BGFX_STATE_BLEND_INV_DST_COLOR:   u64 = 0x000000000000a000_u64;
pub const BGFX_STATE_BLEND_SRC_ALPHA_SAT:   u64 = 0x000000000000b000_u64;
pub const BGFX_STATE_BLEND_FACTOR:          u64 = 0x000000000000c000_u64;
pub const BGFX_STATE_BLEND_INV_FACTOR:      u64 = 0x000000000000d000_u64;
pub const BGFX_STATE_BLEND_SHIFT:           u64 = 12_u64;
pub const BGFX_STATE_BLEND_MASK:            u64 = 0x000000000ffff000_u64;
pub const BGFX_STATE_BLEND_EQUATION_ADD:    u64 = 0x0000000000000000_u64;
pub const BGFX_STATE_BLEND_EQUATION_SUB:    u64 = 0x0000000010000000_u64;
pub const BGFX_STATE_BLEND_EQUATION_REVSUB: u64 = 0x0000000020000000_u64;
pub const BGFX_STATE_BLEND_EQUATION_MIN:    u64 = 0x0000000030000000_u64;
pub const BGFX_STATE_BLEND_EQUATION_MAX:    u64 = 0x0000000040000000_u64;
pub const BGFX_STATE_BLEND_EQUATION_SHIFT:  u64 = 28_u64;
pub const BGFX_STATE_BLEND_EQUATION_MASK:   u64 = 0x00000003f0000000_u64;
pub const BGFX_STATE_BLEND_INDEPENDENT:     u64 = 0x0000000400000000_u64;
pub const BGFX_STATE_CULL_CW:               u64 = 0x0000001000000000_u64;
pub const BGFX_STATE_CULL_CCW:              u64 = 0x0000002000000000_u64;
pub const BGFX_STATE_CULL_SHIFT:            u64 = 36_u64;
pub const BGFX_STATE_CULL_MASK:             u64 = 0x0000003000000000_u64;
pub const BGFX_STATE_ALPHA_REF_SHIFT:       u64 = 40_u64;
pub const BGFX_STATE_ALPHA_REF_MASK:        u64 = 0x0000ff0000000000_u64;
pub const BGFX_STATE_PT_TRISTRIP:           u64 = 0x0001000000000000_u64;
pub const BGFX_STATE_PT_LINES:              u64 = 0x0002000000000000_u64;
pub const BGFX_STATE_PT_LINESTRIP:          u64 = 0x0003000000000000_u64;
pub const BGFX_STATE_PT_POINTS:             u64 = 0x0004000000000000_u64;
pub const BGFX_STATE_PT_SHIFT:              u64 = 48_u64;
pub const BGFX_STATE_PT_MASK:               u64 = 0x0007000000000000_u64;
pub const BGFX_STATE_POINT_SIZE_SHIFT:      u64 = 52_u64;
pub const BGFX_STATE_POINT_SIZE_MASK:       u64 = 0x0ff0000000000000_u64;
pub const BGFX_STATE_MSAA:                  u64 = 0x1000000000000000_u64;
pub const BGFX_STATE_RESERVED_MASK:         u64 = 0xe000000000000000_u64;
pub const BGFX_STATE_NONE:                  u64 = 0x0000000000000000_u64;
pub const BGFX_STATE_MASK:                  u64 = 0xffffffffffffffff_u64;

pub const BGFX_STATE_DEFAULT:               u64 =
    (
        BGFX_STATE_RGB_WRITE |
        BGFX_STATE_ALPHA_WRITE |
        BGFX_STATE_DEPTH_TEST_LESS |
        BGFX_STATE_DEPTH_WRITE |
        BGFX_STATE_CULL_CW |
        BGFX_STATE_MSAA
	);

#[macro_export]
macro_rules! BGFX_STATE_ALPHA_REF {
    ($aref:expr) => ((($aref as u64) << bgfx_sys::BGFX_STATE_ALPHA_REF_SHIFT) & bgfx_sys::BGFX_STATE_ALPHA_REF_MASK)
}

#[macro_export]
macro_rules! BGFX_STATE_POINT_SIZE {
    ($size:expr) => ((($size as u64) << bgfx_sys::BGFX_STATE_POINT_SIZE_SHIFT) & bgfx_sys::BGFX_STATE_POINT_SIZE_MASK)
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_SEPARATE {
    ($srcrgb:expr, $dstrgb:expr, $srca:expr, $dsta:expr) => (
        ($srcrgb as u64) | (($dstrgb as u64) << 4) | (($srca as u64) << 8) | (($dsta as u64) << 12)
    );
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_EQUATION_SEPARATE {
    ($rgb:expr, $a:expr) => (($rgb as u64) | (($a as u64) << 3))
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC {
    ($src:expr, $dst:expr) => (BGFX_STATE_BLEND_FUNC_SEPARATE!($src, $dst, $src, $dst))
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_EQUATION {
    ($equation:expr) => (BGFX_STATE_BLEND_EQUATION_SEPARATE!($equation, $equation))
}

pub const BGFX_STATE_BLEND_ADD:             u64 = (BGFX_STATE_BLEND_FUNC!(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_ONE          ) );
pub const BGFX_STATE_BLEND_ALPHA:           u64 = (BGFX_STATE_BLEND_FUNC!(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) );
pub const BGFX_STATE_BLEND_DARKEN:          u64 = (BGFX_STATE_BLEND_FUNC!(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_ONE          ) | BGFX_STATE_BLEND_EQUATION!(BGFX_STATE_BLEND_EQUATION_MIN) );
pub const BGFX_STATE_BLEND_LIGHTEN:         u64 = (BGFX_STATE_BLEND_FUNC!(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_ONE          ) | BGFX_STATE_BLEND_EQUATION!(BGFX_STATE_BLEND_EQUATION_MAX) );
pub const BGFX_STATE_BLEND_MULTIPLY:        u64 = (BGFX_STATE_BLEND_FUNC!(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO         ) );
pub const BGFX_STATE_BLEND_NORMAL:          u64 = (BGFX_STATE_BLEND_FUNC!(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_INV_SRC_ALPHA) );
pub const BGFX_STATE_BLEND_SCREEN:          u64 = (BGFX_STATE_BLEND_FUNC!(BGFX_STATE_BLEND_ONE,       BGFX_STATE_BLEND_INV_SRC_COLOR) );
pub const BGFX_STATE_BLEND_LINEAR_BURN:     u64 = (BGFX_STATE_BLEND_FUNC!(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_INV_DST_COLOR) | BGFX_STATE_BLEND_EQUATION!(BGFX_STATE_BLEND_EQUATION_SUB) );

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_RT_x {
    ($src:expr, $dst:expr) => (
        (($src >> bgfx_sys::BGFX_STATE_BLEND_SHIFT) as u32) | ((($dst >> bgfx_sys::BGFX_STATE_BLEND_SHIFT) as u32) << 4)
    );
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_RT_xE {
    ($src:expr, $dst:expr, $equation:expr) => (
        BGFX_STATE_BLEND_FUNC_RT_x!($src, $dst) | ((($equation >> bgfx_sys::BGFX_STATE_BLEND_EQUATION_SHIFT) as u32) << 8)
    );
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_RT_1 {
    ($src:expr, $dst:expr) => (BGFX_STATE_BLEND_FUNC_RT_x!($src, $dst))
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_RT_2 {
    ($src:expr, $dst:expr) => (BGFX_STATE_BLEND_FUNC_RT_x!($src, $dst) << 11)
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_RT_3 {
    ($src:expr, $dst:expr) => (BGFX_STATE_BLEND_FUNC_RT_x!($src, $dst) << 22)
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_RT_1E {
    ($src:expr, $dst:expr, $equation:expr) => (BGFX_STATE_BLEND_FUNC_RT_xE!($src, $dst, $equation))
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_RT_2E {
    ($src:expr, $dst:expr, $equation:expr) => (BGFX_STATE_BLEND_FUNC_RT_xE!($src, $dst, $equation) << 11)
}

#[macro_export]
macro_rules! BGFX_STATE_BLEND_FUNC_RT_3E {
    ($src:expr, $dst:expr, $equation:expr) => (BGFX_STATE_BLEND_FUNC_RT_xE!($src, $dst, $equation) << 22)
}

// Texture flags

pub const BGFX_TEXTURE_NONE:               u32 = 0x00000000_u32;
pub const BGFX_TEXTURE_U_MIRROR:           u32 = 0x00000001_u32;
pub const BGFX_TEXTURE_U_CLAMP :           u32 = 0x00000002_u32;
pub const BGFX_TEXTURE_U_BORDER:           u32 = 0x00000003_u32;
pub const BGFX_TEXTURE_U_SHIFT:            u32 = 0_u32;
pub const BGFX_TEXTURE_U_MASK  :           u32 = 0x00000003_u32;
pub const BGFX_TEXTURE_V_MIRROR:           u32 = 0x00000004_u32;
pub const BGFX_TEXTURE_V_CLAMP :           u32 = 0x00000008_u32;
pub const BGFX_TEXTURE_V_BORDER:           u32 = 0x0000000c_u32;
pub const BGFX_TEXTURE_V_SHIFT:            u32 = 2_u32;
pub const BGFX_TEXTURE_V_MASK  :           u32 = 0x0000000c_u32;
pub const BGFX_TEXTURE_W_MIRROR:           u32 = 0x00000010_u32;
pub const BGFX_TEXTURE_W_CLAMP :           u32 = 0x00000020_u32;
pub const BGFX_TEXTURE_W_BORDER:           u32 = 0x00000030_u32;
pub const BGFX_TEXTURE_W_SHIFT:            u32 = 4_u32;
pub const BGFX_TEXTURE_W_MASK  :           u32 = 0x00000030_u32;
pub const BGFX_TEXTURE_MIN_POINT:          u32 = 0x00000040_u32;
pub const BGFX_TEXTURE_MIN_ANISOTROPIC:    u32 = 0x00000080_u32;
pub const BGFX_TEXTURE_MIN_SHIFT:          u32 = 6_u32;
pub const BGFX_TEXTURE_MIN_MASK:           u32 = 0x000000c0_u32;
pub const BGFX_TEXTURE_MAG_POINT:          u32 = 0x00000100_u32;
pub const BGFX_TEXTURE_MAG_ANISOTROPIC:    u32 = 0x00000200_u32;
pub const BGFX_TEXTURE_MAG_SHIFT:          u32 = 8_u32;
pub const BGFX_TEXTURE_MAG_MASK:           u32 = 0x00000300_u32;
pub const BGFX_TEXTURE_MIP_POINT:          u32 = 0x00000400_u32;
pub const BGFX_TEXTURE_MIP_SHIFT:          u32 = 12_u32;
pub const BGFX_TEXTURE_MIP_MASK:           u32 = 0x00000400_u32;
pub const BGFX_TEXTURE_MSAA_SAMPLE:        u32 = 0x00000800_u32;
pub const BGFX_TEXTURE_RT:                 u32 = 0x00001000_u32;
pub const BGFX_TEXTURE_RT_MSAA_X2:         u32 = 0x00002000_u32;
pub const BGFX_TEXTURE_RT_MSAA_X4:         u32 = 0x00003000_u32;
pub const BGFX_TEXTURE_RT_MSAA_X8:         u32 = 0x00004000_u32;
pub const BGFX_TEXTURE_RT_MSAA_X16:        u32 = 0x00005000_u32;
pub const BGFX_TEXTURE_RT_MSAA_SHIFT:      u32 = 12_u32;
pub const BGFX_TEXTURE_RT_MSAA_MASK:       u32 = 0x00007000_u32;
pub const BGFX_TEXTURE_RT_WRITE_ONLY:      u32 = 0x00008000_u32;
pub const BGFX_TEXTURE_RT_MASK:            u32 = 0x0000f000_u32;
pub const BGFX_TEXTURE_COMPARE_LESS:       u32 = 0x00010000_u32;
pub const BGFX_TEXTURE_COMPARE_LEQUAL:     u32 = 0x00020000_u32;
pub const BGFX_TEXTURE_COMPARE_EQUAL:      u32 = 0x00030000_u32;
pub const BGFX_TEXTURE_COMPARE_GEQUAL:     u32 = 0x00040000_u32;
pub const BGFX_TEXTURE_COMPARE_GREATER:    u32 = 0x00050000_u32;
pub const BGFX_TEXTURE_COMPARE_NOTEQUAL:   u32 = 0x00060000_u32;
pub const BGFX_TEXTURE_COMPARE_NEVER:      u32 = 0x00070000_u32;
pub const BGFX_TEXTURE_COMPARE_ALWAYS:     u32 = 0x00080000_u32;
pub const BGFX_TEXTURE_COMPARE_SHIFT:      u32 = 16_u32;
pub const BGFX_TEXTURE_COMPARE_MASK:       u32 = 0x000f0000_u32;
pub const BGFX_TEXTURE_COMPUTE_WRITE:      u32 = 0x00100000_u32;
pub const BGFX_TEXTURE_SRGB:               u32 = 0x00200000_u32;
pub const BGFX_TEXTURE_BLIT_DST:           u32 = 0x00400000_u32;
pub const BGFX_TEXTURE_READ_BACK:          u32 = 0x00800000_u32;
pub const BGFX_TEXTURE_BORDER_COLOR_SHIFT: u32 = 24;
pub const BGFX_TEXTURE_BORDER_COLOR_MASK:  u32 = 0x0f000000_u32;
pub const BGFX_TEXTURE_RESERVED_SHIFT:     u32 = 2;
pub const BGFX_TEXTURE_RESERVED_MASK:      u32 = 0xf0000000_u32;


