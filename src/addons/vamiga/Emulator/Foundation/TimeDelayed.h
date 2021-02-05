// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"

template <class T, int delay> class TimeDelayed : AmigaObject {
    
    /* Value pipeline (history buffer)
     *
     *    pipeline[0] : Value that was written at time timeStamp
     *    pipeline[n] : Value that was written at time timeStamp - n
     */
    T pipeline[delay + 1];
        
    // Remembers the time of the most recent call to write()
    i64 timeStamp = 0;
        
    // Pointer to reference clock
    i64 *clock = nullptr;

    
    //
    // Initializing
    //
    
public:
    
    TimeDelayed(u64 *clock) {
        
        timeStamp = 0;
        this->clock = (i64 *)clock;
        clear();
    }
    
    TimeDelayed() : TimeDelayed(nullptr) { };
          
    const char *getDescription() const { return "TimeDelayed"; }
    
    // Sets the reference clock
    void setClock(i64 *clock) { this->clock = clock; }

    // Overwrites all pipeline entries with a reset value
    void reset(T value) {
        for (isize i = 0; i <= delay; i++) pipeline[i] = value;
        timeStamp = 0;
    }
    
    // Zeroes out all pipeline entries
    void clear() { reset((T)0); }
    
    
    //
    // Analyzing
    //
    
public:
    
    void dump() {
        msg("TimeStamp: %lld clock: %lld\n", timeStamp, AS_DMA_CYCLES(*clock));
        for (isize i = delay; i >= 0; i--) msg("[%d] ", readWithDelay(i));
        msg("\n");
    }

    
    //
    // Serializing
    //
    
public:
        
    template <class W>
    void applyToItems(W& worker)
    {
        worker & pipeline & timeStamp;
    }
    

    //
    // Accessing
    //
    
    // Write a value into the pipeline
    void write(T value) {
        
        i64 dmaClock = AS_DMA_CYCLES(*clock);
        // Shift pipeline
        i64 diff = dmaClock - timeStamp;
        for (isize i = delay; i >= 0; i--) {
            assert(i - diff < 0 || i - diff <= delay);
            pipeline[i] = (i - diff < 0) ? pipeline[0] : pipeline[i - diff];
        }
        
        // Assign new value
        timeStamp = dmaClock;
        pipeline[0] = value;
    }
    
    // Reads the most recent pipeline element
    T current() { return pipeline[0]; }
    
    // Reads a value from the pipeline with the standard delay
    T delayed() { return pipeline[MAX(0, timeStamp - AS_DMA_CYCLES(*clock) + delay)]; }
    
    // Reads a value from the pipeline with a custom delay
    T readWithDelay(u8 customDelay) {
        assert(customDelay <= delay);
        return pipeline[MAX(0, timeStamp - AS_DMA_CYCLES(*clock) + customDelay)];
    }
};
