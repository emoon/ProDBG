// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <string.h>
#include "Macros.h"

namespace util {

//
// Basic memory buffer I/O
//

inline u8 read8(const u8 *& buf)
{
    u8 result = R8BE(buf);
    buf += 1;
    return result;
}

inline u16 read16(const u8 *& buf)
{
    u16 result = R16BE(buf);
    buf += 2;
    return result;
}

inline u32 read32(const u8 *& buf)
{
    u32 result = R32BE(buf);
    buf += 4;
    return result;
}

inline u64 read64(const u8 *& buf)
{
    u32 hi = read32(buf);
    u32 lo = read32(buf);
    return ((u64)hi << 32) | lo;
}

inline double readDouble(const u8 *& buf)
{
    double result = 0;
    for (isize i = 0; i < 8; i++) ((u8 *)&result)[i] = read8(buf);
    return result;
}

inline void write8(u8 *& buf, u8 value)
{
    W8BE(buf, value);
    buf += 1;
}

inline void write16(u8 *& buf, u16 value)
{
    W16BE(buf, value);
    buf += 2;
}

inline void write32(u8 *& buf, u32 value)
{
    W32BE(buf, value);
    buf += 4;
}

inline void write64(u8 *& buf, u64 value)
{
    write32(buf, (u32)(value >> 32));
    write32(buf, (u32)(value));
}

inline void writeDouble(u8 *& buf, double value)
{
    for (isize i = 0; i < 8; i++) write8(buf, ((u8 *)&value)[i]);
}


//
// Counter (determines the state size)
//

#define COUNT(type,size) \
auto& operator<<(type& v) \
{ \
count += size; \
return *this; \
}

#define COUNT8(type) static_assert(sizeof(type) == 1); COUNT(type,1)
#define COUNT16(type) static_assert(sizeof(type) == 2); COUNT(type,2)
#define COUNT64(type) static_assert(sizeof(type) <= 8); COUNT(type,8)
#define COUNTD(type) static_assert(sizeof(type) <= 8); COUNT(type,8)

class SerCounter
{
public:

    isize count;

    SerCounter() { count = 0; }

    COUNT8(const bool)
    COUNT8(const char)
    COUNT8(const signed char)
    COUNT8(const unsigned char)
    COUNT16(const short)
    COUNT16(const unsigned short)
    COUNT64(const int)
    COUNT64(const unsigned int)
    COUNT64(const long)
    COUNT64(const unsigned long)
    COUNT64(const long long)
    COUNT64(const unsigned long long)
    COUNTD(const float)
    COUNTD(const double)

    template <class T, isize N>
    SerCounter& operator<<(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this << v[i];
        }
        return *this;
    }

    template <class T>
    SerCounter& operator>>(T &v)
    {
        v << *this;
        return *this;
    }

    template <class T, isize N>
    SerCounter& operator>>(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            v[i] << *this;
        }
        return *this;
    }
};


//
// Reader (Deserializer)
//

#define DESERIALIZE(type,function) \
SerReader& operator<<(type& v) \
{ \
v = (type)function(ptr); \
return *this; \
}

#define DESERIALIZE8(type)  static_assert(sizeof(type) == 1); DESERIALIZE(type,read8)
#define DESERIALIZE16(type) static_assert(sizeof(type) == 2); DESERIALIZE(type,read16)
#define DESERIALIZE64(type) static_assert(sizeof(type) <= 8); DESERIALIZE(type,read64)
#define DESERIALIZED(type) static_assert(sizeof(type) <= 8); DESERIALIZE(type,readDouble)

class SerReader
{
public:

    const u8 *ptr;

    SerReader(const u8 *p) : ptr(p)
    {
    }

    DESERIALIZE8(bool)
    DESERIALIZE8(char)
    DESERIALIZE8(signed char)
    DESERIALIZE8(unsigned char)
    DESERIALIZE16(short)
    DESERIALIZE16(unsigned short)
    DESERIALIZE64(int)
    DESERIALIZE64(unsigned int)
    DESERIALIZE64(long)
    DESERIALIZE64(unsigned long)
    DESERIALIZE64(long long)
    DESERIALIZE64(unsigned long long)
    DESERIALIZED(float)
    DESERIALIZED(double)

    template <class T, isize N>
    SerReader& operator<<(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this << v[i];
        }
        return *this;
    }

    template <class T>
    SerReader& operator>>(T &v)
    {
        v << *this;
        return *this;
    }

    template <class T, isize N>
    SerReader& operator>>(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            v[i] << *this;
        }
        return *this;
    }

    void copy(void *dst, isize n)
    {
        memcpy(dst, (void *)ptr, n);
        ptr += n;
    }
};


//
// Writer (Serializer)
//

#define SERIALIZE(type,function,cast) \
SerWriter& operator<<(type& v) \
{ \
function(ptr, (cast)v); \
return *this; \
}

#define SERIALIZE8(type)  static_assert(sizeof(type) == 1); SERIALIZE(type,write8,u8)
#define SERIALIZE16(type) static_assert(sizeof(type) == 2); SERIALIZE(type,write16,u16)
#define SERIALIZE64(type) static_assert(sizeof(type) <= 8); SERIALIZE(type,write64,u64)
#define SERIALIZED(type) static_assert(sizeof(type) <= 8); SERIALIZE(type,writeDouble,double)

class SerWriter
{
public:

    u8 *ptr;

    SerWriter(u8 *p) : ptr(p)
    {
    }

    SERIALIZE8(const bool)
    SERIALIZE8(const char)
    SERIALIZE8(const signed char)
    SERIALIZE8(const unsigned char)
    SERIALIZE16(const short)
    SERIALIZE16(const unsigned short)
    SERIALIZE64(const int)
    SERIALIZE64(const unsigned int)
    SERIALIZE64(const long)
    SERIALIZE64(const unsigned long)
    SERIALIZE64(const long long)
    SERIALIZE64(const unsigned long long)
    SERIALIZED(const float)
    SERIALIZED(const double)

    template <class T, isize N>
    SerWriter& operator<<(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this << v[i];
        }
        return *this;
    }

    template <class T>
    SerWriter& operator>>(T &v)
    {
        v << *this;
        return *this;
    }

    template <class T, isize N>
    SerWriter& operator>>(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            v[i] << *this;
        }
        return *this;
    }

    void copy(const void *src, isize n)
    {
        memcpy((void *)ptr, src, n);
        ptr += n;
    }

};


//
// Resetter
//

#define RESET(type) \
SerResetter& operator<<(type& v) \
{ \
v = (type)0; \
return *this; \
}

class SerResetter
{
public:

    SerResetter()
    {
    }

    RESET(bool)
    RESET(char)
    RESET(signed char)
    RESET(unsigned char)
    RESET(short)
    RESET(unsigned short)
    RESET(int)
    RESET(unsigned int)
    RESET(long)
    RESET(unsigned long)
    RESET(long long)
    RESET(unsigned long long)
    RESET(float)
    RESET(double)

    template <class T, isize N>
    SerResetter& operator<<(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this << v[i];
        }
        return *this;
    }

    template <class T>
    SerResetter& operator>>(T &v)
    {
        v << *this;
        return *this;
    }

    template <class T, isize N>
    SerResetter& operator>>(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            v[i] << *this;
        }
        return *this;
    }
};

}
