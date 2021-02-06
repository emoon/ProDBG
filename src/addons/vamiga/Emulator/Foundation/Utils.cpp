// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"

bool
releaseBuild()
{
#ifdef RELEASEBUILD
    return true;
#else
    return false;
#endif
}

char *
extractFirstPathComponent(const char *path)
{
    assert(path != nullptr);
    
    const char *pos = strchr(path, '/');
    return pos ? strdup(pos + 1) : strdup(path);
}

char *
extractPathComponent(const char *path, unsigned n)
{
    assert(path != nullptr);

    // Seek the n-th occurance of '/'
    while (n--) if ((path = strchr(path, '/')) == nullptr) return nullptr;

    // Return the first path component
    return extractFirstPathComponent(path);
}

char *
stripFilename(const char *path)
{
    assert(path != nullptr);
    
    const char *pos = strrchr(path, '/');
    return pos ? strndup(path, pos + 1 - path) : strdup("");
}

char *
extractFilename(const char *path)
{
    assert(path != nullptr);
    
    const char *pos = strrchr(path, '/');
    return pos ? strdup(pos + 1) : strdup(path);
}

char *
replaceFilename(const char *path, const char *name)
{
    char *dir = stripFilename(path);
    char *result = (char *)malloc(strlen(dir) + strlen(name) + 1);

    strcpy(result, dir);
    strcat(result, name);

    delete(dir);
    return result;
}

char *
extractSuffix(const char *path)
{
    assert(path != nullptr);
    
    const char *pos = strrchr(path, '.');
    return pos ? strdup(pos + 1) : strdup("");
}

char *
extractFilenameWithoutSuffix(const char *path)
{
    assert(path != nullptr);
    
    char *result;
    char *filename = extractFilename(path);
    char *suffix   = extractSuffix(filename);
    
    if (strlen(suffix) == 0)
        result = strdup(filename);
    else
        result = strndup(filename, strlen(filename) - strlen(suffix) - 1);
    
    free(filename);
    free(suffix);
    return result;
}

bool
checkFileSuffix(const char *path, const char *suffix)
{
    assert(path != nullptr);
    assert(suffix != nullptr);
    
    if (strlen(suffix) > strlen(path))
        return false;
    
    path += (strlen(path) - strlen(suffix));
    if (strcmp(path, suffix) == 0)
        return true;
    else
        return false;
}

bool isDirectory(const char *path)
{
    struct stat fileProperties;
    
    if (path == nullptr)
        return -1;
        
    if (stat(path, &fileProperties) != 0)
        return -1;
    
    return S_ISDIR(fileProperties.st_mode);
}

long
getSizeOfFile(const char *path)
{
    struct stat fileProperties;
    
    if (path == nullptr)
        return -1;
    
    if (stat(path, &fileProperties) != 0)
        return -1;
    
    return fileProperties.st_size;
}

bool
checkFileSize(const char *path, long size)
{
    return checkFileSizeRange(path, size, size);
}

bool
checkFileSizeRange(const char *path, long min, long max)
{
    long filesize = getSizeOfFile(path);
    
    if (filesize == -1)
        return false;
    
    if (min > 0 && filesize < min)
        return false;
    
    if (max > 0 && filesize > max)
        return false;
    
    return true;
}

bool
matchingFileHeader(const char *path, const u8 *header, size_t length)
{
    assert(path != nullptr);
    assert(header != nullptr);
    
    bool result = true;
    FILE *file;
    
    if ((file = fopen(path, "r")) == nullptr)
        return false;
    
    for (unsigned i = 0; i < length; i++) {
        int c = fgetc(file);
        if (c != (int)header[i]) {
            result = false;
            break;
        }
    }
    
    fclose(file);
    return result;
}

bool
matchingBufferHeader(const u8 *buffer, const u8 *header, size_t length)
{
    assert(buffer != nullptr);
    assert(header != nullptr);
    
    for (unsigned i = 0; i < length; i++) {
        if (header[i] != buffer[i])
            return false;
    }

    return true;
}

bool
loadFile(const char *path, u8 **buffer, long *size)
{
    assert(path != nullptr);
    assert(buffer != nullptr);
    assert(size != nullptr);

    *buffer = nullptr;
    *size = 0;
    
    // Get file size
    long bytes = getSizeOfFile(path);
    if (bytes == -1) return false;
    
    // Open file
    FILE *file = fopen(path, "r");
    if (file == nullptr) return false;
     
    // Allocate memory
    u8 *data = new u8[bytes];
    if (data == nullptr) { fclose(file); return false; }
    
    // Read data
    for (unsigned i = 0; i < bytes; i++) {
        int c = fgetc(file);
        if (c == EOF) break;
        data[i] = (u8)c;
    }
    
    fclose(file);
    *buffer = data;
    *size = bytes;
    return true;
}

bool
loadFile(const char *path, const char *name, u8 **buffer, long *size)
{
    assert(path != nullptr);
    assert(name != nullptr);

    char *fullpath = new char[strlen(path) + strlen(name) + 2];
    strcpy(fullpath, path);
    strcat(fullpath, "/");
    strcat(fullpath, name);
    
    return loadFile(fullpath, buffer, size);
}

u32
fnv_1a_32(const u8 *addr, size_t size)
{
    if (addr == nullptr || size == 0) return 0;

    u32 hash = fnv_1a_init32();

    for (size_t i = 0; i < size; i++) {
        hash = fnv_1a_it32(hash, (u32)addr[i]);
    }

    return hash;
}

u64
fnv_1a_64(const u8 *addr, size_t size)
{
    if (addr == nullptr || size == 0) return 0;
    
    u64 hash = fnv_1a_init64();
    
    for (size_t i = 0; i < size; i++) {
        hash = fnv_1a_it64(hash, (u64)addr[i]);
    }
    
    return hash;
}

u16 crc16(const u8 *addr, size_t size)
{
    u8 x;
    u16 crc = 0xFFFF;

    while (size--){
        x = crc >> 8 ^ *addr++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((u16)(x << 12)) ^ ((u16)(x <<5)) ^ ((u16)x);
    }
    return crc;
}

u32
crc32(const u8 *addr, size_t size)
{
    if (addr == nullptr || size == 0) return 0;

    u32 result = 0;

    // Setup lookup table
    u32 table[256];
    for(int i = 0; i < 256; i++) table[i] = crc32forByte(i);

    // Compute CRC-32 checksum
     for(size_t i = 0; i < size; i++)
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

int
sha_1(u8 *digest, char *hexdigest, const u8 *addr, size_t size)
{
    // Slight modification of https://github.com/CTrabant/teeny-sha1

#define SHA1ROTATELEFT(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

    u32 W[80];
    u32 H[] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
    u32 a, b, c, d, e, f = 0, k = 0;
    u32 idx, lidx, widx, didx = 0;

    i32 wcount;
    u32 temp;
    u64 databits = ((uint64_t)size) * 8;
    u32 loopcount = (size + 8) / 64 + 1;
    u32 tailbytes = 64 * loopcount - size;
    u8 datatail[128] = {0};

    if (!digest && !hexdigest)
        return -1;

    if (!addr)
        return -1;

    // Pre-processing of data tail (includes padding to fill out 512-bit chunk):
    // Add bit '1' to end of message (big-endian)
    // Add 64-bit message length in bits at very end (big-endian)

    datatail[0] = 0x80;
    datatail[tailbytes - 8] = (uint8_t) (databits >> 56 & 0xFF);
    datatail[tailbytes - 7] = (uint8_t) (databits >> 48 & 0xFF);
    datatail[tailbytes - 6] = (uint8_t) (databits >> 40 & 0xFF);
    datatail[tailbytes - 5] = (uint8_t) (databits >> 32 & 0xFF);
    datatail[tailbytes - 4] = (uint8_t) (databits >> 24 & 0xFF);
    datatail[tailbytes - 3] = (uint8_t) (databits >> 16 & 0xFF);
    datatail[tailbytes - 2] = (uint8_t) (databits >> 8 & 0xFF);
    datatail[tailbytes - 1] = (uint8_t) (databits >> 0 & 0xFF);

    // Process each 512-bit chunk
    for (lidx = 0; lidx < loopcount; lidx++)
    {
        // Compute all elements in W
        memset (W, 0, 80 * sizeof (uint32_t));

        // Break 512-bit chunk into sixteen 32-bit, big endian words
        for (widx = 0; widx <= 15; widx++)
        {
            wcount = 24;

            // Copy byte-per byte from specified buffer
            while (didx < size && wcount >= 0)
            {
                W[widx] += (((uint32_t)addr[didx]) << wcount);
                didx++;
                wcount -= 8;
            }
            // Fill out W with padding as needed
            while (wcount >= 0)
            {
                W[widx] += (((uint32_t)datatail[didx - size]) << wcount);
                didx++;
                wcount -= 8;
            }
        }

        // Extend the sixteen 32-bit words into eighty 32-bit words, with
        // potential optimization from: "Improving the Performance of the
        // Secure Hash Algorithm (SHA-1)" by Max Locktyukhin
        for (widx = 16; widx <= 31; widx++)
        {
            W[widx] = SHA1ROTATELEFT ((W[widx - 3] ^ W[widx - 8] ^ W[widx - 14] ^ W[widx - 16]), 1);
        }
        for (widx = 32; widx <= 79; widx++)
        {
            W[widx] = SHA1ROTATELEFT ((W[widx - 6] ^ W[widx - 16] ^ W[widx - 28] ^ W[widx - 32]), 2);
        }

        // Main loop
        a = H[0];
        b = H[1];
        c = H[2];
        d = H[3];
        e = H[4];

        for (idx = 0; idx <= 79; idx++)
        {
            if (idx <= 19)
            {
                f = (b & c) | ((~b) & d);
                k = 0x5A827999;
            }
            else if (idx >= 20 && idx <= 39)
            {
                f = b ^ c ^ d;
                k = 0x6ED9EBA1;
            }
            else if (idx >= 40 && idx <= 59)
            {
                f = (b & c) | (b & d) | (c & d);
                k = 0x8F1BBCDC;
            }
            else if (idx >= 60 && idx <= 79)
            {
                f = b ^ c ^ d;
                k = 0xCA62C1D6;
            }
            temp = SHA1ROTATELEFT (a, 5) + f + e + k + W[idx];
            e = d;
            d = c;
            c = SHA1ROTATELEFT (b, 30);
            b = a;
            a = temp;
        }

        H[0] += a;
        H[1] += b;
        H[2] += c;
        H[3] += d;
        H[4] += e;
    }

    // Store binary digest in supplied buffer
    if (digest)
    {
        for (idx = 0; idx < 5; idx++)
        {
            digest[idx * 4 + 0] = (uint8_t) (H[idx] >> 24);
            digest[idx * 4 + 1] = (uint8_t) (H[idx] >> 16);
            digest[idx * 4 + 2] = (uint8_t) (H[idx] >> 8);
            digest[idx * 4 + 3] = (uint8_t) (H[idx]);
        }
    }

    // Store hex version of digest in supplied buffer
    if (hexdigest)
    {
        snprintf (hexdigest, 41, "%08x%08x%08x%08x%08x",
                  H[0],H[1],H[2],H[3],H[4]);
    }

    return 0;
}
