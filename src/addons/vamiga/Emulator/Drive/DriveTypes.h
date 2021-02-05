// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DrivePublicTypes.h"
#include "Reflection.h"

struct DriveTypeEnum : Reflection<DriveTypeEnum, DriveType> {
    
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
