// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Types.h"

namespace util {

// Returns the FNV-1a seed value
inline u32 fnv_1a_init32() { return 0x811c9dc5; }
inline u64 fnv_1a_init64() { return 0xcbf29ce484222325; }

// Performs a single iteration of the FNV-1a hash algorithm
u32 fnv_1a_it32(u32 prv, u32 val);
u64 fnv_1a_it64(u64 prv, u64 val);

// Computes a FNV-1a checksum for a given buffer
u32 fnv_1a_32(const u8 *addr, isize size);
u64 fnv_1a_64(const u8 *addr, isize size);

// Computes a CRC checksum for a given buffer
u16 crc16(const u8 *addr, isize size);
u32 crc32(const u8 *addr, isize size);
u32 crc32forByte(u32 r);

}
