// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSBitmapBlock::FSBitmapBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
    dealloc();
}

FSBitmapBlock::~FSBitmapBlock()
{
    delete [] data;
}

void
FSBitmapBlock::dump()
{
    printf("   Allocated: ");

    for (u32 i = 0; i < volume.capacity; i++) {
        if (isAllocated(i)) printf("%d ", i);
    }
    
    printf("\n");
}

bool
FSBitmapBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    for (u32 i = 2; i < volume.capacity; i++) {
                
        FSBlockType type = volume.blocks[i]->type();

        if (type == FS_EMPTY_BLOCK && isAllocated(i)) {
            if (verbose) printf("Empty block %d is marked as allocated.\n", i);
            result = false;
        }
        if (type != FS_EMPTY_BLOCK && !isAllocated(i)) {
            if (verbose) printf("Non-empty block %d is marked as free.\n", i);
            result = false;
        }
    }

    return result;
}

void
FSBitmapBlock::locateBlockBit(u32 nr, u32 *byte, u32 *bit)
{
    // The first two blocks are not part the map (they are always allocated)
    assert(nr >= 2);
    nr -= 2;
    
    // Compute the location (the long word ordering of 'byte' is inversed)
    *bit = nr % 8;
    *byte = nr / 8;

    // Rectifiy the ordering
    switch (*byte % 4) {
        case 0: *byte += 3; break;
        case 1: *byte += 1; break;
        case 2: *byte -= 1; break;
        case 3: *byte -= 3; break;
    }

    assert(*byte <= bsize() - 4);
    assert(*bit < 8);
}

void
FSBitmapBlock::updateChecksum()
{
    set32(0, 0);
    set32(0, checksum());
}

bool
FSBitmapBlock::isAllocated(u32 block)
{
    // The first two blocks are always allocated
    if (block < 2) return true;
    
    // Consider non-existing blocks as allocated, too
    if (!volume.isBlockNumber(block)) return true;

    // Get the location of the allocation bit
    u32 byte, bit;
    locateBlockBit(block, &byte, &bit);

    // The block is allocated if the allocation bit is cleared
    return GET_BIT(data[byte + 4], bit) == 0;
}

void
FSBitmapBlock::alloc(u32 block, bool allocate)
{
    if (!volume.isBlockNumber(block)) return;

    u32 byte, bit;
    locateBlockBit(block, &byte, &bit);
    assert(byte <= bsize() - 4);
    assert(bit <= 7);
    
    // 0 = allocated, 1 = not allocated
    allocate ? CLR_BIT(data[4 + byte], bit) : SET_BIT(data[4 + byte], bit);
}

void
FSBitmapBlock::dealloc()
{
    // Mark all blocks except the first two as free
    for (u32 i = 2; i < volume.capacity; i++) dealloc(i);
}
