// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"
#include "FSDevice.h"

FSBlock *
FSBlock::makeWithType(FSPartition &p, u32 nr, FSBlockType type)
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
    
    for (u32 i = 0; i < bsize(); i++) {
        
        if ((error = check(i, &expected, strict)) != ERROR_OK) {
            count++;
            debug(FS_DEBUG, "Block %d [%d.%d]: %s\n", nr, i / 4, i % 4,
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
    u32 loc = checksumLocation();
    assert(loc <= 5);
    
    // Wipe out the old checksum
    u32 old = get32(loc);
    set32(loc, 0);
    
    // Compute the new checksum
    u32 result = 0;
    for (u32 i = 0; i < bsize() / 4; i++) result += get32(i);
    result = ~result + 1;
    
    // Undo the modification
    set32(loc, old);
    
    return result;
}

void
FSBlock::updateChecksum()
{
    u32 ref = checksumLocation();
    if (ref < bsize() / 4) set32(ref, checksum());
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
    u32 ref = getParentDirRef();
    return ref ? partition.dev.blockPtr(ref) : nullptr;
}

FSFileHeaderBlock *
FSBlock::getFileHeaderBlock()
{
    u32 ref = getFileHeaderRef();
    return ref ? partition.dev.fileHeaderBlockPtr(ref) : nullptr;
}

FSBlock *
FSBlock::getNextHashBlock()
{
    u32 ref = getNextHashRef();
    return ref ? partition.dev.blockPtr(ref) : nullptr;
}

FSFileListBlock *
FSBlock::getNextListBlock()
{
    u32 ref = getNextListBlockRef();
    return ref ? partition.dev.fileListBlockPtr(ref) : nullptr;
}

FSBitmapExtBlock *
FSBlock::getNextBmExtBlock()
{
    u32 ref = getNextBmExtBlockRef();
    return ref ? partition.dev.bitmapExtBlockPtr(ref) : nullptr;
}


FSDataBlock *
FSBlock::getFirstDataBlock()
{
    u32 ref = getFirstDataBlockRef();
    return ref ? partition.dev.dataBlockPtr(ref) : nullptr;
}

FSDataBlock *
FSBlock::getNextDataBlock()
{
    u32 ref = getNextDataBlockRef();
    return ref ? partition.dev.dataBlockPtr(ref) : nullptr;
}

u32
FSBlock::getHashRef(u32 nr) const
{
    return (nr < hashTableSize()) ? get32(6 + nr) : 0;
}

void
FSBlock::setHashRef(u32 nr, u32 ref)
{
    if (nr < hashTableSize()) set32(6 + nr, ref);
}

void
FSBlock::dumpHashTable() const
{
    for (u32 i = 0; i < hashTableSize(); i++) {
        
        u32 value = read32(data + 24 + 4 * i);
        if (value) {
            msg("%d: %d ", i, value);
        }
    }
}

isize
FSBlock::getMaxDataBlockRefs() const
{
    return bsize() / 4 - 56;
}
