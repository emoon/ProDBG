// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DISK_CONTROLLER_T_H
#define _DISK_CONTROLLER_T_H

//
// Enumerations
//

VAMIGA_ENUM(int, DriveState)
{
    DRIVE_DMA_OFF,     // Drive is idle

    DRIVE_DMA_WAIT,    // Drive is waiting for the sync word
    DRIVE_DMA_READ,    // Drive is reading

    DRIVE_DMA_WRITE,   // Drive is writing
    DRIVE_DMA_FLUSH,   // Drive is finishing up the write process
};

inline bool isDriveState(long value)
{
    return value >= DRIVE_DMA_OFF && value <= DRIVE_DMA_FLUSH;
}

inline const char *driveStateName(DriveState state)
{
    assert(isDriveState(state));

    switch (state) {
        case DRIVE_DMA_OFF:   return "DRIVE_DMA_OFF";
        case DRIVE_DMA_WAIT:  return "DRIVE_DMA_WAIT";
        case DRIVE_DMA_READ:  return "DRIVE_DMA_READ";
        case DRIVE_DMA_WRITE: return "DRIVE_DMA_WRITE";
        case DRIVE_DMA_FLUSH: return "DRIVE_DMA_FLUSH";
        default:              return "???";
    }
}

//
// Structures
//

typedef struct
{
    bool connected[4];
    
    /* Acceleration factor. This value equals the number of words that get
     * transfered into memory during a single disk DMA cycle. This value must
     * be 1 to emulate a real Amiga. If it set to, e.g., 2, the drive loads
     * twice as fast. A value of -1 indicates a turbo drive. In this case,
     * the exact value of the acceleration factor has no meaning.
     */
    long speed;

    bool lockDskSync;
    bool autoDskSync;
}
DiskControllerConfig;

inline bool isValidDriveSpeed(i16 speed)
{
    switch (speed) {
        case -1: case 1: case 2: case 4: case 8: return true;
    }
    return false;
}

typedef struct
{
    i8 selectedDrive;
    DriveState state;
    i32 fifo[6];
    u8 fifoCount;

    u16 dsklen;
    u16 dskbytr;
    u16 dsksync;
    u8 prb;
}
DiskControllerInfo;

#endif
