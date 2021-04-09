// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Oscillator.h"
#include "Agnus.h"
#include "Chrono.h"

const double Oscillator::masterClockFrequency = 28.37516;
const double Oscillator::cpuClockFrequency = masterClockFrequency / 4.0;
const double Oscillator::dmaClockFrequency = masterClockFrequency / 8.0;

Oscillator::Oscillator(Amiga& ref) : AmigaComponent(ref)
{

}
    
const char *
Oscillator::getDescription() const
{
#ifdef __MACH__
    return "Oscillator (Mac)";
#else
    return "Oscillator (Generic)";
#endif
}

void
Oscillator::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {
        
    }
}

void
Oscillator::restart()
{
    clockBase = agnus.clock;
    timeBase = util::Time::now();
}

void
Oscillator::synchronize()
{
    syncCounter++;
    
    // Only proceed if we are not running in warp mode
    if (warpMode) return;
    
    auto now          = util::Time::now();
    auto elapsedCyles = agnus.clock - clockBase;
    auto elapsedNanos = util::Time((i64)(elapsedCyles * 1000 / masterClockFrequency));
    auto targetTime   = timeBase + elapsedNanos;
    
    // Check if we're running too slow...
    if (now > targetTime) {
        
        // Check if we're completely out of sync...
        if ((now - targetTime).asMilliseconds() > 200) {
            
            // warn("The emulator is way too slow (%f).\n", (now - targetTime).asSeconds());
            restart();
            return;
        }
    }
    
    // Check if we're running too fast...
    if (now < targetTime) {
        
        // Check if we're completely out of sync...
        if ((targetTime - now).asMilliseconds() > 200) {
            
            warn("The emulator is way too fast (%f).\n", (targetTime - now).asSeconds());
            restart();
            return;
        }
        
        // See you soon...
        loadClock.stop();
        targetTime.sleepUntil();
        loadClock.go();
    }
    
    // Compute the CPU load once in a while
    if (syncCounter % 32 == 0) {
        
        auto used  = loadClock.getElapsedTime().asSeconds();
        auto total = nonstopClock.getElapsedTime().asSeconds();
        
        cpuLoad = used / total;
        
        loadClock.restart();
        nonstopClock.restart();
    }
}
