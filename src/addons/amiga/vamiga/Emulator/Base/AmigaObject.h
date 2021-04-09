// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Debug.h"
#include "Error.h"

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

/* This file provides several macros for printing messages:
 *
 *   - msg    Information message   (Shows up in all builds)
 *   - warn   Warning message       (Shows up in all builds)
 *   - debug  Debug message         (Shows up in debug builds, only)
 *   - plain  Plain debug message   (Shows up in debug builds, only)
 *   - trace  Detailed debug output (Shows up in debug builds, only)
 *
 * Debug messages are prefixed by the component name and a line number. Trace
 * messages are prefixed by a more detailed string description produced by the
 * prefix() function.
 *
 * Debug, plain, and trace messages are accompanied by an optional 'verbose'
 * parameter. If 0 is passed in, no output will be generated.
 *
 * Sidenote: In previous releases the printing macros were implemented in form
 * of variadic functions. Although this might seem to be superior at first
 * glance, it is not. Using macros allows modern compilers to verify the format
 * strings against the data types of the provided arguments. This check can't
 * be performed when variadic functions are used.
 */

#define msg(format, ...) \
fprintf(stderr, format, ##__VA_ARGS__);

#define warn(format, ...) \
fprintf(stderr, "Warning: " format, ##__VA_ARGS__);

#ifndef NDEBUG

#define debug(verbose, format, ...) \
if (verbose) { \
fprintf(stderr, "%s:%d " format, getDescription(), __LINE__, ##__VA_ARGS__); }

#define plain(verbose, format, ...) \
if (verbose) { \
fprintf(stderr, format, ##__VA_ARGS__); }

#define trace(verbose, format, ...) \
if (verbose) { \
prefix(); \
fprintf(stderr, "%s:%d " format, getDescription(), __LINE__, ##__VA_ARGS__); }

#else

#define debug(verbose, format, ...)
#define plain(verbose, format, ...)
#define trace(verbose, format, ...)

#endif
