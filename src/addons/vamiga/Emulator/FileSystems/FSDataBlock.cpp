// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSDataBlock::FSDataBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
}

FSDataBlock::~FSDataBlock()
{
    delete [] data;
}


//
// Original File System (OFS)
//

OFSDataBlock::OFSDataBlock(FSVolume &ref, u32 nr, u32 cnt) : FSDataBlock(ref, nr)
{
    data = new u8[ref.bsize]();
    
    set32(0, 8);    // Block type
    set32(2, cnt);  // Position in block sequence (numbering starts with 1)
}

void
OFSDataBlock::dump()
{    
}

bool
OFSDataBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    /*
    if (blockNumber < 1) {
        
        if (verbose) fprintf(stderr, "Block index %d is smaller than 1\n", blockNumber);
        return false;
    }
    */
    /*
    result &= assertNotNull(fileHeaderBlock, verbose);
    result &= assertInRange(fileHeaderBlock, verbose);
    result &= assertInRange(blockNumber, verbose);
    result &= assertInRange(next, verbose);
    */
    
    return result;
}

void
OFSDataBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
}

size_t
OFSDataBlock::addData(const u8 *buffer, size_t size)
{
    size_t headerSize = 24;
    size_t count = MIN(volume.bsize - headerSize, size);

    memcpy(data + headerSize, buffer, count);

    // Store the number of written bytes in the block header
    write32(data + 12, count);
    
    return count;
}


//
// Fast File System (FFS)
//

FFSDataBlock::FFSDataBlock(FSVolume &ref, u32 nr, u32 cnt) : FSDataBlock(ref, nr)
{
}

void
FFSDataBlock::dump()
{
}

bool
FFSDataBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    return result;
}

size_t
FFSDataBlock::addData(const u8 *buffer, size_t size)
{
    size_t count = MIN(volume.bsize, size);
    memcpy(data, buffer, count);
    return count;
}
