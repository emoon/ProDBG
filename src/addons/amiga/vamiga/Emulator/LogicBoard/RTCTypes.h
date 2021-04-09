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

enum_long(RTC_REVISION)
{
    RTC_NONE,
    RTC_OKI,
    RTC_RICOH,
    
    RTC_COUNT
};
typedef RTC_REVISION RTCRevision;

#ifdef __cplusplus
struct RTCRevisionEnum : util::Reflection<RTCRevisionEnum, RTCRevision> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < RTC_COUNT;
    }

    static const char *prefix() { return "RTC"; }
    static const char *key(RTCRevision value)
    {
        switch (value) {
                
            case RTC_NONE:   return "NONE";
            case RTC_OKI:    return "OKI";
            case RTC_RICOH:  return "RICOH";
            case RTC_COUNT:  return "???";
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
    RTCRevision model;
}
RTCConfig;
