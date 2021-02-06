// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_BITMAP_BLOCK_H
#define _FS_BITMAP_BLOCK_H

#include "FSBlock.h"

struct FSBitmapBlock : FSBlock {
                    
    FSBitmapBlock(FSVolume &ref, u32 nr);
    ~FSBitmapBlock();
     
    FSBlockType type() override { return FS_BITMAP_BLOCK; }
    void dump() override;
    bool check(bool verbose) override;
    // void exportBlock(u8 *p, size_t size) override;
    void updateChecksum() override;

    // Computes location of the allocation bit for a certain block
    void locateBlockBit(u32 nr, u32 *byte, u32 *bit);
    
    // Checks whether a block is allocated
    bool isAllocated(u32 block);

    // Allocates or deallocates a single block
    void alloc(u32 block, bool allocate);
    void alloc(u32 block) { alloc(block, true); }
    void dealloc(u32 block) { alloc(block, false); }
    void dealloc();
};

#endif
