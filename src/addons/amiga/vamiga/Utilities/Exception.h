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
#include <exception>

namespace util {

struct Exception : public std::exception {
    
    string description;
    i64 data;
    
    Exception(const string &s, i64 d) : description(s), data(d) { }
    Exception(const string &s) : description(s), data(0) { }
    Exception(i64 d) : description(""), data(d) { }

    const char *what() const throw() override { return description.c_str(); }
};

//
// Syntactic sugar
//

/* The following keyword is used for documentary purposes only. It is used to
 * mark all methods that use the exception mechanism to signal error conditions
 * instead of returning error codes. It is used in favor of classic throw
 * lists, since the latter cause the compiler to embed unwanted runtime checks
 * in the code.
 */
#define throws

}
