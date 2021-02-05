// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

/* Memory source identifiers. The identifiers are used in the mem source lookup
 * table to specify the source and target of a peek or poke operation,
 * respectively.
 */
enum_long(MEM_SOURCE)
{
    MEM_NONE,
    MEM_CHIP,
    MEM_CHIP_MIRROR,
    MEM_SLOW,
    MEM_SLOW_MIRROR,
    MEM_FAST,
    MEM_CIA,
    MEM_CIA_MIRROR,
    MEM_RTC,
    MEM_CUSTOM,
    MEM_CUSTOM_MIRROR,
    MEM_AUTOCONF,
    MEM_ROM,
    MEM_ROM_MIRROR,
    MEM_WOM,
    MEM_EXT,
    
    MEM_COUNT
};
typedef MEM_SOURCE MemorySource;

enum_long(ACCESSOR_TYPE)
{
    ACCESSOR_CPU,
    ACCESSOR_AGNUS,
    
    ACCESSOR_COUNT
};
typedef ACCESSOR_TYPE Accessor;

enum_long(BANK_MAP)
{
    BANK_MAP_A500,
    BANK_MAP_A1000,
    BANK_MAP_A2000A,
    BANK_MAP_A2000B,
    
    BANK_MAP_COUNT
};
typedef BANK_MAP BankMap;

enum_long(RAM_INIT_PATTERN)
{
    RAM_INIT_RANDOMIZED,
    RAM_INIT_ALL_ZEROES,
    RAM_INIT_ALL_ONES,
    
    RAM_INIT_COUNT
};
typedef RAM_INIT_PATTERN RamInitPattern;

enum_long(UNMAPPED_MEMORY)
{
    UNMAPPED_FLOATING,
    UNMAPPED_ALL_ZEROES,
    UNMAPPED_ALL_ONES,

    UNMAPPED_COUNT
};
typedef UNMAPPED_MEMORY UnmappedMemory;


//
// Structures
//

typedef struct
{
    // RAM size in bytes
    u32 chipSize;
    u32 slowSize;
    u32 fastSize;

    // ROM size in bytes
    u32 romSize;
    u32 womSize;
    u32 extSize;

    // Indicates if slow Ram accesses need a free bus
    bool slowRamDelay;
    
    // Memory layout
    BankMap bankMap;
    
    // Ram contents on startup
    RamInitPattern ramInitPattern;
    
    // Specifies how to deal with unmapped memory
    UnmappedMemory unmappingType;
    
    // First memory page where the extended ROM is blended it
    u32 extStart;
}
MemoryConfig;

typedef struct
{
    struct { long raw; double accumulated; } chipReads;
    struct { long raw; double accumulated; } chipWrites;
    struct { long raw; double accumulated; } slowReads;
    struct { long raw; double accumulated; } slowWrites;
    struct { long raw; double accumulated; } fastReads;
    struct { long raw; double accumulated; } fastWrites;
    struct { long raw; double accumulated; } kickReads;
    struct { long raw; double accumulated; } kickWrites;
}
MemoryStats;
