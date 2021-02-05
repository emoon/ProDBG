// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSBlock.h"

struct FSBitmapBlock : FSBlock {
                    
    FSBitmapBlock(FSPartition &p, u32 nr);
    ~FSBitmapBlock();
     
    const char *getDescription() const override { return "FSBitmapBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() const override { return FS_BITMAP_BLOCK; }
    FSItemType itemType(isize byte) const override;
    ErrorCode check(isize pos, u8 *expected, bool strict) const override;
    void dump() const override;
    u32 checksumLocation() const override { return 0; }
};
