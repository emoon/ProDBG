// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_UTILS_H
#define _AMIGA_UTILS_H

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

#include "AmigaConfig.h"
#include "AmigaConstants.h"


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
// Handling files
//

/* Extracts the first path component.
 * Returns a newly created string. You need to delete it manually.
 */
char *extractFirstPathComponent(const char *path);

/* Extracts the n-th path component.
 * Returns a newly created string. You need to delete it manually.
 */
char *extractPathComponent(const char *path, unsigned n);

/* Strips the filename from a path.
 * Returns a newly created string. You need to delete it manually.
 */
char *stripFilename(const char *path);

/* Extracts the filename from a path.
 * Returns a newly created string. You need to delete it manually.
 */
char *extractFilename(const char *path);

/* Replaces a filename from a path.
 * Returns a newly created string. You need to delete it manually.
 */
char *replaceFilename(const char *path, const char *name);

/* Extracts file suffix from a path.
 * Returns a newly created string. You need to delete it manually.
 */
char *extractSuffix(const char *path);

/* Extracts filename from a path without its suffix.
 * Returns a newly created string. You need to delete it manually.
 */
char *extractFilenameWithoutSuffix(const char *path);

/* Compares the file suffix with a given string.
 * The function is used for determining the type of a file.
 */
bool checkFileSuffix(const char *path, const char *suffix);

// Checks if a path points to a directory
bool isDirectory(const char *path);

// Returns the size of a file in bytes
long getSizeOfFile(const char *path);

// Checks the size of a file
bool checkFileSize(const char *path, long size);
bool checkFileSizeRange(const char *path, long min, long max);

// Checks the header signature (magic bytes) of a file or buffer
bool matchingFileHeader(const char *path, const u8 *header, size_t length);
bool matchingBufferHeader(const u8 *buffer, const u8 *header, size_t length);

// Loads a file from disk
bool loadFile(const char *path, u8 **buffer, long *size);
bool loadFile(const char *path, const char *name, u8 **buffer, long *size);


//
// Controlling time
//

// Puts the current thread to sleep for a given amout of micro seconds
// void sleepMicrosec(unsigned usec);

/* Sleeps until the kernel timer reaches kernelTargetTime
 *
 * kernelEarlyWakeup: To increase timing precision, the function wakes up the
 *                    thread earlier by this amount and waits actively in a
 *                    delay loop until the deadline is reached.
 *
 * Returns the overshoot time (jitter), measured in kernel time units. Smaller
 * values are better, 0 is best.
 */
// i64 sleepUntil(u64 kernelTargetTime, u64 kernelEarlyWakeup);


//
// Computing checksums
//

// Returns the FNV-1a seed value
inline u32 fnv_1a_init32() { return 0x811c9dc5; }
inline u64 fnv_1a_init64() { return 0xcbf29ce484222325; }

// Performs a single iteration of the FNV-1a hash algorithm
inline u32 fnv_1a_it32(u32 prv, u32 val) { return (prv ^ val) * 0x1000193; }
inline u64 fnv_1a_it64(u64 prv, u64 val) { return (prv ^ val) * 0x100000001b3; }

// Computes a FNV-1a checksum for a given buffer
u32 fnv_1a_32(const u8 *addr, size_t size);
u64 fnv_1a_64(const u8 *addr, size_t size);

// Computes a CRC checksum for a given buffer
u16 crc16(const u8 *addr, size_t size);
u32 crc32(const u8 *addr, size_t size);
u32 crc32forByte(u32 r);

// Computes a SHA-1 checksum for a given buffer
int sha_1(u8 *digest, char *hexdigest, const u8 *addr, size_t size);

#endif
