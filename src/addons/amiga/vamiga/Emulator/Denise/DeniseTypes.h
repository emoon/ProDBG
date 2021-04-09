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

enum_long(DENISE_REV)
{
    DENISE_OCS,           // Revision 8362R8
    DENISE_OCS_BRDRBLNK,  // Revision 8362R8 + ECS Border blank feature
    DENISE_ECS,           // Revision 8373 (not supported yet)

    DENISE_COUNT
};
typedef DENISE_REV DeniseRevision;

#ifdef __cplusplus
struct DeniseRevisionEnum : util::Reflection<DeniseRevisionEnum, DeniseRevision> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < DENISE_COUNT;
    }

    static const char *prefix() { return "DENISE"; }
    static const char *key(DeniseRevision value)
    {
        switch (value) {
                
            case DENISE_OCS:          return "OCS";
            case DENISE_OCS_BRDRBLNK: return "OCS_BRDRBLNK";
            case DENISE_ECS:          return "ECS";
            case DENISE_COUNT:        return "???";
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
