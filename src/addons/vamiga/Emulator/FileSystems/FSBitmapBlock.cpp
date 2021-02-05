// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"

FSBitmapBlock::FSBitmapBlock(FSPartition &p, u32 nr) : FSBlock(p, nr)
{
    data = new u8[p.dev.bsize]();
}

FSBitmapBlock::~FSBitmapBlock()
{
    delete [] data;
}

FSItemType
FSBitmapBlock::itemType(isize pos) const
{
    return pos < 4 ? FSI_CHECKSUM : FSI_BITMAP;
}

ErrorCode
FSBitmapBlock::check(isize byte, u8 *expected, bool strict) const
{
    isize word = byte / 4;
    u32 value = get32(word);
    
    if (word == 0) EXPECT_CHECKSUM;
    
    return ERROR_OK;
}

void
FSBitmapBlock::dump() const
{
    u32 count = 0;
    for (u32 i = 1; i < bsize() / 4; i++) {
        if (u32 value = get32(i)) {
            for (isize j = 0; j < 32; j++) {
                if (GET_BIT(value, j)) count++;
            }
        }
    }
    printf("         Free : %d blocks\n", count);
}
