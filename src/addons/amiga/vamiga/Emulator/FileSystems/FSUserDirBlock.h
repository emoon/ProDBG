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

struct FSUserDirBlock : FSBlock {
                
    FSUserDirBlock(FSPartition &p, Block nr);
    FSUserDirBlock(FSPartition &p, Block nr, const char *name);
    ~FSUserDirBlock();
    
    const char *getDescription() const override { return "FSUserDirBlock"; }

    
    //
    // Methods from Block class
    //
    
    FSBlockType type() const override  { return FS_USERDIR_BLOCK; }
    FSItemType itemType(isize byte) const override;
    ErrorCode check(isize pos, u8 *expected, bool strict) const override;
    void dump() const override;
    isize checksumLocation() const override { return 5; }

    ErrorCode exportBlock(const char *path) override;

    u32 getProtectionBits() const override     { return get32(-48     );       }
    void setProtectionBits(u32 val) override   {        set32(-48, val);       }

    FSComment getComment() const override      { return FSComment(addr32(-46)); }
    void setComment(FSComment name) override   { name.write(addr32(-46));      }

    FSTime getCreationDate() const override    { return FSTime(addr32(-23));   }
    void setCreationDate(FSTime t) override    { t.write(addr32(-23));         }

    FSName getName() const override            { return FSName(addr32(-20));   }
    void setName(FSName name) override         { name.write(addr32(-20));      }
    bool isNamed(FSName &other) const override { return getName() == other;    }

    Block getNextHashRef() const override      { return get32(-4     );        }
    void setNextHashRef(Block ref) override    {        set32(-4, ref);        }

    Block getParentDirRef() const override     { return get32(-3      );       }
    void setParentDirRef(Block ref) override   {        set32(-3,  ref);       }

    isize hashTableSize() const override       { return 72; }
    u32 hashValue() const override             { return getName().hashValue(); }
};
