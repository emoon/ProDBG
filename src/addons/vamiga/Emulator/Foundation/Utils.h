// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaConfig.h"
#include "AmigaConstants.h"
#include "Debug.h"
#include "Errors.h"
#include "AmigaTypes.h"

#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <set>
#include <sstream>
#include <fstream>

using std::string;

//
// Optimizing code
//

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

// Returns true if this executable is a release build
bool releaseBuild();


//
// Converting units
//

// Macros for converting kilo bytes and mega bytes to bytes
#define KB(x) ((x) << 10)
#define MB(x) ((x) << 20)

// Macros for converting kilo Hertz and mega Hertz to Hertz
#define KHz(x) ((x) * 1000)
#define MHz(x) ((x) * 1000000)


//
// Performing overflow-prone arithmetic
//

// Sanitizer friendly macros for adding signed offsets to u32 values
#define U32_ADD(x,y) (u32)((i64)(x) + (i64)(y))
#define U32_SUB(x,y) (u32)((i64)(x) - (i64)(y))
#define U32_ADD3(x,y,z) (u32)((i64)(x) + (i64)(y) + (i64)(z))
#define U32_SUB3(x,y,z) (u32)((i64)(x) - (i64)(y) - (i64)(z))

// Sanitizer friendly macros for adding signed offsets to u64 values
#define U64_ADD(x,y) (u64)((i64)(x) + (i64)(y))
#define U64_SUB(x,y) (u64)((i64)(x) - (i64)(y))
#define U64_ADD3(x,y,z) (u64)((i64)(x) + (i64)(y) + (i64)(z))
#define U64_SUB3(x,y,z) (u64)((i64)(x) - (i64)(y) - (i64)(z))


//
// Accessing bits and bytes
//

// Returns the low byte or the high byte of a 16 bit value
#define LO_BYTE(x) (u8)((x) & 0xFF)
#define HI_BYTE(x) (u8)((x) >> 8)

// Returns the low word or the high word of a 32 bit value
#define LO_WORD(x) (u16)((x) & 0xFFFF)
#define HI_WORD(x) (u16)((x) >> 16)

// Constructs a larger integer in little endian byte format
#define LO_HI(x,y) (u16)((y) << 8 | (x))
#define LO_LO_HI(x,y,z) (u32)((z) << 16 | (y) << 8 | (x))
#define LO_LO_HI_HI(x,y,z,w) (u32)((w) << 24 | (z) << 16 | (y) << 8 | (x))
#define LO_W_HI_W(x,y) (u32)((y) << 16 | (x))

// Constructs a larger integer in big endian byte format
#define HI_LO(x,y) (u16)((x) << 8 | (y))
#define HI_HI_LO(x,y,z) (u32)((x) << 16 | (y) << 8 | (z))
#define HI_HI_LO_LO(x,y,z,w) (u32)((x) << 24 | (y) << 16 | (z) << 8 | (w))
#define HI_W_LO_W(x,y) (u32)((x) << 16 | (y))

// Returns a certain byte of a larger integer
#define BYTE0(x) LO_BYTE(x)
#define BYTE1(x) LO_BYTE((x) >> 8)
#define BYTE2(x) LO_BYTE((x) >> 16)
#define BYTE3(x) LO_BYTE((x) >> 24)

// Returns a non-zero value if the n-th bit is set in x
#define GET_BIT(x,nr) ((x) & (1 << (nr)))

// Sets, clears, or toggles single bits
#define SET_BIT(x,nr) ((x) |= (1 << (nr)))
#define CLR_BIT(x,nr) ((x) &= ~(1 << (nr)))
#define TOGGLE_BIT(x,nr) ((x) ^= (1 << (nr)))

// Replaces bits, bytes, and words
#define REPLACE_BIT(x,nr,v) ((v) ? SET_BIT(x, nr) : CLR_BIT(x, nr))
#define REPLACE_LO(x,y) (((x) & ~0x00FF) | (y))
#define REPLACE_HI(x,y) (((x) & ~0xFF00) | ((y) << 8))
#define REPLACE_LO_WORD(x,y) (((x) & ~0xFFFF) | (y))
#define REPLACE_HI_WORD(x,y) (((x) & ~0xFFFF0000) | ((y) << 16))

// Checks for a rising or a falling edge
#define RISING_EDGE(x,y) (!(x) && (y))
#define RISING_EDGE_BIT(x,y,n) (!((x) & (1 << (n))) && ((y) & (1 << (n))))
#define FALLING_EDGE(x,y) ((x) && !(y))
#define FALLING_EDGE_BIT(x,y,n) (((x) & (1 << (n))) && !((y) & (1 << (n))))

// Checks is a number is even or odd
#define IS_EVEN(x) (!IS_ODD(x))
#define IS_ODD(x) ((x) & 1)

// Rounds a number up or down to the next even or odd number
#define UP_TO_NEXT_EVEN(x) ((x) + ((x) & 1))
#define DOWN_TO_NEXT_EVEN(x) ((x) & (~1))
#define UP_TO_NEXT_ODD(x) ((x) | 1)
#define DOWN_TO_NEXT_ODD(x) ((x) - !((x) & 1))


//
// Accessing memory
//

// Reads a value in big-endian format
#define R8BE(a)  (*(u8 *)(a))
#define R16BE(a) HI_LO(*(u8 *)(a), *(u8 *)((a)+1))
#define R32BE(a) HI_HI_LO_LO(*(u8 *)(a), *(u8 *)((a)+1), *(u8 *)((a)+2), *(u8 *)((a)+3))

#define R8BE_ALIGNED(a)  (*(u8 *)(a))
#define R16BE_ALIGNED(a) (htons(*(u16 *)(a)))
#define R32BE_ALIGNED(a) (htonl(*(u32 *)(a)))

// Writes a value in big-endian format
#define W8BE(a,v)  { *(u8 *)(a) = (v); }
#define W16BE(a,v) { *(u8 *)(a) = HI_BYTE(v); *(u8 *)((a)+1) = LO_BYTE(v); }
#define W32BE(a,v) { W16BE(a,HI_WORD(v)); W16BE((a)+2,LO_WORD(v)); }

#define W8BE_ALIGNED(a,v)  { *(u8 *)(a) = (u8)(v); }
#define W16BE_ALIGNED(a,v) { *(u16 *)(a) = ntohs((u16)v); }
#define W32BE_ALIGNED(a,v) { *(u32 *)(a) = ntohl((u32)v); }


//
// Pretty printing
//

// Prints a hex dump of a buffer to the console (for debugging)
void hexdump(u8 *p, isize size, isize cols, isize pad);
void hexdump(u8 *p, isize size, isize cols = 32);
void hexdumpWords(u8 *p, isize size, isize cols = 32);
void hexdumpLongwords(u8 *p, isize size, isize cols = 32);


//
// Handling files
//

// Extracts a certain component from a path
string extractPath(const string &path);
string extractName(const string &path);
string extractSuffix(const string &path);

// Strips a certain component from a path
string stripPath(const string &path);
string stripName(const string &path);
string stripSuffix(const string &path);

// Returns the size of a file in bytes
isize getSizeOfFile(const string &path);
isize getSizeOfFile(const char *path);

// Checks if a path points to a directory
bool isDirectory(const string &path);
bool isDirectory(const char *path);

// Returns the number of files in a directory
isize numDirectoryItems(const string &path);
isize numDirectoryItems(const char *path);

// Checks the header signature (magic bytes) of a stream or buffer
bool matchingStreamHeader(std::istream &stream, const u8 *header, isize len);
bool matchingBufferHeader(const u8 *buffer, const u8 *header, isize len);

// Loads a file from disk
bool loadFile(const char *path, u8 **buffer, isize *size);
bool loadFile(const char *path, const char *name, u8 **buffer, isize *size);


//
// Handling streams
//

isize streamLength(std::istream &stream);


//
// Computing checksums
//

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
