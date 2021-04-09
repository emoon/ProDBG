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

struct Frame
{
    // Frame count
    i64 nr;
    
    // The long frame flipflop
    bool lof;
    
    // Value of the frame flipflop in the previous frame
    bool prevlof;
    
    template <class W>
    void operator<<(W& worker)
    {
        worker

        << nr
        << lof
        << prevlof;
    }
    
    Frame() : nr(0), lof(false) { }
    
    bool isLongFrame() const { return lof; }
    bool isShortFrame() const { return !lof; }
    int numLines() const { return lof ? 313 : 312; }
    int lastLine() const { return lof ? 312 : 311; }
    
    bool wasLongFrame() const { return prevlof; }
    bool wasShortFrame() const { return !prevlof; }
    int prevNumLines() const { return prevlof ? 313 : 312; }
    int prevLastLine() const { return prevlof ? 312 : 311; }

    // Advances one frame
    void next(bool laceBit)
    {
        nr++;
        prevlof = lof;
        
        // Toggle the long frame flipflop in interlace mode
        if (laceBit) { lof = !lof; }
    }
};
