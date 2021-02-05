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

struct FSFileHeaderBlock : FSBlock {
                
    FSFileHeaderBlock(FSPartition &p, u32 nr);
    FSFileHeaderBlock(FSPartition &p, u32 nr, const char *name);

    const char *getDescription() const override { return "FSFileHeaderBlock"; }

    
    //
    // Methods from Block class
    //

    FSBlockType type() const override { return FS_FILEHEADER_BLOCK; }
    FSItemType itemType(isize byte) const override;
    ErrorCode check(isize pos, u8 *expected, bool strict) const override;
    void dump() const override;
    u32 checksumLocation() const override { return 5; }

    ErrorCode exportBlock(const char *path) override;

    isize getNumDataBlockRefs() const override  { return get32(2);            }
    void setNumDataBlockRefs(u32 val) override  {        set32(2, val);       }
    void incNumDataBlockRefs() override         {        inc32(2);            }

    u32 getFirstDataBlockRef() const override   { return get32(4     );       }
    void setFirstDataBlockRef(u32 ref) override {        set32(4, ref);       }
    
    u32 getDataBlockRef(isize nr) const         { return get32(-51-nr     );  }
    void setDataBlockRef(isize nr, u32 ref)     {        set32(-51-nr, ref);  }

    u32 getProtectionBits() const override      { return get32(-48     );     }
    void setProtectionBits(u32 val) override    {        set32(-48, val);     }

    u32 getFileSize() const override            { return get32(-47     );     }
    void setFileSize(u32 val) override          {        set32(-47, val);     }

    FSComment getComment() const override       { return FSComment(addr32(-46));}
    void setComment(FSComment name) override    { name.write(addr32(-46));      }

    FSTime getCreationDate() const override     { return FSTime(addr32(-23));   }
    void setCreationDate(FSTime t) override     { t.write(addr32(-23));         }

    FSName getName() const override             { return FSName(addr32(-20));   }
    void setName(FSName name) override          { name.write(addr32(-20));      }
    bool isNamed(FSName &other) const override  { return getName() == other;  }

    u32 getNextHashRef() const override         { return get32(-4     );      }
    void setNextHashRef(u32 ref) override       {        set32(-4, ref);      }

    u32 getParentDirRef() const override        { return get32(-3     );      }
    void setParentDirRef(u32 ref) override      {        set32(-3, ref);      }

    u32 getNextListBlockRef() const override    { return get32(-2     );      }
    void setNextListBlockRef(u32 ref) override  {        set32(-2, ref);      }

    
    //
    // Block specific methods
    //
    
    isize writeData(FILE *file);
    isize addData(const u8 *buffer, isize size) override;

    bool addDataBlockRef(u32 ref);
    bool addDataBlockRef(u32 first, u32 ref) override;
    
    u32 hashValue() const override { return getName().hashValue(); }
};
