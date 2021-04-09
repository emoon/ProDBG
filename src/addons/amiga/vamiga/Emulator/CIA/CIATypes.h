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
#include "Reflection.h"
#include "TODTypes.h"

/* Emulated CIA model
 *
 *   CIA_8520_DIP  mimics option "[ ] 391078-01" in UAE (default)
 *   CIA_8520_PLCC mimics option "[X] 391078-01" in UAE (A600)
 */
enum_long(CIARevision)
{
    CIA_8520_DIP,
    CIA_8520_PLCC,
    
    CIA_COUNT
};

#ifdef __cplusplus
struct CIARevisionEnum : util::Reflection<CIARevisionEnum, CIARevision> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < CIA_COUNT;
    }

    static const char *prefix() { return "CIA"; }
    static const char *key(CIARevision value)
    {
        switch (value) {
                
            case CIA_8520_DIP:   return "8520_DIP";
            case CIA_8520_PLCC:  return "8520_PLCC";
            case CIA_COUNT:      return "???";
        }
        return "???";
    }
};
#endif

//
// Structures
//

typedef struct
{
    CIARevision revision;
    bool todBug;
    bool eClockSyncing;
}
CIAConfig;

typedef struct
{
    struct {
        u8 port;
        u8 reg;
        u8 dir;
    } portA;

    struct {
        u8 port;
        u8 reg;
        u8 dir;
    } portB;

    struct {
        u16 count;
        u16 latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerA;

    struct {
        u16 count;
        u16 latch;
        bool running;
        bool toggle;
        bool pbout;
        bool oneShot;
    } timerB;

    u8 sdr;
    u8 ssr;
    u8 icr;
    u8 imr;
    bool intLine;
    
    CounterInfo cnt;
    bool cntIntEnable;
    
    Cycle idleSince;
    Cycle idleTotal;
    double idlePercentage;
}
CIAInfo;
