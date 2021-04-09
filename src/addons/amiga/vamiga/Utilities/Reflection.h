// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#ifdef __cplusplus

#include "Types.h"
#include <map>

namespace util {

#define assert_enum(e,v) assert(e##Enum::isValid(v))

template <class T, typename E> struct Reflection {

    // Returns the shortened key as a C string
    static const char *key(long nr) { return T::key((E)nr); }

    // Collects all key / value pairs
    static std::map <string, long> pairs(long min = 1) {
        
        std::map <string,long> result;
                
        for (isize i = 0;; i++) {
            if (T::isValid(i)) {
                result.insert(std::make_pair(key(i), i));
            } else {
                if (i >= min) break;
            }
        }
        
        return result;
    }

    // Returns a list in form of a colon seperated string
    static string keyList(bool prefix = false) {
        
        string result;
        
        auto p = pairs();
        for(auto it = std::begin(p); it != std::end(p); ++it) {
            if (it != std::begin(p)) result += ", ";
            if (prefix && T::prefix()) result += T::prefix();
            result += it->first;
        }
        
        return result;
    }
};

}

#endif
