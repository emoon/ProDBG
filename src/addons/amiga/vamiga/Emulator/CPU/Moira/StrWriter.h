// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <math.h>

namespace moira {

//
// Wrapper structures controlling the output format
//

struct Int        { i32 raw;        Int(i32 v) : raw(v) { } };
struct UInt       { u32 raw;       UInt(u32 v) : raw(v) { } };
struct UInt8      { u8  raw;      UInt8(u8  v) : raw(v) { } };
struct UInt16     { u16 raw;     UInt16(u16 v) : raw(v) { } };
struct UInt32     { u32 raw;     UInt32(u32 v) : raw(v) { } };
struct Dn         { int raw;         Dn(int v) : raw(v) { } };
struct An         { int raw;         An(int v) : raw(v) { } };
struct Rn         { int raw;         Rn(int v) : raw(v) { } };
struct Imu        { u32 raw;        Imu(u32 v) : raw(v) { } };
struct Ims        { i32 raw;        Ims(i32 v) : raw(v) { } };
struct Imd        { u32 raw;        Imd(u32 v) : raw(v) { } };
struct Scale      { int raw;      Scale(int v) : raw(v) { } };
struct Align      { int raw;      Align(int v) : raw(v) { } };
struct RegList    { u16 raw;    RegList(u16 v) : raw(v) { } };
struct RegRegList { u16 raw; RegRegList(u16 v) : raw(v) { } };

template <Instr I>        struct Ins { };
template <Size S>         struct Sz  { };
template <Mode M, Size S> struct Ea  { u32 pc; u16 reg; u32 ext1; u32 ext2; u32 ext3; };

struct Finish     { };

class StrWriter
{
    char comment[32];  // Appended to the end of the disassembled string
    char *base;        // Start address of the destination string
    char *ptr;         // Current writing position
    bool hex;          // Number format: Hexadecimal / Decimal
    bool upper;        // Text format: Upper case / Lower case

public:

    StrWriter(char *p, bool h, bool u) : base(p), ptr(p), hex(h), upper(u)
    {
        comment[0] = 0;
    };

    //
    // Printing instruction fragments
    //

    StrWriter& operator<<(const char *str);
    StrWriter& operator<<(int i);
    StrWriter& operator<<(Int i);
    StrWriter& operator<<(UInt u);
    StrWriter& operator<<(UInt8 value);
    StrWriter& operator<<(UInt16 value);
    StrWriter& operator<<(UInt32 value);
    StrWriter& operator<<(Dn dn);
    StrWriter& operator<<(An an);
    StrWriter& operator<<(Rn rn);
    StrWriter& operator<<(Imu im);
    StrWriter& operator<<(Ims im);
    StrWriter& operator<<(Imd im);
    StrWriter& operator<<(Scale s);
    StrWriter& operator<<(Align align);
    StrWriter& operator<<(RegRegList l);
    StrWriter& operator<<(RegList l);
    template <Instr I> StrWriter& operator<<(Ins<I> i);
    template <Size S> StrWriter& operator<<(Sz<S> sz);
    template <Mode M, Size S> StrWriter& operator<<(const Ea<M,S> &ea);
    StrWriter& operator<<(Finish finish);

private:

    template <Mode M, Size S> void briefExtension(const Ea<M,S> &ea);
    template <Mode M, Size S> void fullExtension(const Ea<M,S> &ea);
};

}
