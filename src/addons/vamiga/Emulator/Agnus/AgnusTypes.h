// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _AGNUS_TYPES_H
#define _AGNUS_TYPES_H

#include "BlitterTypes.h"
#include "CopperTypes.h"

VAMIGA_ENUM(long, AgnusRevision)
{
    AGNUS_OCS,              // Revision 8367
    AGNUS_ECS_1MB,          // Revision 8372
    AGNUS_ECS_2MB,          // Revision 8375
    AGNUS_CNT
};

inline bool isAgnusRevision(long value)
{
    return value >= 0 && value < AGNUS_CNT;
}

inline const char *sAgnusRevision(AgnusRevision value)
{
    switch (value) {
        case AGNUS_OCS:     return "AGNUS_OCS";
        case AGNUS_ECS_1MB: return "AGNUS_ECS_1MB";
        case AGNUS_ECS_2MB: return "AGNUS_ECS_2MB";
        default:            return "???";
    }
}

typedef struct
{
    AgnusRevision revision;
    bool slowRamMirror;
}
AgnusConfig;

VAMIGA_ENUM(i8, BusOwner)
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

static inline bool isBusOwner(long value)
{
    return value >= 0 && value < BUS_COUNT;
}

VAMIGA_ENUM(long, DDFState)
{
    DDF_OFF,
    DDF_READY,
    DDF_ON
};

VAMIGA_ENUM(long, SprDMAState)
{
    SPR_DMA_IDLE,
    SPR_DMA_ACTIVE
};


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

#endif
