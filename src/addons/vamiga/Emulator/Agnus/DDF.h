// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DDF_H
#define _DDF_H

#include <sys/param.h>
#include "Aliases.h"

template <bool hires>
struct DDF
{
    // First bitplane DMA cycle of odd and even bitplanes
    i16 strtOdd;
    i16 strtEven;
    
    // Last bitplane DMA cycle + 1 of odd and even bitplanes
    i16 stopOdd;
    i16 stopEven;

    DDF() : strtOdd(0), strtEven(0), stopOdd(0), stopEven(0) { }

    template <class T> void applyToItems(T& worker) {
        
        worker
        
        & strtOdd
        & strtEven
        & stopOdd
        & stopEven;
    }
    
    void clear() { strtOdd = strtEven = stopOdd = stopEven = 0; }
    
    bool operator==(const DDF& ddf) const
    {
        return
        strtOdd == ddf.strtOdd &&
        strtEven == ddf.strtEven &&
        stopOdd == ddf.stopOdd &&
        stopEven == ddf.stopEven;
    }

    bool operator!=(const DDF& ddf) const
    {
        return !(*this == ddf);
    }
    
    bool inRangeOdd(i16 pos) { return pos > strtOdd && pos < stopOdd; }
    bool inRangeEven(i16 pos) { return pos > strtEven && pos < stopEven; }
    bool oddAndEvenDiffer() { return strtOdd != strtEven || stopOdd != stopEven; }
    
    /* Computes a DDF window
     *
     *       strt : Cycle number from DDFSTRT
     *       stop : Cycle number from DDFSTOP
     *     scroll : Scroll value from BPLCON1
     *
     * The function assumes that stop is greater than strt. Other combinations
     * are not yet supported by the emulator.
     */
    void compute(i16 ddfstrt, i16 ddfstop, u16 bplcon1);
    void compute(i16 &strt, i16 &stop, i16 ddfstrt, i16 ddfstop, int scroll);
};

#endif
