// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "BlitterPublicTypes.h"
#include "CopperPublicTypes.h"

enum_long(AGNUS_REVISION)
{
    AGNUS_OCS,              // Revision 8367
    AGNUS_ECS_1MB,          // Revision 8372
    AGNUS_ECS_2MB,          // Revision 8375
    
    AGNUS_COUNT
};
typedef AGNUS_REVISION AgnusRevision;


//
// Structures
//

typedef struct
{
    AgnusRevision revision;
    bool slowRamMirror;
}
AgnusConfig;

enum_i8(BUS_OWNER)
{
    BUS_NONE,
    BUS_CPU,
    BUS_REFRESH,
    BUS_DISK,
    BUS_AUDIO,
    BUS_BPL1,
    BUS_BPL2,
    BUS_BPL3,
    BUS_BPL4,
    BUS_BPL5,
    BUS_BPL6,
    BUS_SPRITE0,
    BUS_SPRITE1,
    BUS_SPRITE2,
    BUS_SPRITE3,
    BUS_SPRITE4,
    BUS_SPRITE5,
    BUS_SPRITE6,
    BUS_SPRITE7,
    BUS_COPPER,
    BUS_BLITTER,
    BUS_COUNT
};
typedef BUS_OWNER BusOwner;

static inline bool isBusOwner(long value)
{
    return (unsigned long)value < BUS_COUNT;
}

inline const char *BusOwnerName(BusOwner value)
{
    switch (value) {
            
        case BUS_NONE:     return "NONE";
        case BUS_CPU:      return "CPU";
        case BUS_REFRESH:  return "REFRESH";
        case BUS_DISK:     return "DISK";
        case BUS_AUDIO:    return "AUDIO";
        case BUS_BPL1:     return "BPL1";
        case BUS_BPL2:     return "BPL2";
        case BUS_BPL3:     return "BPL3";
        case BUS_BPL4:     return "BPL4";
        case BUS_BPL5:     return "BPL5";
        case BUS_BPL6:     return "BPL6";
        case BUS_SPRITE0:  return "SPRITE0";
        case BUS_SPRITE1:  return "SPRITE1";
        case BUS_SPRITE2:  return "SPRITE2";
        case BUS_SPRITE3:  return "SPRITE3";
        case BUS_SPRITE4:  return "SPRITE4";
        case BUS_SPRITE5:  return "SPRITE5";
        case BUS_SPRITE6:  return "SPRITE6";
        case BUS_SPRITE7:  return "SPRITE7";
        case BUS_COPPER:   return "COPPER";
        case BUS_BLITTER:  return "BLITTER";
        case BUS_COUNT:    return "???";
    }
    return "???";
}


//
// Structures
//

typedef struct
{
    i16 vpos;
    i16 hpos;

    u16 dmacon;
    u16 bplcon0;
    u8  bpu;
    u16 ddfstrt;
    u16 ddfstop;
    u16 diwstrt;
    u16 diwstop;

    u16 bpl1mod;
    u16 bpl2mod;
    u16 bltamod;
    u16 bltbmod;
    u16 bltcmod;
    u16 bltdmod;
    u16 bltcon0;
    
    u32 coppc;
    u32 dskpt;
    u32 bplpt[6];
    u32 audpt[4];
    u32 audlc[4];
    u32 bltpt[4];
    u32 sprpt[8];

    bool bls;
}
AgnusInfo;

typedef struct
{
    long usage[BUS_COUNT];
    
    double copperActivity;
    double blitterActivity;
    double diskActivity;
    double audioActivity;
    double spriteActivity;
    double bitplaneActivity;
}
AgnusStats;
