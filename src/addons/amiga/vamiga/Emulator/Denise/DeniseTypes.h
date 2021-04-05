// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _DENISE_TYPES_H
#define _DENISE_TYPES_H

#include "Aliases.h"

//
// Enumerations
//

VAMIGA_ENUM(long, DeniseRevision)
{
    DENISE_OCS,          // Revision 8362R8
    // DENISE_OCS_BRDRBLNK, // Revision 8362R8 + Border Blank feature
    DENISE_ECS,          // Revision 8373 (not supported yet)
    DENISE_COUNT
    
};

inline bool isDeniseRevision(long value)
{
    return value >= 0 && value < DENISE_COUNT;
}

inline const char *sDeniseRevision(DeniseRevision value)
{
    switch (value) {
        case DENISE_OCS:  return "DENISE_OCS";
        case DENISE_ECS:  return "DENISE_ECS";
        default:          return "???";
    }
}

VAMIGA_ENUM(long, Palette)
{
    PALETTE_COLOR = 0,
    PALETTE_BLACK_WHITE,
    PALETTE_PAPER_WHITE,
    PALETTE_GREEN,
    PALETTE_AMBER,
    PALETTE_SEPIA,
    PALETTE_COUNT
};

inline bool isPalette(long value)
{
    return value >= PALETTE_COLOR && value < PALETTE_COUNT;
}


//
// Structures
//

typedef struct
{
    u32 *data;
    bool longFrame;
}
ScreenBuffer;

typedef struct
{
    // Number of lines the sprite was armed
    u16 height;

    // Extracted information from SPRxPOS and SPRxCTL
    i16 hstrt;
    i16 vstrt;
    i16 vstop;
    bool attach;
    
    // Upper 16 color register (recorded where the observed sprite starts)
    u16 colors[16];
}
SpriteInfo;

typedef struct
{
    // Emulated chip model
    DeniseRevision revision;

    // Borderblank feature (was introduced with ECS chipset)
    bool borderblank;
    
    // Hides certain sprites
    u8 hiddenSprites;

    // Hides certain graphics layers
    u16 hiddenLayers;
    
    // Alpha channel value for hidden layers
    u8 hiddenLayerAlpha;
    
    // Checks for sprite-sprite collisions
    bool clxSprSpr;

    // Checks for sprite-playfield collisions
    bool clxSprPlf;

    // Checks for playfield-playfield collisions
    bool clxPlfPlf;
}
DeniseConfig;

typedef struct
{
    u16 bplcon0;
    u16 bplcon1;
    u16 bplcon2;
    i16 bpu;
    u16 bpldat[6];

    u16 diwstrt;
    u16 diwstop;
    i16 diwHstrt;
    i16 diwHstop;
    i16 diwVstrt;
    i16 diwVstop;

    u16 joydat[2];
    u16 clxdat;

    u16 colorReg[32];
    u32 color[32];
}
DeniseInfo;

#endif
