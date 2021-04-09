// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DDF.h"
#include <algorithm>

template <bool hires> void
DDF<hires>::compute(i16 ddfstrt, i16 ddfstop, u16 bplcon1)
{
    compute(strtEven, stopEven, ddfstrt, ddfstop, bplcon1 >> 4);
    compute(strtOdd,  stopOdd,  ddfstrt, ddfstop, bplcon1);
}

template <bool hires> void
DDF<hires>::compute(i16 &strt, i16 &stop, i16 ddfstrt, i16 ddfstop, int scroll)
{
    if (hires) {
                
        // Take the scroll value of BPLCON1 into account
        i16 hiresStrt = ddfstrt - ((scroll & 0x7) >> 1);
        
        // Align the start cycle to the start of the next fetch unit
        int hiresShift = (4 - (hiresStrt & 0b11)) & 0b11;
        
        // Compute the beginning of the fetch window
        strt = hiresStrt + hiresShift;
        
        // Compute the number of fetch units
        int fetchUnits = ((ddfstop - ddfstrt) + 15) >> 3;
        
        // Compute the end of the DDF window
        stop = std::min(strt + 8 * fetchUnits, 0xE0);

    } else {
        
        // Take the scroll value of BPLCON1 into account
        i16 loresStrt = ddfstrt - ((scroll & 0xF) >> 1);
        
        // Align ddfstrt at the start of the next fetch unit
        int loresShift = (8 - (loresStrt & 0b111)) & 0b111;
        
        // Compute the beginning of the fetch window
        strt = loresStrt + loresShift;
        
        // Compute the number of fetch units
        int fetchUnits = ((ddfstop - ddfstrt) + 15) >> 3;
        
        // Compute the end of the DDF window
        stop = std::min(strt + 8 * fetchUnits, 0xE0);
    }
}

template void DDF<true>::compute(i16 ddfstrt, i16 ddfstop, u16 bplcon1);
template void DDF<false>::compute(i16 ddfstrt, i16 ddfstop, u16 bplcon1);
