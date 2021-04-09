//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSBlock.h"
#include <vector>

struct FSRootBlock : FSBlock {
          
    FSRootBlock(FSPartition &p, Block nr);
    ~FSRootBlock();

    const char *getDescription() const override { return "FSRootBlock"; }

    
    //
    // Methods from Block class
    //

    void dump() const override;

    // Methods from Block class
    FSBlockType type() const override { return FS_ROOT_BLOCK; }
    FSItemType itemType(isize byte) const override;
    ErrorCode check(isize pos, u8 *expected, bool strict) const override;
    isize checksumLocation() const override { return 5; }
 
    Block getBmBlockRef(isize nr) const           { return get32(nr-49);       }
    void setBmBlockRef(isize nr, Block ref)       {        set32(nr-49, ref);  }

    Block getNextBmExtBlockRef() const override   { return get32(-24);         }
    void setNextBmExtBlockRef(Block ref) override {        set32(-24, ref);    }
    
    FSTime getModificationDate() const override   { return FSTime(addr32(-23));}
    void setModificationDate(FSTime t) override   { t.write(addr32(-23));      }

    FSName getName() const override               { return FSName(addr32(-20));}
    void setName(FSName name) override            { name.write(addr32(-20));   }

    FSTime getCreationDate() const override       { return FSTime(addr32(-7)); }
    void setCreationDate(FSTime t) override       { t.write(addr32(-7));       }

    isize hashTableSize() const override          { return 72;                 }

    bool addBitmapBlockRefs(std::vector<Block> &refs);
};
