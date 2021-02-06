// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_FILEHEADER_BLOCK_H
#define _FS_FILEHEADER_BLOCK_H

#include "FSBlock.h"

struct FSFileHeaderBlock : FSBlock {
                
    FSFileHeaderBlock(FSVolume &ref, u32 nr);
    FSFileHeaderBlock(FSVolume &ref, u32 nr, const char *name);

    FSBlockType type() override { return FS_FILEHEADER_BLOCK; }
    void dump() override;
    bool check(bool verbose) override;
    void updateChecksum() override;

    bool matches(FSName &otherName) override { return getName() == otherName; }

    //
    // Block items
    //
    
    FSName getName() override;
    void setName(FSName name) override;

    FSComment getComment() override;
    void setComment(FSComment name) override;

    FSTime getCreationDate() override           { return FSTime(addr(-23));  }
    void setCreationDate(FSTime t) override     { t.write(addr(-23));        }

    u32 maxDataBlockRefs() override             { return bsize() / 4 - 56;   }
    u32 numDataBlockRefs() override             { return get32(2);           }
    void incDataBlockRefs() override            {        inc32(2);           }

    u32 getFirstDataBlockRef() override         { return get32(4     );      }
    void setFirstDataBlockRef(u32 ref) override {        set32(4, ref);      }

    u32 getDataBlockRef(int nr)                 { return get32(-51-nr     ); }
    void setDataBlockRef(int nr, u32 ref)       {        set32(-51-nr, ref); }

    u32 getProtectionBits() override            { return get32(-48     );    }
    void setProtectionBits(u32 val) override    {        set32(-48, val);    }

    u32 getFileSize() override                  { return get32(-47     );    }
    void setFileSize(u32 val) override          {        set32(-47, val);    }

    u32 getNextHashRef() override               { return get32(-4     );     }
    void setNextHashRef(u32 ref) override       {        set32(-4, ref);     }

    u32 getParentDirRef() override              { return get32(-3     );     }
    void setParentDirRef(u32 ref) override      {        set32(-3, ref);     }

    u32 getNextListBlockRef() override          { return get32(-2     );     }
    void setNextListBlockRef(u32 ref) override  {        set32(-2, ref);     }

    bool addDataBlockRef(u32 ref) override;
    bool addDataBlockRef(u32 first, u32 ref) override;

    size_t addData(const u8 *buffer, size_t size) override;

    u32 hashValue() override { return getName().hashValue(); }
};

#endif
