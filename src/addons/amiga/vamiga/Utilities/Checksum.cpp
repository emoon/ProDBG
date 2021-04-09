// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Checksum.h"
#include "Macros.h"

namespace util {

u32
NO_SANITIZE("unsigned-integer-overflow")
fnv_1a_it32(u32 prv, u32 val)
{
    return (prv ^ val) * 0x1000193;
}
 
u64
NO_SANITIZE("unsigned-integer-overflow")
fnv_1a_it64(u64 prv, u64 val)
{
    return (prv ^ val) * 0x100000001b3;
}

u32
fnv_1a_32(const u8 *addr, isize size)
{
    if (addr == nullptr || size == 0) return 0;

    u32 hash = fnv_1a_init32();

    for (isize i = 0; i < size; i++) {
        hash = fnv_1a_it32(hash, (u32)addr[i]);
    }

    return hash;
}

u64
fnv_1a_64(const u8 *addr, isize size)
{
    if (addr == nullptr || size == 0) return 0;
    
    u64 hash = fnv_1a_init64();
    
    for (isize i = 0; i < size; i++) {
        hash = fnv_1a_it64(hash, (u64)addr[i]);
    }
    
    return hash;
}

u16 crc16(const u8 *addr, isize size)
{
    u8 x;
    u16 crc = 0xFFFF;

    while (size--){
        x = crc >> 8 ^ *addr++;
        x ^= x>>4;
        crc = (u16)((crc << 8) ^ ((u16)(x << 12)) ^ ((u16)(x <<5)) ^ ((u16)x));
    }
    return crc;
}

u32
crc32(const u8 *addr, isize size)
{
    if (addr == nullptr || size == 0) return 0;

    u32 result = 0;

    // Setup lookup table
    u32 table[256];
    for(u32 i = 0; i < 256; i++) table[i] = crc32forByte(i);

    // Compute CRC-32 checksum
     for(isize i = 0; i < size; i++)
       result = table[(u8)result ^ addr[i]] ^ result >> 8;

    return result;
}

u32
crc32forByte(u32 r)
{
    for(int j = 0; j < 8; ++j)
        r = (r & 1? 0: (u32)0xEDB88320L) ^ r >> 1;
    return r ^ (u32)0xFF000000L;
}

}
