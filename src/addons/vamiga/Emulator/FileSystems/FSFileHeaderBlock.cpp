// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
   
    // Setup constant values
    
    set32(0, 2);                   // Type
    set32(1, nr);                  // Block pointer to itself
    setCreationDate(time(NULL));   // Creation date
    set32(-1, (u32)-3);            // Sub type
}

FSFileHeaderBlock::FSFileHeaderBlock(FSVolume &ref, u32 nr, const char *name) :
FSFileHeaderBlock(ref, nr)
{
    setName(FSName(name));
}

void
FSFileHeaderBlock::dump()
{
    printf("           Name : %s\n", getName().cStr);
    printf("           Path : ");    printPath(); printf("\n");
    printf("        Comment : %s\n", getComment().cStr);
    printf("        Created : ");    getCreationDate().print(); printf("\n");
    printf("           Next : %d\n", getNextHashRef());
    printf("      File size : %d\n", getFileSize());

    printf("    Block count : %d / %d\n", numDataBlockRefs(), maxDataBlockRefs());
    printf("          First : %d\n", getFirstDataBlockRef());
    printf("     Parent dir : %d\n", getParentDirRef());
    printf(" FileList block : %d\n", getNextListBlockRef());
    
    printf("    Data blocks : ");
    for (u32 i = 0; i < numDataBlockRefs(); i++) printf("%d ", getDataBlockRef(i));
    printf("\n");
}

bool
FSFileHeaderBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    result &= assertNotNull(getParentDirRef(), verbose);
    result &= assertInRange(getParentDirRef(), verbose);
    result &= assertInRange(getFirstDataBlockRef(), verbose);
    result &= assertInRange(getNextListBlockRef(), verbose);

    for (u32 i = 0; i < maxDataBlockRefs(); i++) {
        result &= assertInRange(getDataBlockRef(i), verbose);
    }
    
    if (numDataBlockRefs() > 0 && getFirstDataBlockRef() == 0) {
        if (verbose) fprintf(stderr, "Missing reference to first data block\n");
        return false;
    }
    
    if (numDataBlockRefs() < maxDataBlockRefs() && getNextListBlockRef() != 0) {
        if (verbose) fprintf(stderr, "Unexpectedly found an extension block\n");
        return false;
    }
    
    return result;
}

void
FSFileHeaderBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
}

FSName
FSFileHeaderBlock::getName()
{
    return FSName(data + bsize() - 20 * 4);
}

void
FSFileHeaderBlock::setName(FSName name)
{
    name.write(data + bsize() - 20 * 4);
}

FSComment
FSFileHeaderBlock::getComment()
{
    return FSComment(data + bsize() - 46 * 4);
}

void
FSFileHeaderBlock::setComment(FSComment name)
{
    name.write(data + bsize() - 46 * 4);
}

size_t
FSFileHeaderBlock::addData(const u8 *buffer, size_t size)
{
    printf("addData(%p,%zu)\n", buffer, size);

    assert(getFileSize() == 0);
    
    // Compute the required number of DataBlocks
    u32 bytes = volume.bytesInDataBlock();
    u32 numDataBlocks = (size + bytes - 1) / bytes;

    // Compute the required number of FileListBlocks
    u32 numDataListBlocks = 0;
    if (numDataBlocks > maxDataBlockRefs()) {
        numDataListBlocks = 1 + (numDataBlocks - maxDataBlockRefs()) / maxDataBlockRefs();
    }

    printf("Required DataBlocks: %d\n", numDataBlocks);
    printf("Required DataListBlocks: %d\n", numDataListBlocks);
    
    // TODO: Check if the volume has enough free space
    
    for (u32 ref = nr, i = 0; i < numDataListBlocks; i++) {

        // Add a new file list block
        ref = volume.addFileListBlock(nr, ref);
    }
    
    for (u32 ref = nr, i = 1; i <= numDataBlocks; i++) {

        // Add a new data block
        ref = volume.addDataBlock(i, nr, ref);

        // Add references to the new data block
        addDataBlockRef(ref);
        
        // Add data
        FSBlock *block = volume.block(ref);
        if (block) {
            size_t written = block->addData(buffer, size);
            setFileSize(getFileSize() + written);
            buffer += written;
            size -= written;
        }
    }

    return getFileSize();
}


bool
FSFileHeaderBlock::addDataBlockRef(u32 ref)
{
    return addDataBlockRef(nr, ref);
}

bool
FSFileHeaderBlock::addDataBlockRef(u32 first, u32 ref)
{
    // If this block has space for more references, add it here
    if (numDataBlockRefs() < maxDataBlockRefs()) {

        if (numDataBlockRefs() == 0) setFirstDataBlockRef(ref);
        setDataBlockRef(numDataBlockRefs(), ref);
        incDataBlockRefs();
        return true;
    }

    // Otherwise, add it to an extension block
    FSFileListBlock *item = getNextExtensionBlock();
    
    for (int i = 0; item && i < searchLimit; i++) {
        
        if (item->addDataBlockRef(first, ref)) return true;
        item = item->getNextExtensionBlock();
    }
    
    assert(false);
    return false;
}
