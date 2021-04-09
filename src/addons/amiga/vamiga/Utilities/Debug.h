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

#ifdef __cplusplus

// Prints a hex dump of a buffer to the console (for debugging)
void hexdump(u8 *p, isize size, isize cols, isize pad);
void hexdump(u8 *p, isize size, isize cols = 32);
void hexdumpWords(u8 *p, isize size, isize cols = 32);
void hexdumpLongwords(u8 *p, isize size, isize cols = 32);

#endif
