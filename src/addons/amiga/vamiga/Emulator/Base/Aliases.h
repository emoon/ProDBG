// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Macros.h"

// Cycles
typedef i64 Cycle;            // Master cycle units
typedef i64 CPUCycle;         // CPU cycle units
typedef i64 CIACycle;         // CIA cycle units
typedef i64 DMACycle;         // DMA cycle units

// Pixels
typedef i16 Pixel;

// Floppy disks
typedef i16 Side;
typedef i16 Cylinder;
typedef i16 Track;
typedef i16 Sector;


// File Systems
typedef u32 Block;


//
// Converting units
//

// Converts a certain unit to master cycles
#define USEC(delay)           ((delay) * 28)
#define MSEC(delay)           ((delay) * 28000)
#define SEC(delay)            ((delay) * 28000000)

#define CPU_CYCLES(cycles)    ((cycles) << 2)
#define CIA_CYCLES(cycles)    ((cycles) * 40)
#define DMA_CYCLES(cycles)    ((cycles) << 3)

// Converts master cycles to a certain unit
#define AS_USEC(delay)        ((delay) / 28)
#define AS_MSEC(delay)        ((delay) / 28000)
#define AS_SEC(delay)         ((delay) / 28000000)

#define AS_CPU_CYCLES(cycles) ((cycles) >> 2)
#define AS_CIA_CYCLES(cycles) ((cycles) / 40)
#define AS_DMA_CYCLES(cycles) ((cycles) >> 3)

#define IS_CPU_CYCLE(cycles)  ((cycles) & 3 == 0)
#define IS_CIA_CYCLE(cycles)  ((cycles) % 40 == 0)
#define IS_DMA_CYCLE(cycles)  ((cycles) & 7 == 0)

// Converts kilo and mega bytes to bytes
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)

// Converts kilo and mega Hertz to Hertz
#define KHz(x) ((x) * 1000)
#define MHz(x) ((x) * 1000000)
