// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"

FSBitmapExtBlock::FSBitmapExtBlock(FSPartition &p, u32 nr) : FSBlock(p, nr)
{
    data = new u8[p.dev.bsize]();
}

FSBitmapExtBlock::~FSBitmapExtBlock()
{
    delete [] data;
}

FSItemType
FSBitmapExtBlock::itemType(isize pos) const
{
    return pos < (bsize() - 4) ? FSI_BITMAP : FSI_BITMAP_EXT_BLOCK_REF;
}

ErrorCode
FSBitmapExtBlock::check(isize byte, u8 *expected, bool strict) const
{
    isize word = byte / 4;
    u32 value = get32(word);
    
    if (word == (i32)(bsize() - 4)) EXPECT_OPTIONAL_BITMAP_EXT_REF;
    
    return ERROR_OK;
}

void
FSBitmapExtBlock::dump() const
{
    msg("Bitmap blocks : ");
    for (u32 i = 0; i < (bsize() / 4) - 1; i++) {
        if (u32 ref = getBmBlockRef(i)) msg("%d ", ref);
    }
    msg("\n");
    msg("         Next : %d\n", getNextBmExtBlockRef());
}

void
FSBitmapExtBlock::addBitmapBlockRefs(vector<u32> &refs, std::vector<u32>::iterator &it)
{
    isize max = (bsize() / 4) - 1;
    
    for (isize i = 0; i < max; i++, it++) {
        if (it == refs.end()) return;
        setBmBlockRef(i, *it);
    }
}
