// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaPublicTypes.h"
#include "Utils.h"

#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <thread>

using std::vector;
using std::map;
using std::queue;
using std::pair;
using std::swap;
// using std::string;

/* Base class for all Amiga objects. This class adds a textual description
 * the object together with functions for printing debug messages and warnings.
 */
class AmigaObject {
    
public:
    
    virtual ~AmigaObject() { };
        
    
    //
    // Initializing
    //
    
public:
    
    // Returns the name for this component (e.g., "Agnus" or "Denise")
    virtual const char *getDescription() const { return ""; }
    
    // Called by debug() and trace() to produce a detailed debug output
    virtual void prefix() const { };
};
