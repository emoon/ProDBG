// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DRIVE_TYPES_H
#define _DRIVE_TYPES_H

#include "Aliases.h"

//
// Enumerations
//

VAMIGA_ENUM(long, DriveType)
{
    DRIVE_35_DD,
    DRIVE_35_HD,
    DRIVE_525_DD
};

inline bool isDriveType(long value)
{
    return value >= DRIVE_35_DD && value <= DRIVE_525_DD;
}

inline const char *driveTypeName(DriveType type)
{
    assert(isDriveType(type));
    
    switch (type) {
        case DRIVE_35_DD:    return "Drive 3.5\" DD";
        case DRIVE_35_HD:    return "Drive 3.5\" HD";
        case DRIVE_525_DD:   return "Drive 5.25\" DD";
        default:             return "???";
    }
}

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

#endif
