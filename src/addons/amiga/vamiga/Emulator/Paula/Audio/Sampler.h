// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "SamplerTypes.h"
#include "Constants.h"
#include "RingBuffer.h"
#include "Reflection.h"

/* This buffer type is used to temporarily store the generated sound samples as
 * they are produced by the state machine. Note that the state machine doesn't
 * output samples at a constant sampling rate. Instead, a new sound sample is
 * generated whenever the period counter underflows. To preserve this timing
 * information, each sample is tagged by the cycle it was produced.
 */

struct TaggedSample
{
    Cycle tag;
    i16   sample;
};

struct Sampler : util::RingBuffer <TaggedSample, VPOS_CNT * HPOS_CNT> {
    
    /* Initializes the ring buffer by removing all existing elements and adding
     * a single dummy element. The dummy element is added because some methods
     * assume that the buffer is never empty.
     */
    void reset();

    // Clones another Sampler
    void clone(Sampler &other);
     
    /* Interpolates a sound sample for the specified target cycle. Two major
     * steps are involved. In the first step, the function computes index
     * position r1 with the following property:
     *
     *     Cycle of sample at r1 <= Target cycle < Cycle of sample at r1
     *
     * In the second step, the function interpolated between the two samples at
     * r1 and r1 + 1 based on the requested method.
     */
    template <SamplingMethod method> i16 interpolate(Cycle clock);
};
