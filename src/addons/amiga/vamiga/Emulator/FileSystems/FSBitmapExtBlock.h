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
#include <vector>

struct FSBitmapExtBlock : FSBlock {
                    
    FSBitmapExtBlock(FSPartition &p, Block nr);
    ~FSBitmapExtBlock();
     
    const char *getDescription() const override { return "FSBitmapExtBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() const override { return FS_BITMAP_EXT_BLOCK; }
    FSItemType itemType(isize byte) const override;
    ErrorCode check(isize pos, u8 *expected, bool strict) const override;
    void dump() const override;

    Block getNextBmExtBlockRef() const override   { return get32(-1);         }
    void setNextBmExtBlockRef(Block ref) override {        set32(-1, ref);    }

    Block getBmBlockRef(isize nr) const           { return get32(nr     );    }
    void setBmBlockRef(isize nr, Block ref)       {        set32(nr, ref);    }

    void addBitmapBlockRefs(std::vector<Block> &refs, std::vector<Block>::iterator &it);
};
