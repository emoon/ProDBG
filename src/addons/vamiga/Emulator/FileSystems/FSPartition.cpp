
// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"
#include <algorithm>

FSPartition *
FSPartition::makeWithFormat(FSDevice &dev, FSPartitionDescriptor &layout)
{
    FSPartition *p = new FSPartition(dev);

    p->dos         = layout.dos;
    p->lowCyl      = layout.lowCyl;
    p->highCyl     = layout.highCyl;
    p->rootBlock   = layout.rootBlock;
    p->bmBlocks    = layout.bmBlocks;
    p->bmExtBlocks = layout.bmExtBlocks;
    
    p->firstBlock  = (u32)(p->lowCyl * dev.numHeads * dev.numSectors);
    p->lastBlock   = (u32)((p->highCyl + 1) * dev.numHeads * dev.numSectors - 1);
    
    // Do some consistency checking
    for (u32 i = p->firstBlock; i <= p->lastBlock; i++) assert(dev.blocks[i] == nullptr);
    
    // Create boot blocks
    dev.blocks[p->firstBlock]     = new FSBootBlock(*p, p->firstBlock);
    dev.blocks[p->firstBlock + 1] = new FSBootBlock(*p, p->firstBlock + 1);

    // Create the root block
    FSRootBlock *rb = new FSRootBlock(*p, p->rootBlock);
    dev.blocks[layout.rootBlock] = rb;
    
    // Create the bitmap blocks
    for (auto& ref : layout.bmBlocks) {
        
        dev.blocks[ref] = new FSBitmapBlock(*p, ref);
    }
    
    // Add bitmap extension blocks
    FSBlock *pred = rb;
    for (auto& ref : layout.bmExtBlocks) {
        
        dev.blocks[ref] = new FSBitmapExtBlock(*p, ref);
        pred->setNextBmExtBlockRef(ref);
        pred = dev.blocks[ref];
    }
    
    // Add all bitmap block references
    rb->addBitmapBlockRefs(layout.bmBlocks);
    
    // Add free blocks
    for (u32 i = p->firstBlock; i <= p->lastBlock; i++) {
        
        if (dev.blocks[i] == nullptr) {
            dev.blocks[i] = new FSEmptyBlock(*p, i);
            p->markAsFree(i);
        }
    }
    
    return p;
}

FSPartition::FSPartition(FSDevice &ref) : dev(ref)
{
    
}

void
FSPartition::info() const
{
    msg("DOS%lld  ",      dos);
    msg("%6zd (x %3zd) ", numBlocks(), bsize());
    msg("%6zd  ",         usedBlocks());
    msg("%6zd   ",        freeBlocks());
    msg("%3zd%%   ",      (isize)(100.0 * usedBlocks() / numBlocks()));
    msg("%s\n",           getName().c_str());
    msg("\n");
}

void
FSPartition::dump() const
{
    msg("      First cylinder : %zd\n", lowCyl);
    msg("       Last cylinder : %zd\n", highCyl);
    msg("         First block : %d\n", firstBlock);
    msg("          Last block : %d\n", lastBlock);
    msg("          Root block : %d\n", rootBlock);
    msg("       Bitmap blocks : ");
    for (auto& it : bmBlocks) { msg("%d ", it); }
    msg("\n");
    msg("Extension blocks : ");
    for (auto& it : bmExtBlocks) { msg("%d ", it); }
    msg("\n\n");
}

FSBlockType
FSPartition::predictBlockType(u32 nr, const u8 *buffer) const
{
    assert(buffer != nullptr);

    // Only proceed if the block belongs to this partition
    if (nr < firstBlock || nr > lastBlock) return FS_UNKNOWN_BLOCK;
    
    // Is it a boot block?
    if (nr == firstBlock + 0) return FS_BOOT_BLOCK;
    if (nr == firstBlock + 1) return FS_BOOT_BLOCK;
    
    // Is it a bitmap block?
    if (std::find(bmBlocks.begin(), bmBlocks.end(), nr) != bmBlocks.end())
        return FS_BITMAP_BLOCK;
    
    // is it a bitmap extension block?
    if (std::find(bmExtBlocks.begin(), bmExtBlocks.end(), nr) != bmExtBlocks.end())
        return FS_BITMAP_EXT_BLOCK;

    // For all other blocks, check the type and subtype fields
    u32 type = FSBlock::read32(buffer);
    u32 subtype = FSBlock::read32(buffer + bsize() - 4);

    if (type == 2  && subtype == 1)       return FS_ROOT_BLOCK;
    if (type == 2  && subtype == 2)       return FS_USERDIR_BLOCK;
    if (type == 2  && subtype == (u32)-3) return FS_FILEHEADER_BLOCK;
    if (type == 16 && subtype == (u32)-3) return FS_FILELIST_BLOCK;

    // Check if this block is a data block
    if (isOFS()) {
        if (type == 8) return FS_DATA_BLOCK_OFS;
    } else {
        for (u32 i = 0; i < bsize(); i++) if (buffer[i]) return FS_DATA_BLOCK_FFS;
    }
    
    return FS_EMPTY_BLOCK;
}

FSName
FSPartition::getName() const
{
    FSRootBlock *rb = dev.rootBlockPtr((u32)rootBlock);
    return rb ? rb->getName() : FSName("");
}

void
FSPartition::setName(FSName name)
{
    FSRootBlock *rb = dev.rootBlockPtr((u32)rootBlock);
    assert(rb != nullptr);

    rb->setName(name);
}

isize
FSPartition::bsize() const
{
    return dev.bsize;
}

isize
FSPartition::numBlocks() const
{
    return numCyls() * dev.numHeads * dev.numSectors;
}

isize
FSPartition::numBytes() const
{
    return numBlocks() * bsize();
}

isize
FSPartition::freeBlocks() const
{
    u32 result = 0;
    
    for (isize i = firstBlock; i <= lastBlock; i++) {
        if (isFree((u32)i)) result++;
    }

    return result;
}

isize
FSPartition::usedBlocks() const
{
    return numBlocks() - freeBlocks();
}

isize
FSPartition::freeBytes() const
{
    return freeBlocks() * bsize();
}

isize
FSPartition::usedBytes() const
{
    return usedBlocks() * bsize();
}

isize
FSPartition::requiredDataBlocks(isize fileSize) const
{
    // Compute the capacity of a single data block
    isize numBytes = bsize() - (isOFS() ? OFSDataBlock::headerSize() : 0);

    // Compute the required number of data blocks
    return (fileSize + numBytes - 1) / numBytes;
}

isize
FSPartition::requiredFileListBlocks(isize fileSize) const
{
    // Compute the required number of data blocks
    isize numBlocks = requiredDataBlocks(fileSize);
    
    // Compute the number of data block references in a single block
    isize numRefs = (bsize() / 4) - 56;

    // Small files do not require any file list block
    if (numBlocks <= numRefs) return 0;

    // Compute the required number of additional file list blocks
    return (numBlocks - 1) / numRefs;
}

isize
FSPartition::requiredBlocks(isize fileSize) const
{
    isize numDataBlocks = requiredDataBlocks(fileSize);
    isize numFileListBlocks = requiredFileListBlocks(fileSize);
    
    if (FS_DEBUG) {
        
        msg("Required file header blocks : %d\n",  1);
        msg("       Required data blocks : %zd\n", numDataBlocks);
        msg("  Required file list blocks : %zd\n", numFileListBlocks);
        msg("                Free blocks : %zd\n", freeBlocks());
    }
    
    return 1 + numDataBlocks + numFileListBlocks;
}
 
u32
FSPartition::allocateBlock()
{
    if (u32 ref = allocateBlockAbove((u32)rootBlock)) return ref;
    if (u32 ref = allocateBlockBelow((u32)rootBlock)) return ref;

    return 0;
}

u32
FSPartition::allocateBlockAbove(u32 ref)
{
    assert(ref >= firstBlock && ref <= lastBlock);
    
    for (isize i = firstBlock + 1; i <= lastBlock; i++) {
        if (dev.blocks[i]->type() == FS_EMPTY_BLOCK) {
            markAsAllocated((u32)i);
            return (u32)i;
        }
    }
    return 0;
}

u32
FSPartition::allocateBlockBelow(u32 ref)
{
    assert(ref >= firstBlock && ref <= lastBlock);
    
    for (isize i = (isize)ref - 1; i >= firstBlock; i--) {
        if (dev.blocks[i]->type() == FS_EMPTY_BLOCK) {
            markAsAllocated((u32)i);
            return (u32)i;
        }
    }
    return 0;
}

void
FSPartition::deallocateBlock(u32 ref)
{
    assert(ref >= firstBlock && ref <= lastBlock);
    assert(dev.blocks[ref]);
    
    delete dev.blocks[ref];
    dev.blocks[ref] = new FSEmptyBlock(*this, ref);
    markAsFree(ref);
}

u32
FSPartition::addFileListBlock(u32 head, u32 prev)
{
    FSBlock *prevBlock = dev.blockPtr(prev);
    if (!prevBlock) return 0;
    
    u32 ref = allocateBlock();
    if (!ref) return 0;
    
    dev.blocks[ref] = new FSFileListBlock(*this, ref);
    dev.blocks[ref]->setFileHeaderRef(head);
    prevBlock->setNextListBlockRef(ref);
    
    return ref;
}

u32
FSPartition::addDataBlock(u32 count, u32 head, u32 prev)
{
    FSBlock *prevBlock = dev.blockPtr(prev);
    if (!prevBlock) return 0;

    u32 ref = allocateBlock();
    if (!ref) return 0;

    FSDataBlock *newBlock;
    if (isOFS()) {
        newBlock = new OFSDataBlock(*this, ref);
    } else {
        newBlock = new FFSDataBlock(*this, ref);
    }
    
    dev.blocks[ref] = newBlock;
    newBlock->setDataBlockNr(count);
    newBlock->setFileHeaderRef(head);
    prevBlock->setNextDataBlockRef(ref);
    
    return ref;
}


FSUserDirBlock *
FSPartition::newUserDirBlock(const char *name)
{
    FSUserDirBlock *block = nullptr;
    
    if (u32 ref = allocateBlock()) {
    
        block = new FSUserDirBlock(*this, ref, name);
        dev.blocks[ref] = block;
    }
    
    return block;
}

FSFileHeaderBlock *
FSPartition::newFileHeaderBlock(const char *name)
{
    FSFileHeaderBlock *block = nullptr;
    
    if (u32 ref = allocateBlock()) {

        block = new FSFileHeaderBlock(*this, ref, name);
        dev.blocks[ref] = block;
    }
    
    return block;
}

FSBitmapBlock *
FSPartition::bmBlockForBlock(u32 relRef)
{
    assert(relRef >= 2 && relRef < numBlocks());
        
    // Locate the bitmap block
    isize bitsPerBlock = (bsize() - 4) * 8;
    isize nr = (relRef - 2) / bitsPerBlock;

    if (nr >= (isize)bmBlocks.size()) {
        warn("Allocation bit is located in non-existent bitmap block %zd\n", nr);
        return nullptr;
    }

    return dev.bitmapBlockPtr(bmBlocks[nr]);
}

bool
FSPartition::isFree(u32 ref) const
{
    assert(ref >= firstBlock && ref <= lastBlock);
    
    // Translate rel to a relative block index
    ref -= lowCyl;
    
    // The first two blocks are always allocated and not part of the bitmap
    if (ref < 2) return false;
    
    // Locate the allocation bit in the bitmap block
    u32 byte, bit;
    FSBitmapBlock *bm = locateAllocationBit(ref, &byte, &bit);
        
    // Read the bit
    return bm ? GET_BIT(bm->data[byte], bit) : false;
}

void
FSPartition::setAllocationBit(u32 ref, bool value)
{
    u32 byte, bit;
    
    if (FSBitmapBlock *bm = locateAllocationBit(ref, &byte, &bit)) {
        REPLACE_BIT(bm->data[byte], bit, value);
    }
}

FSBitmapBlock *
FSPartition::locateAllocationBit(u32 ref, u32 *byte, u32 *bit) const
{
    assert(ref >= firstBlock && ref <= lastBlock);

    // Make ref a relative offset
    ref -= firstBlock;

    // The first two blocks are always allocated and not part of the map
    if (ref < 2) return nullptr;
    ref -= 2;
    
    // Locate the bitmap block which stores the allocation bit
    isize bitsPerBlock = (bsize() - 4) * 8;
    isize nr = ref / bitsPerBlock;
    ref = ref % bitsPerBlock;

    // Get the bitmap block
    FSBitmapBlock *bm;
    bm = (nr < (isize)bmBlocks.size()) ? dev.bitmapBlockPtr(bmBlocks[nr]) : nullptr;
    if (bm == nullptr) {
        warn("Allocation bit is located in non-existent bitmap block %zd\n", nr);
        return nullptr;
    }
    
    // Locate the byte position (note: the long word ordering will be reversed)
    u32 rByte = ref / 8;
    
    // Rectifiy the ordering
    switch (rByte % 4) {
        case 0: rByte += 3; break;
        case 1: rByte += 1; break;
        case 2: rByte -= 1; break;
        case 3: rByte -= 3; break;
    }

    // Skip the checksum which is located in the first four bytes
    rByte += 4;
    assert(rByte >= 4 && rByte < bsize());
    
    *byte = rByte;
    *bit = (ref - 2) % 8;
    
    // debug(FS_DEBUG, "Alloc bit for %d: block: %d byte: %d bit: %d\n",
    //       ref, bm->nr, *byte, *bit);

    return bm;
}

void
FSPartition::makeBootable(long bootBlockID)
{
    assert(dev.blocks[firstBlock + 0]->type() == FS_BOOT_BLOCK);
    assert(dev.blocks[firstBlock + 1]->type() == FS_BOOT_BLOCK);

    ((FSBootBlock *)dev.blocks[firstBlock + 0])->writeBootBlock(bootBlockID, 0);
    ((FSBootBlock *)dev.blocks[firstBlock + 1])->writeBootBlock(bootBlockID, 1);
}

void
FSPartition::killVirus()
{
    assert(dev.blocks[firstBlock + 0]->type() == FS_BOOT_BLOCK);
    assert(dev.blocks[firstBlock + 1]->type() == FS_BOOT_BLOCK);

    long bootBlockID = isOFS() ? 0 : isFFS() ? 1 : -1;

    if (bootBlockID != -1) {
        ((FSBootBlock *)dev.blocks[firstBlock + 0])->writeBootBlock(bootBlockID, 0);
        ((FSBootBlock *)dev.blocks[firstBlock + 1])->writeBootBlock(bootBlockID, 1);
    } else {
        memset(dev.blocks[firstBlock + 0]->data + 4, 0, bsize() - 4);
        memset(dev.blocks[firstBlock + 1]->data, 0, bsize());
    }
}

bool
FSPartition::check(bool strict, FSErrorReport &report) const
{
    report.bitmapErrors = 0;
    
    for (isize i = firstBlock; i <= lastBlock; i++) {

        FSBlock *block = dev.blocks[i];
        if (block->type() == FS_EMPTY_BLOCK && !isFree((u32)i)) {
            report.bitmapErrors++;
            debug(FS_DEBUG, "Empty block %zd is marked as allocated\n", i);
        }
        if (block->type() != FS_EMPTY_BLOCK && isFree((u32)i)) {
            report.bitmapErrors++;
            debug(FS_DEBUG, "Non-empty block %zd is marked as free\n", i);
        }
    }
 
    return report.bitmapErrors == 0;
}
