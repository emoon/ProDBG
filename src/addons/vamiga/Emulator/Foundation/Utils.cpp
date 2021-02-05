// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"

#include <ctype.h>

bool
releaseBuild()
{
#ifdef RELEASEBUILD
    return true;
#else
    return false;
#endif
}

void hexdump(u8 *p, isize size, isize cols, isize pad)
{
    while (size) {
        
        isize cnt = MIN(size, cols);
        for (isize x = 0; x < cnt; x++) {
            fprintf(stderr, "%02X %s", p[x], ((x + 1) % pad) == 0 ? " " : "");
        }
        
        size -= cnt;
        p += cnt;
        
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void hexdump(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, cols);
}

void hexdumpWords(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, 2);
}

void hexdumpLongwords(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, 4);
}

string extractPath(const string &s)
{
    auto idx = s.rfind('/');
    auto pos = 0;
    auto len = idx != string::npos ? idx + 1 : 0;
    return s.substr(pos, len);
}

string extractName(const string &s)
{
    auto idx = s.rfind('/');
    auto pos = idx != string::npos ? idx + 1 : 0;
    auto len = string::npos;
    return s.substr(pos, len);
}

string extractSuffix(const string &s)
{
    auto idx = s.rfind('.');
    auto pos = idx != string::npos ? idx + 1 : 0;
    auto len = string::npos;
    return s.substr(pos, len);
}

string stripPath(const string &s)
{
    auto idx = s.rfind('/');
    auto pos = idx != string::npos ? idx + 1 : 0;
    auto len = string::npos;
    return s.substr(pos, len);
}

string stripName(const string &s)
{
    auto idx = s.rfind('/');
    auto pos = 0;
    auto len = idx != string::npos ? idx : 0;
    return s.substr(pos, len);
}

string stripSuffix(const string &s)
{
    auto idx = s.rfind('.');
    auto pos = 0;
    auto len = idx != string::npos ? idx : string::npos;
    return s.substr(pos, len);
}

bool isDirectory(const string &path)
{
    return isDirectory(path.c_str());
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

isize numDirectoryItems(const string &path)
{
    return numDirectoryItems(path.c_str());
}

isize numDirectoryItems(const char *path)
{
    isize count = 0;
    
    if (DIR *dir = opendir(path)) {
        
        struct dirent *dp;
        while ((dp = readdir(dir))) {
            if (dp->d_name[0] != '.') count++;
        }
    }
    
    return count;
}

isize
getSizeOfFile(const string &path)
{
    return getSizeOfFile(path.c_str());
}

isize
getSizeOfFile(const char *path)
{
    struct stat fileProperties;
        
    if (stat(path, &fileProperties) != 0)
        return -1;
    
    return fileProperties.st_size;
}

bool matchingStreamHeader(std::istream &stream, const u8 *header, isize len)
{
    stream.seekg(0, std::ios::beg);
    
    for (isize i = 0; i < len; i++) {
        int c = stream.get();
        if (c != (int)header[i]) {
            stream.seekg(0, std::ios::beg);
            return false;
        }
    }
    stream.seekg(0, std::ios::beg);
    return true;
}

bool
matchingBufferHeader(const u8 *buffer, const u8 *header, isize len)
{
    assert(buffer != nullptr);
    assert(header != nullptr);
    
    for (isize i = 0; i < len; i++) {
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
    for (isize i = 0; i < bytes; i++) {
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

u32 __attribute__((no_sanitize("unsigned-integer-overflow")))
fnv_1a_it32(u32 prv, u32 val)
{
    return (prv ^ val) * 0x1000193;
}
 
u64 __attribute__((no_sanitize("unsigned-integer-overflow")))
fnv_1a_it64(u64 prv, u64 val)
{
    return (prv ^ val) * 0x100000001b3;
}

isize
streamLength(std::istream &stream)
{
    auto cur = stream.tellg();
    stream.seekg(0, std::ios::beg);
    auto beg = stream.tellg();
    stream.seekg(0, std::ios::end);
    auto end = stream.tellg();
    stream.seekg(cur, std::ios::beg);
    
    return (isize)(end - beg);
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
    for(int i = 0; i < 256; i++) table[i] = crc32forByte(i);

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
