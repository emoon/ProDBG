// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"

FSFileHeaderBlock::FSFileHeaderBlock(FSPartition &p, u32 nr) : FSBlock(p, nr)
{
    data = new u8[p.dev.bsize]();
   
    // Setup constant values
    
    set32(0, 2);                     // Type
    set32(1, nr);                    // Block pointer to itself
    setCreationDate(time(nullptr));  // Creation date
    set32(-1, (u32)-3);              // Sub type
}

FSFileHeaderBlock::FSFileHeaderBlock(FSPartition &p, u32 nr, const char *name) :
FSFileHeaderBlock(p, nr)
{
    setName(FSName(name));
}

FSItemType
FSFileHeaderBlock::itemType(isize byte) const
{
    // Intercept some special locations
    if (byte == 328) return FSI_BCPL_STRING_LENGTH;
    if (byte == 432) return FSI_BCPL_STRING_LENGTH;

    // Translate the byte index to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;

    switch (word) {
        case 0:   return FSI_TYPE_ID;
        case 1:   return FSI_SELF_REF;
        case 2:   return FSI_DATA_BLOCK_REF_COUNT;
        case 3:   return FSI_UNUSED;
        case 4:   return FSI_FIRST_DATA_BLOCK_REF;
        case 5:   return FSI_CHECKSUM;
        case -50:
        case -49: return FSI_UNUSED;
        case -48: return FSI_PROT_BITS;
        case -47: return FSI_FILESIZE;
        case -23: return FSI_CREATED_DAY;
        case -22: return FSI_CREATED_MIN;
        case -21: return FSI_CREATED_TICKS;
        case -4:  return FSI_NEXT_HASH_REF;
        case -3:  return FSI_PARENT_DIR_REF;
        case -2:  return FSI_EXT_BLOCK_REF;
        case -1:  return FSI_SUBTYPE_ID;
    }
    
    if (word <= -51)                return FSI_DATA_BLOCK_REF;
    if (word >= -46 && word <= -24) return FSI_BCPL_COMMENT;
    if (word >= -20 && word <= -5)  return FSI_BCPL_FILE_NAME;

    assert(false);
    return FSI_UNKNOWN;
}

ErrorCode
FSFileHeaderBlock::check(isize byte, u8 *expected, bool strict) const
{
    /* Note: At locations -4 and -3, many disks reference the bitmap block
     * which is wrong. We ignore to report this common inconsistency if
     * 'strict' is set to false.
     */

    // Translate the byte index to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
    u32 value = get32(word);
    
    switch (word) {
        case   0: EXPECT_LONGWORD(2);                    break;
        case   1: EXPECT_SELFREF;                        break;
        case   3: EXPECT_BYTE(0);                        break;
        case   4: EXPECT_DATABLOCK_REF;                  break;
        case   5: EXPECT_CHECKSUM;                       break;
        case -50: EXPECT_BYTE(0);                        break;
        case  -4: if (strict) EXPECT_OPTIONAL_HASH_REF;  break;
        case  -3: if (strict) EXPECT_PARENT_DIR_REF;     break;
        case  -2: EXPECT_OPTIONAL_FILELIST_REF;          break;
        case  -1: EXPECT_LONGWORD(-3);                   break;
    }
        
    // Data block reference area
    if (word <= -51 && value) EXPECT_DATABLOCK_REF;
    if (word == -51) {
        if (value == 0 && getNumDataBlockRefs() > 0) {
            return ERROR_FS_EXPECTED_REF;
        }
        if (value != 0 && getNumDataBlockRefs() == 0) {
            return ERROR_FS_EXPECTED_NO_REF;
        }
    }
    
    return ERROR_OK;
}

void
FSFileHeaderBlock::dump() const
{
    msg("           Name : %s\n", getName().c_str());
    msg("        Comment : %s\n", getComment().c_str());
    msg("        Created : %s\n", getCreationDate().str().c_str());
    msg("           Next : %d\n", getNextHashRef());
    msg("      File size : %d\n", getFileSize());

    msg("    Block count : %zd / %zd\n", getNumDataBlockRefs(), getMaxDataBlockRefs());
    msg("          First : %d\n", getFirstDataBlockRef());
    msg("     Parent dir : %d\n", getParentDirRef());
    msg(" FileList block : %d\n", getNextListBlockRef());
    
    msg("    Data blocks : ");
    for (isize i = 0; i < getNumDataBlockRefs(); i++) msg("%d ", getDataBlockRef(i));
    msg("\n");
}

ErrorCode
FSFileHeaderBlock::exportBlock(const char *exportDir)
{
    string path = exportDir;
    path += "/" + partition.dev.getPath(this);

    printf("Creating file %s\n", path.c_str());
    
    FILE *file = fopen(path.c_str(), "w");
    if (file == nullptr) return ERROR_FS_CANNOT_CREATE_FILE;
    
    writeData(file);
    fclose(file);
        
    return ERROR_OK;
}

isize
FSFileHeaderBlock::writeData(FILE *file)
{
    long bytesRemaining = getFileSize();
    long bytesTotal = 0;
    long blocksTotal = 0;

    // Start here and iterate through all connected file list blocks
    FSBlock *block = this;

    while (block && blocksTotal < partition.numBlocks()) {

        blocksTotal++;

        // Iterate through all data blocks references in this block
        isize num = MIN(block->getNumDataBlockRefs(), block->getMaxDataBlockRefs());
        for (isize i = 0; i < num; i++) {
            
            u32 ref = getDataBlockRef(i);
            if (FSDataBlock *dataBlock = partition.dev.dataBlockPtr(getDataBlockRef(i))) {

                long bytesWritten = dataBlock->writeData(file, bytesRemaining);
                bytesTotal += bytesWritten;
                bytesRemaining -= bytesWritten;
                
            } else {
                
                warn("Ignoring block %d (no data block)\n", ref);
            }
        }
        
        // Continue with the next list block
        block = block->getNextListBlock();
    }
    
    if (bytesRemaining != 0) {
        warn("%ld remaining bytes. Expected 0.\n", bytesRemaining);
    }
    
    return bytesTotal;
}

isize
FSFileHeaderBlock::addData(const u8 *buffer, isize size)
{
    assert(getFileSize() == 0);
        
    // Compute the required number of blocks
    isize numDataBlocks = partition.requiredDataBlocks(size);
    isize numListBlocks = partition.requiredFileListBlocks(size);
    
    debug(FS_DEBUG, "Required data blocks : %zd\n", numDataBlocks);
    debug(FS_DEBUG, "Required list blocks : %zd\n", numListBlocks);
    debug(FS_DEBUG, "         Free blocks : %zd\n", partition.freeBlocks());
    
    if (partition.freeBlocks() < numDataBlocks + numListBlocks) {
        warn("Not enough free blocks\n");
        return 0;
    }
    
    for (u32 ref = nr, i = 0; i < numListBlocks; i++) {

        // Add a new file list block
        ref = partition.addFileListBlock(nr, ref);
    }
    
    for (u32 ref = nr, i = 1; i <= numDataBlocks; i++) {

        // Add a new data block
        ref = partition.addDataBlock(i, nr, ref);

        // Add references to the new data block
        addDataBlockRef(ref);
        
        // Add data
        FSBlock *block = partition.dev.blockPtr(ref);
        if (block) {
            isize written = block->addData(buffer, size);
            setFileSize((u32)(getFileSize() + written));
            buffer += written;
            size -= written;
        }
    }

    return getFileSize();
}


bool
FSFileHeaderBlock::addDataBlockRef(u32 ref)
{
    return addDataBlockRef(ref, ref);
}

bool
FSFileHeaderBlock::addDataBlockRef(u32 first, u32 ref)
{
    std::set<u32> visited;
    
    // If this block has space for more references, add it here
    if (getNumDataBlockRefs() < getMaxDataBlockRefs()) {

        if (getNumDataBlockRefs() == 0) setFirstDataBlockRef(first);
        setDataBlockRef(getNumDataBlockRefs(), ref);
        incNumDataBlockRefs();
        return true;
    }

    // Otherwise, add it to an extension block
    FSFileListBlock *item = getNextListBlock();
    
    while (item) {
        
        // Break the loop if we visit a block twice
        if (visited.find(item->nr) != visited.end()) return false;
        
        if (item->addDataBlockRef(first, ref)) return true;
        item = item->getNextListBlock();
    }
    
    return false;
}
