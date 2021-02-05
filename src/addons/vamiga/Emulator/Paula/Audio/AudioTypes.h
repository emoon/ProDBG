// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AudioPublicTypes.h"
#include "Reflection.h"

//
// Reflection APIs
//

struct SamplingMethodEnum : Reflection<SamplingMethodEnum, SamplingMethod> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < SMP_COUNT;
    }

    static const char *prefix() { return "SMP"; }
    static const char *key(SamplingMethod value)
    {
        switch (value) {
                
            case SMP_NONE:     return "NONE";
            case SMP_NEAREST:  return "NEAREST";
            case SMP_LINEAR:   return "LINEAR";
            case SMP_COUNT:    return "???";
        }
        return "???";
    }
};

struct FilterTypeEnum : Reflection<FilterTypeEnum, FilterType> {
    
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
