// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

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
    long value;
    long latch;
    long alarm;
}
CounterInfo;

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
