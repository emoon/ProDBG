// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Constants.h"

struct Beam
{
    // Counters for the vertical and horizontal beam position
    i16 v;
    i16 h;
    
    // Latches counter values (recorded when BPLCON0::ERSY is set)
    i16 vLatched;
    i16 hLatched;

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << v
        << h
        << vLatched
        << hLatched;
    }
    
    Beam(i16 v, i16 h) : v(v), h(h) { }
    Beam(isize cycle = 0) : Beam((i16)(cycle / HPOS_CNT), (i16)(cycle % HPOS_CNT)) { }

    Beam& operator=(const Beam& beam)
    {
        v = beam.v;
        h = beam.h;
        return *this;
    }

    bool operator==(const Beam& beam) const
    {
        return v == beam.v && h == beam.h;
    }

    bool operator!=(const Beam& beam) const
    {
        return v != beam.v || h != beam.h;
    }

    Beam& operator+=(const Beam& beam)
    {
        v += beam.v;
        h += beam.h;

        if (h >= HPOS_CNT) { h -= HPOS_CNT; v++; }
        else if (h < 0)    { h += HPOS_CNT; v--; }

        return *this;
    }

    Beam operator+(const Beam& beam) const
    {
        i16 newv = v + beam.v;
        i16 newh = h + beam.h;

        if (newh >= HPOS_CNT) { newh -= HPOS_CNT; newv++; }
        else if (newh < 0)    { newh += HPOS_CNT; newv--; }

        return Beam(newv, newh);
    }

    Beam operator+(const isize i) const
    {
        return *this + Beam(i);
    }

    int operator-(const Beam& beam) const
    {
        return (v * HPOS_CNT + h) - (beam.v * HPOS_CNT + beam.h);
    }

    Beam& operator++()
    {
        if (++h > HPOS_MAX) { v++; h = 0; }
        return *this;
    }

    Beam& operator--()
    {
        if (--h < 0) { v--; h = HPOS_MAX; }
        return *this;
    }
};
