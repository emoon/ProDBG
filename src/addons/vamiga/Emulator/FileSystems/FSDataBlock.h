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

struct FSDataBlock : FSBlock {
      
    FSDataBlock(FSPartition &p, u32 nr);
    ~FSDataBlock();
    
    
    //
    // Methods from Block class
    //

    virtual u32 getDataBlockNr() const = 0;
    virtual void setDataBlockNr(u32 val) = 0;

    virtual u32  getDataBytesInBlock() const = 0;
    virtual void setDataBytesInBlock(u32 val) = 0;
    
    virtual isize writeData(FILE *file, isize size) = 0;
    
    
    //
    // Block specific methods
    //

    // Returns the number of data bytes stored in this block
    virtual isize dsize() const = 0;
};

struct OFSDataBlock : FSDataBlock {

    static u32 headerSize() { return 24; }

    OFSDataBlock(FSPartition &p, u32 nr);

    const char *getDescription() const override { return "OFSDataBlock"; }
    FSBlockType type() const override { return FS_DATA_BLOCK_OFS; }
    FSItemType itemType(isize byte) const override;
    void dump() const override;
    ErrorCode check(isize pos, u8 *expected, bool strict) const override;
    u32 checksumLocation() const override { return 5; }

    u32  getFileHeaderRef() const override          { return get32(1);        }
    void setFileHeaderRef(u32 ref) override         {        set32(1, ref);   }

    u32  getDataBlockNr() const override            { return get32(2);        }
    void setDataBlockNr(u32 val) override           {        set32(2, val);   }

    u32  getDataBytesInBlock() const override       { return get32(3);        }
    void setDataBytesInBlock(u32 val) override      {        set32(3, val);   }

    u32  getNextDataBlockRef() const override       { return get32(4);        }
    void setNextDataBlockRef(u32 ref) override      {        set32(4, ref);   }

    isize writeData(FILE *file, isize size) override;
    isize addData(const u8 *buffer, isize size) override;
    
    isize dsize() const override;
};

struct FFSDataBlock : FSDataBlock {
      
    static u32 headerSize() { return 0; }

    FFSDataBlock(FSPartition &p, u32 nr);

    const char *getDescription() const override { return "FFSDataBlock"; }
    FSBlockType type() const override { return FS_DATA_BLOCK_FFS; }
    FSItemType itemType(isize byte) const override;
    void dump() const override;

    u32 getDataBlockNr() const override { return 0; }
    void setDataBlockNr(u32 val) override { }

    u32  getDataBytesInBlock() const override { return 0; }
    void setDataBytesInBlock(u32 val) override { };

    isize writeData(FILE *file, isize size) override;
    isize addData(const u8 *buffer, isize size) override;
    
    isize dsize() const override;
};
