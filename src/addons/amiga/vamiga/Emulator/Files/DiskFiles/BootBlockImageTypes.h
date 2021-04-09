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

enum_long(BB_TYPE)
{
    BB_STANDARD,
    BB_VIRUS,
    BB_CUSTOM,
    
    BB_COUNT
};
typedef BB_TYPE BootBlockType;

#ifdef __cplusplus
struct BootBlockTypeEnum : util::Reflection<BootBlockTypeEnum, BootBlockType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < BB_COUNT;
    }
    
    static const char *prefix() { return "BB"; }
    static const char *key(BootBlockType value)
    {
        switch (value) {
                
            case BB_STANDARD:  return "STANDARD";
            case BB_VIRUS:     return "VIRUS";
            case BB_CUSTOM:    return "CUSTOM";
            case BB_COUNT:     return "???";
        }
        return "???";
    }
};
#endif

enum_long(BB_ID)
{
    BB_NONE,
    BB_AMIGADOS_13,
    BB_AMIGADOS_20,
    BB_SCA,
    BB_BYTE_BANDIT
};
typedef BB_ID BootBlockId;

#ifdef __cplusplus
struct BootBlockIdEnum : util::Reflection<BootBlockIdEnum, BootBlockId> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < BB_COUNT;
    }
    
    static const char *prefix() { return "BB"; }
    static const char *key(BootBlockId value)
    {
        switch (value) {
                
            case BB_NONE:         return "NONE";
            case BB_AMIGADOS_13:  return "AMIGADOS_13";
            case BB_AMIGADOS_20:  return "AMIGADOS_20";
            case BB_SCA:          return "SCA";
            case BB_BYTE_BANDIT:  return "BYTE_BANDIT";
        }
        return "???";
    }
};
#endif
