// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_TYPES_H
#define _FS_TYPES_H

#include "Aliases.h"

VAMIGA_ENUM(long, FSVolumeType)
{
    FS_NONE,
    FS_OFS,
    FS_FFS
};

inline bool isFSType(FSVolumeType value)
{
    return value >= FS_NONE && value <= FS_FFS;
}

inline const char *sFSType(FSVolumeType value)
{
    switch (value) {
        case FS_NONE:  return "None";
        case FS_OFS:   return "OFS";
        case FS_FFS:   return "FFS";
        default:       return "???";
    }
}

VAMIGA_ENUM(long, FSBlockType)
{
    FS_UNKNOWN_BLOCK,
    FS_EMPTY_BLOCK,
    FS_BOOT_BLOCK,
    FS_ROOT_BLOCK,
    FS_BITMAP_BLOCK,
    FS_USERDIR_BLOCK,
    FS_FILEHEADER_BLOCK,
    FS_FILELIST_BLOCK,
    FS_DATA_BLOCK
};

inline bool
isFSBlockType(long value)
{
    return value >= FS_UNKNOWN_BLOCK && value <= FS_DATA_BLOCK;
}

inline const char *
fsBlockTypeName(FSBlockType type)
{
    assert(isFSBlockType(type));

    switch (type) {
        case FS_EMPTY_BLOCK:      return "FS_EMPTY_BLOCK";
        case FS_BOOT_BLOCK:       return "FS_BOOT_BLOCK";
        case FS_ROOT_BLOCK:       return "FS_ROOT_BLOCK";
        case FS_BITMAP_BLOCK:     return "FS_BITMAP_BLOCK";
        case FS_USERDIR_BLOCK:    return "FS_USERDIR_BLOCK";
        case FS_FILEHEADER_BLOCK: return "FS_FILEHEADER_BLOCK";
        case FS_FILELIST_BLOCK:   return "FS_FILELIST_BLOCK";
        case FS_DATA_BLOCK:       return "FS_DATA_BLOCK";
        default:                  return "???";
    }
}

#endif
