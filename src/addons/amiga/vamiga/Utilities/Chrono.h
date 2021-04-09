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

class Time {
    
public:
    
    i64 ticks = 0;

public:
    
    static Time now();
    
    Time() { };
    Time(i64 value) : ticks(value) { };
    
    i64 asNanoseconds()  const { return ticks; }
    i64 asMicroseconds() const { return ticks / 1000; }
    i64 asMilliseconds() const { return ticks / 1000000; }
    float asSeconds()    const { return ticks / 1000000000.f; }
    
    bool operator==(const Time &rhs) const;
    bool operator!=(const Time &rhs) const;
    bool operator<=(const Time &rhs) const;
    bool operator>=(const Time &rhs) const;
    bool operator<(const Time &rhs) const;
    bool operator>(const Time &rhs) const;
    Time operator+(const Time &rhs) const;
    Time operator-(const Time &rhs) const;
    Time operator*(const int i) const;
    Time& operator+=(const Time &rhs);
    Time& operator-=(const Time &rhs);
    Time& operator*=(const int i);
    Time abs() const;
    Time diff() const;
    
    void sleep();
    void sleepUntil();
};

class Clock {
        
    Time start;
    Time elapsed;
    
    bool paused = false;

    void updateElapsed();

public:
    
    Clock();

    Time getElapsedTime();
    
    Time stop();
    Time go();
    Time restart();
};

}
