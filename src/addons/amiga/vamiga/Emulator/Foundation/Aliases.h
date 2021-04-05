// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ALIASES_H
#define _ALIASES_H

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

//
// Basic types
//

/* Note: We don't map i64 to int64_t, because this type is mapped to long long
 * on macOS, but mapped to long on Linux. This would require different format
 * strings when such a value is printed via printf().
 */
typedef int8_t             i8;
typedef int16_t            i16;
typedef int32_t            i32;
typedef long long          i64;
typedef uint8_t            u8;
typedef uint16_t           u16;
typedef uint32_t           u32;
typedef unsigned long long u64;


//
// Syntactic sugar
//

#define fallthrough


//
// Cycles
//

typedef i64 Cycle;            // Master cycle units
typedef i64 CPUCycle;         // CPU cycle units
typedef i64 CIACycle;         // CIA cycle units
typedef i64 DMACycle;         // DMA cycle units

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


//
// Positions
//

typedef i16 PixelPos;


//
// Floppy disks
//

typedef i16 Side;
typedef i16 Cylinder;
typedef i16 Track;
typedef i16 Sector;

/* All enumeration types are declared via VAMIGA_ENUM. We don't use the
 * standard C enum style to make enumerations easily accessible in Swift.
 */

// Definition for Swift
#ifdef VA_ENUM
#define VAMIGA_ENUM(_type, _name) \
typedef VA_ENUM(_type, _name)

// Definition for clang
#elif defined(__clang__)
#define VAMIGA_ENUM(_type, _name) \
typedef enum __attribute__((enum_extensibility(open))) _name : _type _name; \
enum _name : _type

// Definition for gcc
#else
#define VAMIGA_ENUM(_type, _name) \
enum _name : _type
#endif

#endif
