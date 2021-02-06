// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Sampler.h"

template <SamplingMethod method> i16
Sampler::interpolate(Cycle clock)
{
    assert(!isEmpty());

    int r1 = r;
    int r2 = next(r1);

    // Remove all outdated entries
    while (r2 != w && elements[r2].tag <= clock) {
        (void)read();
        r1 = r2;
        r2 = next(r1);
    }

    // If the buffer contains a single element only, return that element
    if (r2 == w) {
        return elements[r1].sample;
    }

    // Interpolate between position r1 and r2
    Cycle c1 = elements[r1].tag;
    Cycle c2 = elements[r2].tag;
    i16 s1 = elements[r1].sample;
    i16 s2 = elements[r2].sample;
    
    /*
    if (!(clock >= c1 && clock < c2)) {
        printf("WARNING: clock: %lld count: %d r: %d w: %d r1: %d r2: %d c1: %lld c2: %lld\n", clock, count(), r, w, r1, r2, c1, c2);
        return 0;
        
    }
    */
    assert(clock >= c1 && clock < c2);

    switch (method) {

        case SMP_NONE:
        {
            return s1;
        }
        case SMP_NEAREST:
        {
            if (clock - c1 < c2 - clock) {
                return s1;
            } else {
                return s2;
            }
        }
        case SMP_LINEAR:
        {
            double dx = (double)(c2 - c1);
            double dy = (double)(s2 - s1);
            double weight = (double)(clock - c1) / dx;
            return (i16)(s1 + weight * dy);
        }
        default:
            assert(false);
            return 0;
    }
}

template i16 Sampler::interpolate<SMP_NONE>(Cycle clock);
template i16 Sampler::interpolate<SMP_NEAREST>(Cycle clock);
template i16 Sampler::interpolate<SMP_LINEAR>(Cycle clock);
