// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSBlock.h"
#include "FSBitmapBlock.h"
#include "FSBitmapExtBlock.h"
#include "FSBootBlock.h"
#include "FSDevice.h"
#include "FSDataBlock.h"
#include "FSEmptyBlock.h"
#include "FSFileHeaderBlock.h"
#include "FSFileListBlock.h"
#include "FSPartition.h"
#include "FSRootBlock.h"
#include "FSUserDirBlock.h"

FSBlock *
FSBlock::makeWithType(FSPartition &p, Block nr, FSBlockType type)
{
    switch (type) {

        case FS_EMPTY_BLOCK:      return new FSEmptyBlock(p, nr);
        case FS_BOOT_BLOCK:       return new FSBootBlock(p, nr);
        case FS_ROOT_BLOCK:       return new FSRootBlock(p, nr);
        case FS_BITMAP_BLOCK:     return new FSBitmapBlock(p, nr);
        case FS_BITMAP_EXT_BLOCK: return new FSBitmapExtBlock(p, nr);
        case FS_USERDIR_BLOCK:    return new FSUserDirBlock(p, nr);
        case FS_FILEHEADER_BLOCK: return new FSFileHeaderBlock(p, nr);
        case FS_FILELIST_BLOCK:   return new FSFileListBlock(p, nr);
        case FS_DATA_BLOCK_OFS:   return new OFSDataBlock(p, nr);
        case FS_DATA_BLOCK_FFS:   return new FFSDataBlock(p, nr);
            
        default:                  return nullptr;
    }
}

isize
FSBlock::bsize() const
{
    return partition.dev.bsize;
}

u32
FSBlock::typeID() const
{
    return get32(0);
}

u32
FSBlock::subtypeID() const
{
    return get32((bsize() / 4) - 1);
}

isize
FSBlock::check(bool strict) const
{
    ErrorCode error;
    isize count = 0;
    u8 expected;
    
    for (isize i = 0; i < bsize(); i++) {
        
        if ((error = check(i, &expected, strict)) != ERROR_OK) {
            count++;
            debug(FS_DEBUG, "Block %d [%zd.%zd]: %s\n", nr, i / 4, i % 4,
                  ErrorCodeEnum::key(error));
        }
    }
    
    return count;
}

u8 *
FSBlock::addr32(isize nr) const
{
    return (data + 4 * nr) + (nr < 0 ? bsize() : 0);
}

u32
FSBlock::read32(const u8 *p)
{
    return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}
    
void
FSBlock::write32(u8 *p, u32 value)
{
    p[0] = (value >> 24) & 0xFF;
    p[1] = (value >> 16) & 0xFF;
    p[2] = (value >>  8) & 0xFF;
    p[3] = (value >>  0) & 0xFF;
}

void
FSBlock::dumpData() const
{
    hexdumpLongwords(data, 512);
}

u32
FSBlock::checksum() const
{
    isize pos = checksumLocation();
    assert(pos >= 0 && pos <= 5);
    
    // Wipe out the old checksum
    u32 old = get32(pos);
    set32(pos, 0);
    
    // Compute the new checksum
    u32 result = 0;
    for (isize i = 0; i < bsize() / 4; i++) result += get32(i);
    result = ~result + 1;
    
    // Undo the modification
    set32(pos, old);
    
    return result;
}

void
FSBlock::updateChecksum()
{
    isize pos = checksumLocation();
    if (pos >= 0 && pos < bsize() / 4) set32(pos, checksum());
}

void
FSBlock::importBlock(const u8 *src, isize size)
{    
    assert(size == bsize());
    assert(src != nullptr);
    assert(data != nullptr);
        
    memcpy(data, src, size);
}

void
FSBlock::exportBlock(u8 *dst, isize size)
{
    assert(size == bsize());
            
    // Rectify the checksum
    updateChecksum();

    // Export the block
    assert(dst != nullptr);
    assert(data != nullptr);
    memcpy(dst, data, size);
}

FSBlock *
FSBlock::getParentDirBlock()
{
    Block nr = getParentDirRef();
    return nr ? partition.dev.blockPtr(nr) : nullptr;
}

FSFileHeaderBlock *
FSBlock::getFileHeaderBlock()
{
    Block nr = getFileHeaderRef();
    return nr ? partition.dev.fileHeaderBlockPtr(nr) : nullptr;
}

FSBlock *
FSBlock::getNextHashBlock()
{
    Block nr = getNextHashRef();
    return nr ? partition.dev.blockPtr(nr) : nullptr;
}

FSFileListBlock *
FSBlock::getNextListBlock()
{
    Block nr = getNextListBlockRef();
    return nr ? partition.dev.fileListBlockPtr(nr) : nullptr;
}

FSBitmapExtBlock *
FSBlock::getNextBmExtBlock()
{
    Block nr = getNextBmExtBlockRef();
    return nr ? partition.dev.bitmapExtBlockPtr(nr) : nullptr;
}


FSDataBlock *
FSBlock::getFirstDataBlock()
{
    Block nr = getFirstDataBlockRef();
    return nr ? partition.dev.dataBlockPtr(nr) : nullptr;
}

FSDataBlock *
FSBlock::getNextDataBlock()
{
    Block nr = getNextDataBlockRef();
    return nr ? partition.dev.dataBlockPtr(nr) : nullptr;
}

u32
FSBlock::getHashRef(Block nr) const
{
    return (nr < (Block)hashTableSize()) ? get32(6 + nr) : 0;
}

void
FSBlock::setHashRef(Block nr, u32 ref)
{
    if (nr < (Block)hashTableSize()) set32(6 + nr, ref);
}

void
FSBlock::dumpHashTable() const
{
    for (isize i = 0; i < hashTableSize(); i++) {
        
        u32 value = read32(data + 24 + 4 * i);
        if (value) {
            msg("%zd: %d ", i, value);
        }
    }
}

isize
FSBlock::getMaxDataBlockRefs() const
{
    return bsize() / 4 - 56;
}
