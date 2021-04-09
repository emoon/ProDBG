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

#ifdef __cplusplus
struct MemorySourceEnum : util::Reflection<MemorySourceEnum, MemorySource> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < MEM_COUNT;
    }

    static const char *prefix() { return "MEM"; }
    static const char *key(MemorySource value)
    {
        switch (value) {
                
            case MEM_NONE:           return "NONE";
            case MEM_CHIP:           return "CHIP";
            case MEM_CHIP_MIRROR:    return "CHIP_MIRROR";
            case MEM_SLOW:           return "SLOW";
            case MEM_SLOW_MIRROR:    return "SLOW_MIRROR";
            case MEM_FAST:           return "FAST";
            case MEM_CIA:            return "CIA";
            case MEM_CIA_MIRROR:     return "CIA_MIRROR";
            case MEM_RTC:            return "RTC";
            case MEM_CUSTOM:         return "CUSTOM";
            case MEM_CUSTOM_MIRROR:  return "CUSTOM_MIRROR";
            case MEM_AUTOCONF:       return "AUTOCONF";
            case MEM_ROM:            return "ROM";
            case MEM_ROM_MIRROR:     return "ROM_MIRROR";
            case MEM_WOM:            return "WOM";
            case MEM_EXT:            return "EXT";
            case MEM_COUNT:          return "???";
        }
        return "???";
    }
};
#endif

enum_long(ACCESSOR_TYPE)
{
    ACCESSOR_CPU,
    ACCESSOR_AGNUS,
    
    ACCESSOR_COUNT
};
typedef ACCESSOR_TYPE Accessor;

#ifdef __cplusplus
struct AccessorEnum : util::Reflection<AccessorEnum, Accessor> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < ACCESSOR_COUNT;
    }

    static const char *prefix() { return "ACCESSOR"; }
    static const char *key(Accessor value)
    {
        switch (value) {
                
            case ACCESSOR_CPU:    return "CPU";
            case ACCESSOR_AGNUS:  return "AGNUS";
            case ACCESSOR_COUNT:  return "???";
        }
        return "???";
    }
};
#endif

enum_long(BANK_MAP)
{
    BANK_MAP_A500,
    BANK_MAP_A1000,
    BANK_MAP_A2000A,
    BANK_MAP_A2000B,
    
    BANK_MAP_COUNT
};
typedef BANK_MAP BankMap;

#ifdef __cplusplus
struct BankMapEnum : util::Reflection<BankMapEnum, BankMap> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < BANK_MAP_COUNT;
    }

    static const char *prefix() { return "BANK_MAP"; }
    static const char *key(BankMap value)
    {
        switch (value) {
                
            case BANK_MAP_A500:    return "A500";
            case BANK_MAP_A1000:   return "A1000";
            case BANK_MAP_A2000A:  return "A2000A";
            case BANK_MAP_A2000B:  return "A2000B";
            case BANK_MAP_COUNT:   return "???";
        }
        return "???";
    }
};
#endif

enum_long(RAM_INIT_PATTERN)
{
    RAM_INIT_RANDOMIZED,
    RAM_INIT_ALL_ZEROES,
    RAM_INIT_ALL_ONES,
    
    RAM_INIT_COUNT
};
typedef RAM_INIT_PATTERN RamInitPattern;

#ifdef __cplusplus
struct RamInitPatternEnum : util::Reflection<RamInitPatternEnum, RamInitPattern> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < RAM_INIT_COUNT;
    }

    static const char *prefix() { return "RAM_INIT"; }
    static const char *key(RamInitPattern value)
    {
        switch (value) {
                
            case RAM_INIT_RANDOMIZED:  return "RANDOMIZED";
            case RAM_INIT_ALL_ZEROES:  return "ZEROES";
            case RAM_INIT_ALL_ONES:    return "ONES";
            case RAM_INIT_COUNT:       return "???";
        }
        return "???";
    }
};
#endif

enum_long(UNMAPPED_MEMORY)
{
    UNMAPPED_FLOATING,
    UNMAPPED_ALL_ZEROES,
    UNMAPPED_ALL_ONES,

    UNMAPPED_COUNT
};
typedef UNMAPPED_MEMORY UnmappedMemory;

#ifdef __cplusplus
struct UnmappedMemoryEnum : util::Reflection<UnmappedMemoryEnum, UnmappedMemory> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < UNMAPPED_COUNT;
    }

    static const char *prefix() { return "UNMAPPED"; }
    static const char *key(UnmappedMemory value)
    {
        switch (value) {
                
            case UNMAPPED_FLOATING:    return "FLOATING";
            case UNMAPPED_ALL_ZEROES:  return "ALL_ZEROES";
            case UNMAPPED_ALL_ONES:    return "ALL_ONES";
            case UNMAPPED_COUNT:       return "???";
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
    // RAM size in bytes
    i32 chipSize;
    i32 slowSize;
    i32 fastSize;

    // ROM size in bytes
    i32 romSize;
    i32 womSize;
    i32 extSize;

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
