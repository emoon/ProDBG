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

//
// Enumerations
//

enum_u32(RunLoopControlFlag)
{
    RL_STOP               = 0b000000001,
    RL_INSPECT            = 0b000000010,
    RL_WARP_ON            = 0b000000100,
    RL_WARP_OFF           = 0b000001000,
    RL_BREAKPOINT_REACHED = 0b000010000,
    RL_WATCHPOINT_REACHED = 0b000100000,
    RL_AUTO_SNAPSHOT      = 0b001000000,
    RL_USER_SNAPSHOT      = 0b010000000
};

//
// Structures
//

typedef struct
{
    Cycle cpuClock;
    Cycle dmaClock;
    Cycle ciaAClock;
    Cycle ciaBClock;
    long frame;
    long vpos;
    long hpos;
}
AmigaInfo;
