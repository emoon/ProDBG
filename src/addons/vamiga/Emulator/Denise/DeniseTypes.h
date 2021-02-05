// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DenisePublicTypes.h"
#include "Reflection.h"

//
// Reflection APIs
//

struct DeniseRevisionEnum : Reflection<DeniseRevisionEnum, DeniseRevision> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < DENISE_COUNT;
    }

    static const char *prefix() { return "DENISE"; }
    static const char *key(DeniseRevision value)
    {
        switch (value) {
                
            case DENISE_OCS:    return "OCS";
            case DENISE_ECS:    return "ECS";
            case DENISE_COUNT:  return "???";
        }
        return "???";
    }
};

struct PaletteEnum : Reflection<PaletteEnum, Palette> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < PALETTE_COUNT;
    }

    static const char *prefix() { return "PALETTE"; }
    static const char *key(Palette value)
    {
        switch (value) {
                
            case PALETTE_COLOR:        return "COLOR";
            case PALETTE_BLACK_WHITE:  return "BLACK_WHITE";
            case PALETTE_PAPER_WHITE:  return "PAPER_WHITE";
            case PALETTE_GREEN:        return "GREEN";
            case PALETTE_AMBER:        return "AMBER";
            case PALETTE_SEPIA:        return "SEPIA";
            case PALETTE_COUNT:        return "???";
        }
        return "???";
    }
};
