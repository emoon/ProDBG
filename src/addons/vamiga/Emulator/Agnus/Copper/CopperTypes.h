// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _COPPER_TYPES_H

typedef struct
{
    u8   copList;
    bool active;
    bool cdang;
    u32  coppc;
    u32  cop1lc;
    u32  cop2lc;
    u16  cop1ins;
    u16  cop2ins;
    i16  length1;
    i16  length2;
}
CopperInfo;

#endif
