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

//
// Enumerations
//

enum_long(DRIVE_TYPE)
{
    DRIVE_DD_35,
    DRIVE_HD_35,
    DRIVE_DD_525,
    
    DRIVE_COUNT
};
typedef DRIVE_TYPE DriveType;


//
// Structures
//

typedef struct
{
     u8 side;
     u8 cylinder;
     u16 offset;
 }
DriveHead;

typedef struct
{
    DriveType type;
    
    // Indicates whether mechanical delays should be emulated
    bool mechanicalDelays;

    /* Mechanical delays. The start and stop delays specify the number of
     * cycles that pass between switching the drive motor on or off until the
     * drive motor runs at full speed or came to rest, respectively. The step
     * delay specifies the number of cycle needed by the drive head to move to
     * another cylinder. During this time, the FIFO is filled with garbage data.
     */
    Cycle startDelay;
    Cycle stopDelay;
    Cycle stepDelay;
}
DriveConfig;

typedef struct
{
    DriveHead head;
    bool hasDisk;
    bool motor;
}
DriveInfo;
