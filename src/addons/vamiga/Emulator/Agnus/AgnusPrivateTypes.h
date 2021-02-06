// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AGNUS_PRIVATE_TYPES_H
#define _AGNUS_PRIVATE_TYPES_H

enum RegChangeID : i32
{
    SET_NONE = 0,
    
    SET_BLTSIZE,
    SET_BLTSIZV,
    SET_BLTCON0,
    SET_BLTCON0L,
    SET_BLTCON1,
    
    SET_INTREQ,
    SET_INTENA,
    
    SET_BPLCON0_AGNUS,
    SET_BPLCON0_DENISE,
    SET_BPLCON1_AGNUS,
    SET_BPLCON1_DENISE,
    SET_BPLCON2,
    SET_BPLCON3,
    SET_DMACON,
    
    SET_DIWSTRT,
    SET_DIWSTOP,
    SET_DDFSTRT,
    SET_DDFSTOP,
    
    SET_BPL1MOD,
    SET_BPL2MOD,
    SET_BPL1PTH,
    SET_BPL2PTH,
    SET_BPL3PTH,
    SET_BPL4PTH,
    SET_BPL5PTH,
    SET_BPL6PTH,
    SET_BPL1PTL,
    SET_BPL2PTL,
    SET_BPL3PTL,
    SET_BPL4PTL,
    SET_BPL5PTL,
    SET_BPL6PTL,

    SET_SPR0DATA,
    SET_SPR1DATA,
    SET_SPR2DATA,
    SET_SPR3DATA,
    SET_SPR4DATA,
    SET_SPR5DATA,
    SET_SPR6DATA,
    SET_SPR7DATA,

    SET_SPR0DATB,
    SET_SPR1DATB,
    SET_SPR2DATB,
    SET_SPR3DATB,
    SET_SPR4DATB,
    SET_SPR5DATB,
    SET_SPR6DATB,
    SET_SPR7DATB,

    SET_SPR0POS,
    SET_SPR1POS,
    SET_SPR2POS,
    SET_SPR3POS,
    SET_SPR4POS,
    SET_SPR5POS,
    SET_SPR6POS,
    SET_SPR7POS,

    SET_SPR0CTL,
    SET_SPR1CTL,
    SET_SPR2CTL,
    SET_SPR3CTL,
    SET_SPR4CTL,
    SET_SPR5CTL,
    SET_SPR6CTL,
    SET_SPR7CTL,

    SET_SPR0PTH,
    SET_SPR1PTH,
    SET_SPR2PTH,
    SET_SPR3PTH,
    SET_SPR4PTH,
    SET_SPR5PTH,
    SET_SPR6PTH,
    SET_SPR7PTH,

    SET_SPR0PTL,
    SET_SPR1PTL,
    SET_SPR2PTL,
    SET_SPR3PTL,
    SET_SPR4PTL,
    SET_SPR5PTL,
    SET_SPR6PTL,
    SET_SPR7PTL,

    REG_COUNT
};

static inline bool isRegChangeID(long value)
{
    return value >= 0 && value < REG_COUNT;
}

#endif
