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

enum_long(ROM_IDENTIFIER)
{
    ROM_MISSING,
    ROM_UNKNOWN,

    // Boot Roms (A1000)
    ROM_BOOT_A1000_8K,
    ROM_BOOT_A1000_64K,

    // Kickstart V1.x
    ROM_KICK11_31_034,
    ROM_KICK12_33_166,
    ROM_KICK12_33_180,
    ROM_KICK121_34_004,
    ROM_KICK13_34_005,
    ROM_KICK13_34_005_SK,

    // Kickstart V2.x
    ROM_KICK20_36_028,
    ROM_KICK202_36_207,
    ROM_KICK204_37_175,
    ROM_KICK205_37_299,
    ROM_KICK205_37_300,
    ROM_KICK205_37_350,

    // Kickstart V3.x
    ROM_KICK30_39_106,
    ROM_KICK31_40_063,

    // Hyperion
    ROM_HYP314_46_143,

    // Free Kickstart Rom replacements
    ROM_AROS_55696,
    ROM_AROS_55696_EXT,

    // Diagnostic cartridges
    ROM_DIAG11,
    ROM_DIAG12,
    ROM_DIAG121,
    ROM_LOGICA20,

    ROM_COUNT
};
typedef ROM_IDENTIFIER RomIdentifier;

#ifdef __cplusplus
struct RomIdentifierEnum : util::Reflection<RomIdentifierEnum, RomIdentifier> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < ROM_COUNT;
    }
    
    static const char *prefix() { return "ROM"; }
    static const char *key(RomIdentifier value)
    {
        switch (value) {
                
            case ROM_MISSING:           return "MISSING";
            case ROM_UNKNOWN:           return "UNKNOWN";

            case ROM_BOOT_A1000_8K:     return "BOOT_A1000_8K";
            case ROM_BOOT_A1000_64K:    return "BOOT_A1000_64K";

            case ROM_KICK11_31_034:     return "KICK11_31_034";
            case ROM_KICK12_33_166:     return "KICK12_33_166";
            case ROM_KICK12_33_180:     return "KICK12_33_180";
            case ROM_KICK121_34_004:    return "KICK121_34_004";
            case ROM_KICK13_34_005:     return "KICK13_34_005";
            case ROM_KICK13_34_005_SK:  return "KICK13_34_005_SK";

            case ROM_KICK20_36_028:     return "KICK20_36_028";
            case ROM_KICK202_36_207:    return "KICK202_36_207";
            case ROM_KICK204_37_175:    return "KICK204_37_175";
            case ROM_KICK205_37_299:    return "KICK205_37_299";
            case ROM_KICK205_37_300:    return "KICK205_37_300";
            case ROM_KICK205_37_350:    return "KICK205_37_350";

            case ROM_KICK30_39_106:     return "KICK30_39_106";
            case ROM_KICK31_40_063:     return "KICK31_40_063";

            case ROM_HYP314_46_143:     return "HYP314_46_143";

            case ROM_AROS_55696:        return "AROS_55696";
            case ROM_AROS_55696_EXT:    return "AROS_55696_EXT";

            case ROM_DIAG11:            return "DIAG11";
            case ROM_DIAG12:            return "DIAG12";
            case ROM_DIAG121:           return "DIAG121";
            case ROM_LOGICA20:          return "LOGICA20";
                
            case ROM_COUNT:             return "???";
        }
        return "???";
    }
};
#endif
