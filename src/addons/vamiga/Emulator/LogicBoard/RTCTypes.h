// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "RTCPublicTypes.h"
#include "Reflection.h"

struct RTCRevisionEnum : Reflection<RTCRevisionEnum, RTCRevision> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < RTC_COUNT;
    }

    static const char *prefix() { return "RTC"; }
    static const char *key(RTCRevision value)
    {
        switch (value) {
                
            case RTC_NONE:   return "NONR";
            case RTC_OKI:    return "OKI";
            case RTC_RICOH:  return "RICOH";
            case RTC_COUNT:  return "???";
        }
        return "???";
    }
};
