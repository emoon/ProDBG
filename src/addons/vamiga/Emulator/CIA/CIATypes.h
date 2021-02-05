// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CIAPublicTypes.h"
#include "Reflection.h"

//
// Reflection APIs
//

struct CIARevisionEnum : Reflection<CIARevisionEnum, CIARevision> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < CIA_COUNT;
    }

    static const char *prefix() { return "CIA"; }
    static const char *key(CIARevision value)
    {
        switch (value) {
                
            case CIA_8520_DIP:   return "8520_DIP";
            case CIA_8520_PLCC:  return "8520_PLCC";
            case CIA_COUNT:      return "???";
        }
        return "???";
    }
};
