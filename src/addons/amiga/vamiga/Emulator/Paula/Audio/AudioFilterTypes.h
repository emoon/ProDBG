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
#include "Reflection.h"

//
// Enumerations
//

enum_long(FILTER_TYPE)
{
    FILTER_NONE,
    FILTER_BUTTERWORTH,
    
    FILTER_COUNT
};
typedef FILTER_TYPE FilterType;

#ifdef __cplusplus
struct FilterTypeEnum : util::Reflection<FilterTypeEnum, FilterType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < FILTER_COUNT;
    }

    static const char *prefix() { return "FILTER"; }
    static const char *key(FilterType value)
    {
        switch (value) {
                
            case FILTER_NONE:         return "NONE";
            case FILTER_BUTTERWORTH:  return "BUTTERWORTH";
            case FILTER_COUNT:        return "???";
        }
        return "???";
    }
};
#endif
