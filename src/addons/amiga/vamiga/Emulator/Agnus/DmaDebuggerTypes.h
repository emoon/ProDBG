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

enum_long(DMA_DISPLAY_MODE)
{
    DMA_DISPLAY_MODE_FG_LAYER,
    DMA_DISPLAY_MODE_BG_LAYER,
    DMA_DISPLAY_MODE_ODD_EVEN_LAYERS,
    
    DMA_DISPLAY_MODE_COUNT
};
typedef DMA_DISPLAY_MODE DmaDisplayMode;

#ifdef __cplusplus
struct DmaDisplayModeEnum : util::Reflection<DmaDisplayModeEnum, DmaDisplayMode> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < DMA_DISPLAY_MODE_COUNT;
    }

    static const char *prefix() { return "DMA_DISPLAY_MODE"; }
    static const char *key(DmaDisplayMode value)
    {
        switch (value) {
                
            case DMA_DISPLAY_MODE_FG_LAYER:        return "FG_LAYER";
            case DMA_DISPLAY_MODE_BG_LAYER:        return "BG_LAYER";
            case DMA_DISPLAY_MODE_ODD_EVEN_LAYERS: return "ODD_EVEN_LAYERS";
            case DMA_DISPLAY_MODE_COUNT:           return "???";
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
    bool enabled;
    
    bool visualizeCopper;
    bool visualizeBlitter;
    bool visualizeDisk;
    bool visualizeAudio;
    bool visualizeSprites;
    bool visualizeBitplanes;
    bool visualizeCpu;
    bool visualizeRefresh;

    DmaDisplayMode displayMode;
    double opacity;
    
    double copperColor[3];
    double blitterColor[3];
    double diskColor[3];
    double audioColor[3];
    double spriteColor[3];
    double bitplaneColor[3];
    double cpuColor[3];
    double refreshColor[3];
}
DMADebuggerInfo;
