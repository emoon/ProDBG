// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Parser.h"

namespace util {
    
bool
parseBool(string& token)
{
    if (token == "1" || token == "true" || token == "yes") return true;
    if (token == "0" || token == "false" || token == "no") return false;

    throw ParseBoolError("");
}

long
parseNum(string& token)
{
    long result;
    
    try { result = stol(token, nullptr, 0); }
    catch (std::exception& err) { throw ParseNumError(token); }

    return result;
}

}
