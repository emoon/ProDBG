// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSBlock.h"

FSString::FSString(const char *cStr, size_t l) : limit(l)
{
    assert(cStr != nullptr);
    assert(limit <= 91);
    
    strncpy(this->cStr, cStr, limit);
    this->cStr[limit] = 0;
}

FSString::FSString(const u8 *bcplStr, size_t l) : limit(l)
{
    assert(bcplStr != nullptr);
    assert(limit <= 91);

    // First entry of BCPL string contains the string length
    u8 len = MIN(bcplStr[0], limit);

    strncpy(cStr, (const char *)(bcplStr + 1), limit);
    cStr[len] = 0;
}

char
FSString::capital(char c)
{
    return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c;
}

bool
FSString::operator== (FSString &rhs)
{
    int n = 0;
    
    while (cStr[n] != 0 || rhs.cStr[n] != 0) {
        if (capital(cStr[n]) != capital(rhs.cStr[n])) return false;
        n++;
    }
    return true;
}

u32
FSString::hashValue()
{
    size_t length = strlen(cStr);
    u32 result = (u32)length;
    
    for (size_t i = 0; i < length; i++) {
        char c = capital(cStr[i]);
        result = (result * 13 + (u32)c) & 0x7FF;
    }
    return result;
}

void
FSString::write(u8 *p)
{
    assert(p != nullptr);
    assert(strlen(cStr) < sizeof(cStr));

    // Write name as a BCPL string (first byte is string length)
    p[0] = strlen(cStr);
    strncpy((char *)(p + 1), cStr, strlen(cStr));
}

void
FSName::rectify()
{
    // Replace all symbols that are not permitted in Amiga filenames
    for (size_t i = 0; i < sizeof(cStr); i++) {
        if (cStr[i] == ':' || cStr[i] == '/') cStr[i] = '_';
    }
}

FSTime::FSTime(time_t t)
{
    const u32 secPerDay = 24 * 60 * 60;
    
    // Shift reference point from Jan 1, 1970 (Unix) to Jan 1, 1978 (Amiga)
    t -= (8 * 365 + 2) * secPerDay - 60 * 60;
    
    days = t / secPerDay;
    mins = (t % secPerDay) / 60;
    ticks = (t % secPerDay % 60) * 50;
}

FSTime::FSTime(const u8 *p)
{
    assert(p != nullptr);
    
    days = FSBlock::read32(p);
    mins = FSBlock::read32(p + 4);
    ticks = FSBlock::read32(p + 8);
}

time_t
FSTime::time()
{
    const u32 secPerDay = 24 * 60 * 60;
    time_t t = days * secPerDay + mins * 60 + ticks / 50;
    
    // Shift reference point from  Jan 1, 1978 (Amiga) to Jan 1, 1970 (Unix)
    t += (8 * 365 + 2) * secPerDay - 60 * 60;
    
    return t;
}

void
FSTime::write(u8 *p)
{
    assert(p != nullptr);
    
    FSBlock::write32(p + 0, days);
    FSBlock::write32(p + 4, mins);
    FSBlock::write32(p + 8, ticks);
}

void
FSTime::print()
{
    time_t tt = time();
    tm *t = localtime(&tt);
    
    printf("%04d-%02d-%02d  ", 1900 + t->tm_year, 1 + t->tm_mon, t->tm_mday);
    printf("%02d:%02d:%02d  ", t->tm_hour, t->tm_min, t->tm_sec);
}
