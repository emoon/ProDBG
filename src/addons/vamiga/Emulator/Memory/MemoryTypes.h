// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MemoryPublicTypes.h"
#include "Reflection.h"

//
// Reflection APIs
//

struct MemorySourceEnum : Reflection<MemorySourceEnum, MemorySource> {
    
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
            case MEM_CHIP_MIRROR:    return "CHIP_MIRROR:";
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

struct AccessorEnum : Reflection<AccessorEnum, Accessor> {
    
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

struct BankMapEnum : Reflection<BankMapEnum, BankMap> {
    
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

struct RamInitPatternEnum : Reflection<RamInitPatternEnum, RamInitPattern> {
    
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

struct UnmappedMemoryEnum : Reflection<UnmappedMemoryEnum, UnmappedMemory> {
    
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
