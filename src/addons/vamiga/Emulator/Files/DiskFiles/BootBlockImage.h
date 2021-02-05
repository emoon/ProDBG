// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FilePublicTypes.h"
#include <stddef.h>

typedef struct
{
    const char *name;
    u16 signature[14];
    const u8 *image;
    isize size;
    BootBlockType type;
}
BBRecord;

class BootBlockImage {

    // Image data
    u8 data[1024];
    
public:
    
    // Result of the data inspection
    BootBlockType type = BB_CUSTOM;
    const char *name = "Custom boot block";
    
    // Constructors
    BootBlockImage(const u8 *buffer);
    BootBlockImage(const char *name);
    BootBlockImage(long id);
    
    // Exports the image
    void write(u8 *buffer, isize first = 0, isize last = 0);
};
