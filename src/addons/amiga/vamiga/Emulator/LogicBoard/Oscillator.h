// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"
#include "Chrono.h"

#ifdef __MACH__
#include <mach/mach_time.h>
#endif

class Oscillator : public AmigaComponent {

public:
    
    // Clock rate of the master clock in MHz (PAL Amiga, 28.37516 MHz)
    static const double masterClockFrequency;

    // Clock rate of the Motorola 68000 CPU (7.09379 MHz)
    static const double cpuClockFrequency;

    // Clock rate of the DMA bus (3.546895 MHz)
    static const double dmaClockFrequency;

private:
    
    /* The heart of this class is method sychronize() which puts the thread to
     * sleep for a certain interval. In order to calculate the delay, the
     * function needs to know the values of the Amiga clock and the Kernel
     * clock at the time the synchronization timer was started. The values are
     * stores in the following two variables and recorded in restart().
     */
        
    // Agnus clock (Amiga master cycles)
    Cycle clockBase = 0;
    
    // Counts the number of calls to 'synchronize'
    isize syncCounter = 0;
    
    // Kernel clock
    util::Time timeBase;

    // The current CPU load (%)
    float cpuLoad = 0.0;
    
    // Clocks for measuring the CPU load
    util::Clock nonstopClock;
    util::Clock loadClock;

    
    //
    // Constructing
    //
    
public:
    
    Oscillator(Amiga& ref);

    const char *getDescription() const override;

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
        worker << clockBase;
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    
    
    //
    // Managing emulation speed
    //
        
public:
    
    // Restarts the synchronization timer
    void restart();

    // Puts the emulator thread to rest
    void synchronize();
};
