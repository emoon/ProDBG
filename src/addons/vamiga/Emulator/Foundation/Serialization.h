// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaTypes.h"
#include "Beam.h"
#include "Buffers.h"
#include "DDF.h"
#include "Event.h"
#include "Frame.h"
#include "TimeDelayed.h"
#include "Sampler.h"
#include "AudioStream.h"

#include <arpa/inet.h>

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

inline float readFloat(const u8 *& buf)
{
    float result;
    *((u32 *)(&result)) = read32(buf);
    return result;
}

inline double readDouble(const u8 *& buf)
{
    double result;
    *((u64 *)(&result)) = read64(buf);
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

inline void writeFloat(u8 *& buf, float value)
{
    write32(buf, *((u32 *)(&value)));
}

inline void writeDouble(u8 *& buf, double value)
{
    write64(buf, *((u64 *)(&value)));
}

//
// Counter (determines the state size)
//

#define COUNT(type) \
auto& operator&(type& v) \
{ \
count += sizeof(type); \
return *this; \
}

#define STRUCT(type) \
auto& operator&(type& v) \
{ \
v.applyToItems(*this); \
return *this; \
}

#define __ ,

class SerCounter
{
public:

    isize count;

    SerCounter() { count = 0; }

    COUNT(const bool)
    COUNT(const char)
    COUNT(const signed char)
    COUNT(const unsigned char)
    COUNT(const short)
    COUNT(const unsigned short)
    COUNT(const int)
    COUNT(const unsigned int)
    COUNT(const long long)
    COUNT(const unsigned long long)
    COUNT(const float)
    COUNT(const double)

    COUNT(const MemorySource)
    COUNT(const EventID)
    COUNT(const BusOwner)
    COUNT(const DDFState)
    COUNT(const SprDMAState)
    COUNT(const SamplingMethod)
    COUNT(const FilterType)
    COUNT(const SerialPortDevice)
    COUNT(const KeyboardState)
    COUNT(const DriveType)
    COUNT(const DriveState)
    COUNT(const RTCRevision)
    COUNT(const DiskDiameter)
    COUNT(const DiskDensity)
    COUNT(const CIARevision)
    COUNT(const AgnusRevision)
    COUNT(const DeniseRevision)

    STRUCT(Beam)
    STRUCT(DDF<true>)
    STRUCT(DDF<false>)
    STRUCT(Event)
    STRUCT(Frame)
    STRUCT(RegChange)
    template <class T, isize capacity> STRUCT(RingBuffer<T __ capacity>)
    template <class T, isize capacity> STRUCT(SortedRingBuffer<T __ capacity>)
    template <class T, int delay> STRUCT(TimeDelayed<T __ delay>)

    template <class T, isize N>
    SerCounter& operator&(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this & v[i];
        }
        return *this;
    }
};


//
// Reader (Deserializer)
//

#define DESERIALIZE(type,function) \
SerReader& operator&(type& v) \
{ \
v = (type)function(ptr); \
return *this; \
}

#define DESERIALIZE8(type)  static_assert(sizeof(type) == 1); DESERIALIZE(type,read8)
#define DESERIALIZE16(type) static_assert(sizeof(type) == 2); DESERIALIZE(type,read16)
#define DESERIALIZE32(type) static_assert(sizeof(type) == 4); DESERIALIZE(type,read32)
#define DESERIALIZE64(type) static_assert(sizeof(type) == 8); DESERIALIZE(type,read64)
#define DESERIALIZEF(type) static_assert(sizeof(type) == 4); DESERIALIZE(type,readFloat)
#define DESERIALIZED(type) static_assert(sizeof(type) == 8); DESERIALIZE(type,readDouble)

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
    DESERIALIZE32(int)
    DESERIALIZE32(unsigned int)
    DESERIALIZE64(long long)
    DESERIALIZE64(unsigned long long)
    DESERIALIZEF(float)
    DESERIALIZED(double)
    DESERIALIZE64(MemorySource)
    DESERIALIZE64(EventID)
    DESERIALIZE8(BusOwner)
    DESERIALIZE64(DDFState)
    DESERIALIZE64(SprDMAState)
    DESERIALIZE64(SamplingMethod)
    DESERIALIZE64(FilterType)
    DESERIALIZE64(SerialPortDevice)
    DESERIALIZE64(KeyboardState)
    DESERIALIZE64(DriveType)
    DESERIALIZE64(DriveState)
    DESERIALIZE64(RTCRevision)
    DESERIALIZE64(DiskDiameter)
    DESERIALIZE64(DiskDensity)
    DESERIALIZE64(CIARevision)
    DESERIALIZE64(AgnusRevision)
    DESERIALIZE64(DeniseRevision)

    STRUCT(Beam)
    STRUCT(DDF<true>)
    STRUCT(DDF<false>)
    STRUCT(Event)
    STRUCT(Frame)
    STRUCT(RegChange)
    template <class T, isize capacity> STRUCT(RingBuffer<T __ capacity>)
    template <class T, isize capacity> STRUCT(SortedRingBuffer<T __ capacity>)
    template <class T, int delay> STRUCT(TimeDelayed<T __ delay>)

    template <class T, isize N>
    SerReader& operator&(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this & v[i];
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
SerWriter& operator&(type& v) \
{ \
function(ptr, (cast)v); \
return *this; \
}

#define SERIALIZE8(type)  static_assert(sizeof(type) == 1); SERIALIZE(type,write8,u8)
#define SERIALIZE16(type) static_assert(sizeof(type) == 2); SERIALIZE(type,write16,u16)
#define SERIALIZE32(type) static_assert(sizeof(type) == 4); SERIALIZE(type,write32,u32)
#define SERIALIZE64(type) static_assert(sizeof(type) == 8); SERIALIZE(type,write64,u64)
#define SERIALIZEF(type) static_assert(sizeof(type) == 4); SERIALIZE(type,writeFloat,float)
#define SERIALIZED(type) static_assert(sizeof(type) == 8); SERIALIZE(type,writeDouble,double)

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
    SERIALIZE32(const int)
    SERIALIZE32(const unsigned int)
    SERIALIZE64(const long long)
    SERIALIZE64(const unsigned long long)
    SERIALIZEF(const float)
    SERIALIZED(const double)
    SERIALIZE64(const MemorySource)
    SERIALIZE64(const EventID)
    SERIALIZE8(const BusOwner)
    SERIALIZE64(const DDFState)
    SERIALIZE64(const SprDMAState)
    SERIALIZE64(const SamplingMethod)
    SERIALIZE64(const FilterType)
    SERIALIZE64(const SerialPortDevice)
    SERIALIZE64(const KeyboardState)
    SERIALIZE64(const DriveType)
    SERIALIZE64(const DriveState)
    SERIALIZE64(const RTCRevision)
    SERIALIZE64(const DiskDiameter)
    SERIALIZE64(const DiskDensity)
    SERIALIZE64(const CIARevision)
    SERIALIZE64(const AgnusRevision)
    SERIALIZE64(const DeniseRevision)

    STRUCT(Beam)
    STRUCT(DDF<true>)
    STRUCT(DDF<false>)
    STRUCT(Event)
    STRUCT(Frame)
    STRUCT(RegChange)
    template <class T, isize capacity> STRUCT(RingBuffer<T __ capacity>)
    template <class T, isize capacity> STRUCT(SortedRingBuffer<T __ capacity>)
    template <class T, int delay> STRUCT(TimeDelayed<T __ delay>)

    template <class T, isize N>
    SerWriter& operator&(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this & v[i];
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
SerResetter& operator&(type& v) \
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
    RESET(MemorySource)
    RESET(EventID)
    RESET(BusOwner)
    RESET(DDFState)
    RESET(SprDMAState)
    RESET(SamplingMethod)
    RESET(FilterType)
    RESET(SerialPortDevice)
    RESET(KeyboardState)
    RESET(DriveType)
    RESET(DriveState)
    RESET(RTCRevision)

    STRUCT(Beam)
    STRUCT(DDF<true>)
    STRUCT(DDF<false>)
    STRUCT(Event)
    STRUCT(Frame)
    STRUCT(RegChange)
    template <class T, isize capacity> STRUCT(RingBuffer<T __ capacity>)
    template <class T, isize capacity> STRUCT(SortedRingBuffer<T __ capacity>)
    template <class T, int delay> STRUCT(TimeDelayed<T __ delay>)

    template <class T, isize N>
    SerResetter& operator&(T (&v)[N])
    {
        for(isize i = 0; i < N; ++i) {
            *this & v[i];
        }
        return *this;
    }
};
