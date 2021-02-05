// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DmaDebuggerPublicTypes.h"
#include "Reflection.h"

//
// Reflection APIs
//

struct DmaDisplayModeEnum : Reflection<DmaDisplayModeEnum, DmaDisplayMode> {
    
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
