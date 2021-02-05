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

struct FSBitmapExtBlock : FSBlock {
                    
    FSBitmapExtBlock(FSPartition &p, u32 nr);
    ~FSBitmapExtBlock();
     
    const char *getDescription() const override { return "FSBitmapExtBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() const override { return FS_BITMAP_EXT_BLOCK; }
    FSItemType itemType(isize byte) const override;
    ErrorCode check(isize pos, u8 *expected, bool strict) const override;
    void dump() const override;

    u32 getNextBmExtBlockRef() const override    { return get32(-1);         }
    void setNextBmExtBlockRef(u32 ref) override  {        set32(-1, ref);    }

    u32 getBmBlockRef(isize nr) const            { return get32(nr     );    }
    void setBmBlockRef(isize nr, u32 ref)        {        set32(nr, ref);    }

    void addBitmapBlockRefs(vector<u32> &refs, std::vector<u32>::iterator &it);
};
