// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _CPU_TYPES_H
#define _CPU_TYPES_H

#define CPUINFO_INSTR_COUNT 256

typedef struct
{
    u32 pc0;
    u32 d[8];
    u32 a[8];
    u32 usp;
    u32 ssp;
    u16 sr;
}
CPUInfo;

#endif
