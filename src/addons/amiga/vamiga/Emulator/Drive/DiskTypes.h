// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DISK_TYPES_H
#define _DISK_TYPES_H

#include "Aliases.h"

//
// Enumerations
//

VAMIGA_ENUM(long, DiskType)
{
    DISK_35,
    DISK_525
};

inline bool isDiskType(DiskType value)
{
    return value >= 0 && value <= DISK_525;
}

/*
inline bool isAmigaDiskType(DiskType value)
{
    return value == DISK_35_DD || value == DISK_35_HD || value == DISK_525_DD;
}
*/

inline const char *sDiskType(DiskType value)
{
    assert(isDiskType(value));
    
    switch (value) {
        case DISK_35:   return "3.5\"";
        case DISK_525:  return "5.25\"";
        default:        return "???";
    }
}

VAMIGA_ENUM(long, DiskDensity)
{
    DISK_SD,
    DISK_DD,
    DISK_HD
};

inline bool isDiskDensity(DiskDensity value)
{
    return value >= 0 && value <= DISK_HD;
}

inline const char *sDiskDensity(DiskDensity value)
{
    switch (value) {
        case DISK_SD:  return "SD";
        case DISK_DD:  return "DD";
        case DISK_HD:  return "HD";
        default:       return "???";
    }
}

/*
VAMIGA_ENUM(long, EmptyDiskFormat)
{
    FS_EMPTY,
    FS_EMPTY_OFS,
    FS_EMPTY_OFS_BOOTABLE,
    FS_EMPTY_FFS,
    FS_EMPTY_FFS_BOOTABLE
};

inline bool isEmptyDiskFormat(EmptyDiskFormat type)
{
    return type >= FS_EMPTY && type <= FS_EMPTY_FFS_BOOTABLE;
}

inline const char *sEmptyDiskFormat(EmptyDiskFormat type)
{
    switch (type) {
        case FS_EMPTY:              return "None";
        case FS_EMPTY_OFS:          return "OFS";
        case FS_EMPTY_OFS_BOOTABLE: return "OFS (bootable)";
        case FS_EMPTY_FFS:          return "FFS";
        case FS_EMPTY_FFS_BOOTABLE: return "FFS (bootable)";
        default:                    return "???";
    }
}
*/

#endif
