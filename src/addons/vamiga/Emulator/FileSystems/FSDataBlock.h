// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_DATA_BLOCK_H
#define _FS_DATA_BLOCK_H

#include "FSBlock.h"

struct FSDataBlock : FSBlock {
      
    FSDataBlock(FSVolume &ref, u32 nr);
    ~FSDataBlock();
    
    FSBlockType type() override { return FS_DATA_BLOCK; }
};

struct OFSDataBlock : FSDataBlock {
      
    OFSDataBlock(FSVolume &ref, u32 nr, u32 cnt);

    void dump() override;
    bool check(bool verbose) override;
    void updateChecksum() override;

    void setFileHeaderRef(u32 ref) override     { write32(data + 4, ref);   }

    u32 getNextDataBlockRef() override          { return read32(data + 16); }
    void setNextDataBlockRef(u32 ref) override  { write32(data + 16, ref);  }

    size_t addData(const u8 *buffer, size_t size) override;
};

struct FFSDataBlock : FSDataBlock {
      
    FFSDataBlock(FSVolume &ref, u32 nr, u32 cnt);

    void dump() override;
    bool check(bool verbose) override;

    size_t addData(const u8 *buffer, size_t size) override;
};

#endif
