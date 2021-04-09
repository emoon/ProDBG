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

enum_long(INT_SOURCE)
{
    INT_TBE,
    INT_DSKBLK,
    INT_SOFT,
    INT_PORTS,
    INT_COPER,
    INT_VERTB,
    INT_BLIT,
    INT_AUD0,
    INT_AUD1,
    INT_AUD2,
    INT_AUD3,
    INT_RBF,
    INT_DSKSYN,
    INT_EXTER,
    INT_COUNT
};
typedef INT_SOURCE IrqSource;

#ifdef __cplusplus
struct IrqSourceEnum : util::Reflection<IrqSourceEnum, IrqSource> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < INT_COUNT;
    }

    static const char *prefix() { return "INT"; }
    static const char *key(IrqSource value)
    {
        switch (value) {
                
            case INT_TBE:     return "TBE";
            case INT_DSKBLK:  return "DSKBLK";
            case INT_SOFT:    return "SOFT";
            case INT_PORTS:   return "PORTS";
            case INT_COPER:   return "COPER";
            case INT_VERTB:   return "VERTB";
            case INT_BLIT:    return "BLIT";
            case INT_AUD0:    return "AUD0";
            case INT_AUD1:    return "AUD1";
            case INT_AUD2:    return "AUD2";
            case INT_AUD3:    return "AUD3";
            case INT_RBF:     return "RBF";
            case INT_DSKSYN:  return "DSKSYN";
            case INT_EXTER:   return "EXTER";
            case INT_COUNT:   return "???";
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
    u16 intreq;
    u16 intena;
    u16 adkcon;
}
PaulaInfo;
