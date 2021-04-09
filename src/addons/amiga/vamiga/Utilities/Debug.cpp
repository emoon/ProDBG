// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Debug.h"

void hexdump(u8 *p, isize size, isize cols, isize pad)
{
    while (size) {
        
        isize cnt = std::min(size, cols);
        for (isize x = 0; x < cnt; x++) {
            fprintf(stderr, "%02X %s", p[x], ((x + 1) % pad) == 0 ? " " : "");
        }
        
        size -= cnt;
        p += cnt;
        
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
}

void hexdump(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, cols);
}

void hexdumpWords(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, 2);
}

void hexdumpLongwords(u8 *p, isize size, isize cols)
{
    hexdump(p, size, cols, 4);
}
