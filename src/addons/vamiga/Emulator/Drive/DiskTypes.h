// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskPublicTypes.h"
#include "Reflection.h"

struct DiskDiameterEnum : Reflection<DiskDiameterEnum, DiskDiameter> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < INCH_COUNT;
    }
    
    static const char *prefix() { return ""; }
    static const char *key(DiskDiameter value)
    {
        switch (value) {
                
            case INCH_35:     return "INCH_35";
            case INCH_525:    return "INCH_525";
            case INCH_COUNT:  return "???";
        }
        return "???";
    }
};

struct DiskDensityEnum : Reflection<DiskDensityEnum, DiskDensity> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < DISK_COUNT;
    }
    
    static const char *prefix() { return "DISK"; }
    static const char *key(DiskDensity value)
    {
        switch (value) {
                
            case DISK_SD:     return "SD";
            case DISK_DD:     return "DD";
            case DISK_HD:     return "HD";
            case DISK_COUNT:  return "???";
        }
        return "???";
    }
};
