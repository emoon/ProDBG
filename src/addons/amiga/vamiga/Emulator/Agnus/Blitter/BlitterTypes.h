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

//
// Structures
//

typedef struct
{
    int accuracy;
}
BlitterConfig;

typedef struct
{
    // bool active;
    u16 bltcon0;
    u16 bltcon1;
    u16 ash;
    u16 bsh;
    u16 minterm;
    u32 bltapt;
    u32 bltbpt;
    u32 bltcpt;
    u32 bltdpt;
    u16 bltafwm;
    u16 bltalwm;
    i16 bltamod;
    i16 bltbmod;
    i16 bltcmod;
    i16 bltdmod;
    u16 aold;
    u16 bold;
    u16 anew;
    u16 bnew;
    u16 ahold;
    u16 bhold;
    u16 chold;
    u16 dhold;
    bool bbusy;
    bool bzero;
    bool firstWord;
    bool lastWord;
    bool fci;
    bool fco;
    bool fillEnable;
    bool storeToDest;
}
BlitterInfo;
