// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include <stdio.h>

#define assert_enum(e,v) assert(e##Enum::isValid(v))

template <class T, typename E> struct Reflection {

    // Returns the shortened key as a C string
    static const char *key(long nr) { return T::key((E)nr); }
    
    // Verifies a key (used by the configuration methods)
    static bool verify(long nr, long min = 1) {
        
        if (T::isValid(nr)) return true;
        
        printf("ERROR: %ld doesn't specify a valid key.\nValid keys: ", nr);
        
        for (long i = 0, j = 0 ;; i++) {
            
            if (T::isValid(i)) {
                
                if (j++) printf(", ");
                if (T::prefix()) printf("%s_", T::prefix());
                printf("%s", key(i));
           
            } else if (i >= min) break;
        }
        
        printf("\n");
        return false;
    }
};
