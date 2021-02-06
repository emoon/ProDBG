// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_OBJECT_H
#define _AMIGA_OBJECT_H

#include "AmigaTypes.h"
#include "Utils.h"
#include "Concurrency.h"

#include <vector>
#include <map>
#include <mutex>
#include <queue>
#include <thread>

using std::vector;
using std::map;
using std::pair;
using std::swap;

#define synchronized \
for (AutoMutex _am(mutex); _am.active; _am.active = false)

/* Base class for all Amiga objects. This class contains a textual description
 * of the object and offers various functions for printing debug messages and
 * warnings.
 */
class AmigaObject {
    
public:
    
    virtual ~AmigaObject() { };
    
private:
    
    /* Textual description of this object. Most debug output methods preceed
     * their output with this string. If set to NULL, no prefix is printed.
     */
    const char *description = NULL;
    
protected:
    
    /* Mutex for implementing the 'synchronized' macro. The macro can be used
     * to prevent multiple threads to enter the same code block. It mimics the
     * behaviour of the well known Java construct 'synchronized(this) { }'.
     */
    Mutex mutex;

    
    //
    // Initializing
    //
    
public:
    
    const char *getDescription() const { return description ? description : ""; }
    void setDescription(const char *str) { description = strdup(str); }
    

    //
    // Debugging the component
    //
    
protected:
    
    /* There a four types of messages:
     *
     *   - msg    Information messages  (Show up in all builds)
     *   - warn   Warning messages      (Show up in all builds)
     *   - debug  Debug messages        (Show up in debug builds, only)
     *   - trace  Detailed debug output (Show up in debug builds, only)
     *
     * Debug messages are prefixed by the name of the component producing it.
     * Trace messages are prefixed by the string description produced by the
     * prefix() function. Some objects overwrite prefix() to output additional
     * debug information.
     *
     * All functions can be called with an optional 'verbose' parameter. If a
     * 0 is passed in, no output will be created. This parameter is mainly used
     * in combination with debug and trace messages.
     */
    virtual void prefix() { };
    
    void msg(const char *fmt, ...);
    void msg(int verbose, const char *fmt, ...);

    void warn(const char *fmt, ...);
    void warn(int verbose, const char *fmt, ...);

    void debug(const char *fmt, ...);
    void debug(int verbose, const char *fmt, ...);

    void trace(const char *fmt, ...);
    void trace(int verbose, const char *fmt, ...);
};

#endif
