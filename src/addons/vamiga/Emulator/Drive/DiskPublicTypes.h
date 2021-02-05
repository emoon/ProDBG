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

enum_long(DISK_DIAMETER)
{
    INCH_35,
    INCH_525,
    
    INCH_COUNT
};
typedef DISK_DIAMETER DiskDiameter;

enum_long(DISK_DENSITY)
{
    DISK_SD,
    DISK_DD,
    DISK_HD,
    
    DISK_COUNT
};
typedef DISK_DENSITY DiskDensity;
