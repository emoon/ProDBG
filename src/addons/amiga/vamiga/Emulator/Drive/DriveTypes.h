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
#include "BootBlockImageTypes.h"
#include "FSTypes.h"
#include "Reflection.h"

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

#ifdef __cplusplus
struct DriveTypeEnum : util::Reflection<DriveTypeEnum, DriveType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <  DRIVE_COUNT;
    }
    
    static const char *prefix() { return "DRIVE"; }
    static const char *key(DriveType value)
    {
        switch (value) {
                
            case DRIVE_DD_35:   return "DD_35";
            case DRIVE_HD_35:   return "HD_35";
            case DRIVE_DD_525:  return "DD_525";
            case DRIVE_COUNT:   return "???";
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
    
    // Noise settings
    i16 pan;
    u8 stepVolume;
    u8 pollVolume;
    u8 insertVolume;
    u8 ejectVolume;
    
    // Blank disk defaults
    FSVolumeType defaultFileSystem;
    BootBlockId defaultBootBlock;
}
DriveConfig;

typedef struct
{
    DriveHead head;
    bool hasDisk;
    bool motor;
}
DriveInfo;
