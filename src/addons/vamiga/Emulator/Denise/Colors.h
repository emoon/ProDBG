// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

struct RgbColor {

    double r;
    double g;
    double b;

    RgbColor() : r(0), g(0), b(0) {}
    RgbColor(double rv, double gv, double bv) : r(rv), g(gv), b(bv) {}
    RgbColor(u8 rv, u8 gv, u8 bv) : r(rv / 255.0), g(gv / 255.0), b(bv / 255.0) {}
    RgbColor(const struct YuvColor &c);
    RgbColor(const struct AmigaColor &c);
    RgbColor(const struct GpuColor &c);

    static const RgbColor black;
    static const RgbColor white;
    static const RgbColor red;
    static const RgbColor green;
    static const RgbColor blue;
    static const RgbColor yellow;
    static const RgbColor magenta;
    static const RgbColor cyan;

    RgbColor mix(RgbColor additive, double weight) const;
    RgbColor tint(double weight) const { return mix(white, weight); }
    RgbColor shade(double weight) const { return mix(black, weight); }
};

struct YuvColor {

    double y;
    double u;
    double v;

    YuvColor() : y(0), u(0), v(0) { }
    YuvColor(double yv, double uv, double vv) : y(yv), u(uv), v(vv) { }
    YuvColor(u8 yv, u8 uv, u8 vv) : y(yv / 255.0), u(uv / 255.0), v(vv / 255.0) { }
    YuvColor(const struct RgbColor &c);
    YuvColor(const struct AmigaColor &c) : YuvColor(RgbColor(c)) { }
    YuvColor(const struct GpuColor &c) : YuvColor(RgbColor(c)) { }

    static const YuvColor black;
    static const YuvColor white;
    static const YuvColor red;
    static const YuvColor green;
    static const YuvColor blue;
    static const YuvColor yellow;
    static const YuvColor magenta;
    static const YuvColor cyan;
};

//
// Amiga color (native Amiga RGB format)
//

struct AmigaColor {

    u16 rawValue;

    AmigaColor() : rawValue(0) {}
    AmigaColor(u16 v) : rawValue(v) {}
    AmigaColor(const struct RgbColor &c);
    AmigaColor(const struct YuvColor &c) : AmigaColor(RgbColor(c)) { }
    AmigaColor(const struct GpuColor &c);

    u16 r() const { return (rawValue >> 4) & 0xF0; }
    u16 g() const { return (rawValue >> 0) & 0xF0; }
    u16 b() const { return (rawValue << 4) & 0xF0; }

    static const AmigaColor black;
    static const AmigaColor white;
    static const AmigaColor red;
    static const AmigaColor green;
    static const AmigaColor blue;
    static const AmigaColor yellow;
    static const AmigaColor magenta;
    static const AmigaColor cyan;
};

//
// GPU color (native GPU RGBA format)
//

struct GpuColor {

    u32 rawValue;

    GpuColor() : rawValue(0) {}
    GpuColor(u32 v) : rawValue(v) {}
    GpuColor(const struct RgbColor &c);
    GpuColor(const struct AmigaColor &c);
    GpuColor(u8 r, u8 g, u8 b);

    static const GpuColor black;
    static const GpuColor white;
    static const GpuColor red;
    static const GpuColor green;
    static const GpuColor blue;
    static const GpuColor yellow;
    static const GpuColor magenta;
    static const GpuColor cyan;

    GpuColor mix(const struct RgbColor &color, double weight) const;
    GpuColor tint(double weight) const { return mix(RgbColor::white, weight); }
    GpuColor shade(double weight) const { return mix(RgbColor::black, weight); }
};
