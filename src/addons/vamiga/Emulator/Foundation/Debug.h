// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

/* This file provides several macros for printing messages:
 *
 *   - msg    Information messages  (Show up in all builds)
 *   - warn   Warning messages      (Show up in all builds)
 *   - debug  Debug messages        (Show up in debug builds, only)
 *   - plain  Plain debug messages  (Show up in debug builds, only)
 *   - trace  Detailed debug output (Show up in debug builds, only)
 *
 * Debug messages are prefixed by the component name and a line number. Trace
 * messages are prefixed by a more detailed string description produced by the
 * prefix() function.
 *
 * Debug, plain, and trace messages are accompanied by an optional 'verbose'
 * parameter. If 0 is passed in, no output will be generated.
 *
 * Sidenote: In previous releases the printing macros were implemented in form
 * of variadic functions. Although this might seem to be a superior approach at
 * first glance, it is not. Using macros allows modern compilers to verify the
 * format string placeholders against the data types of the provided arguments.
 * This check can't be performed when using variadic functions are utilized.
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
#define trace(verbose, format, ...)

#endif
