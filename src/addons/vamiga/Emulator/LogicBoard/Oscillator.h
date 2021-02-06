// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _OSCILLATOR_H
#define _OSCILLATOR_H

#include "AmigaComponent.h"

#ifdef __MACH__
#include <mach/mach_time.h>
#endif

class Oscillator : public AmigaComponent {
    
#ifdef __MACH__

    // Information about the Mach system timer
    static mach_timebase_info_data_t tb;

    // Converts kernel time to nanoseconds
    static u64 abs_to_nanos(u64 abs) { return abs * tb.numer / tb.denom; }
    
    // Converts nanoseconds to kernel time
    static u64 nanos_to_abs(u64 nanos) { return nanos * tb.denom / tb.numer; }

#endif
    
    /* The heart of this class is method sychronize() which puts the thread to
     * sleep for a certain interval. In order to calculate the delay, the
     * function needs to know the values of the Amiga clock and the Kernel
     * clock at the time the synchronization timer was started. The values are
     * stores in the following two variables and recorded in restart().
     */
    
    // Agnus clock (Amiga master cycles)
    Cycle clockBase = 0;

    // Kernel clock (Nanoseconds)
    u64 timeBase = 0;

    
    //
    // Constructing and serializing
    //
    
public:
    
    Oscillator(Amiga& ref);

private:
    
    void _reset(bool hard) override;
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker & clockBase;
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Reading the system clock
    //
    
public:

    // Returns the current kernel time the nano seconds
    static u64 nanos();
    
    
    //
    // Managing emulation speed
    //
        
    // Restarts the synchronization timer
    void restart();

    // Puts the emulator thread to rest
    void synchronize();
    
private:
    
    // Puts the thread to rest until the target time has been reached
    void waitUntil(u64 deadline);
};

#endif
