// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <sys/types.h>

//
// Basic types
//

typedef char               i8;
typedef short              i16;
typedef int                i32;
typedef long long          i64;
typedef ssize_t            isize;
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef size_t             usize;

static_assert(sizeof(i8) == 1,  "i8 size mismatch");
static_assert(sizeof(i16) == 2, "i16 size mismatch");
static_assert(sizeof(i32) == 4, "i32 size mismatch");
static_assert(sizeof(i64) == 8, "i64 size mismatch");
static_assert(sizeof(u8) == 1,  "u8 size mismatch");
static_assert(sizeof(u16) == 2, "u16 size mismatch");
static_assert(sizeof(u32) == 4, "u32 size mismatch");
static_assert(sizeof(u64) == 8, "u64 size mismatch");


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
// Pixels
//

typedef i16 Pixel;


//
// Floppy disks
//

typedef i16 Side;
typedef i16 Cylinder;
typedef i16 Track;
typedef i16 Sector;


//
// Syntactic sugar
//

/* The following keyword is used for documentary purposes only. It is used to
 * mark all methods that use the exception mechanism to signal error conditions
 * instead of returning error codes. It is used in favor of classic throw
 * lists, since the latter cause the compiler to embed unwanted runtime checks
 * in the code.
 */
#define throws

/* Signed alternative for the sizeof keyword */
#define isizeof(x) (isize)(sizeof(x))


//
// Enumerations
//

/* All enumeration types are declared via special 'enum_<type>' macros to make
 * them easily accessible in Swift. All macros have two definitions, one for
 * the Swift side and one for the C side. Please note that the type mapping for
 * enum_long differs on both sides. On the Swift side, enums of this type are
 * mapped to type 'long' to make them accessible via the Swift standard type
 * 'Int'. On the C side all enums are mapped to long long. This ensures the
 * same size for all enums, both on 32-bit and 64-bit architectures.
 */

#if defined(__VAMIGA_GUI__)

// Definition for Swift
#define enum_open(_name, _type) \
typedef enum __attribute__((enum_extensibility(open))) _name : _type _name; \
enum _name : _type

#define enum_long(_name) enum_open(_name, long)
#define enum_u32(_name) enum_open(_name, u32)
#define enum_i8(_name) enum_open(_name, i8)

#else

// Definition for C
#define enum_long(_name) enum _name : long long
#define enum_u32(_name) enum _name : u32
#define enum_i8(_name) enum _name : i8

#endif
