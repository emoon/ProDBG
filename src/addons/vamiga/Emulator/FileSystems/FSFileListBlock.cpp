// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSFileListBlock::FSFileListBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();

    set32(0, 16);                         // Type
    set32(1, nr);                         // Block pointer to itself
    set32(-1, (u32)-3);                   // Sub type
}

FSFileListBlock::~FSFileListBlock()
{
    delete [] data;
}

void
FSFileListBlock::dump()
{
    printf(" Block count : %d / %d\n", numDataBlockRefs(), maxDataBlockRefs());
    printf("       First : %d\n", getFirstDataBlockRef());
    printf("Header block : %d\n", getFileHeaderRef());
    printf("   Extension : %d\n", getNextListBlockRef());
    printf(" Data blocks : ");
    for (u32 i = 0; i < numDataBlockRefs(); i++) printf("%d ", getDataBlockRef(i));
    printf("\n");
}

bool
FSFileListBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    
    result &= assertNotNull(getFileHeaderRef(), verbose);
    result &= assertInRange(getFileHeaderRef(), verbose);
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
FSFileListBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
}

bool
FSFileListBlock::addDataBlockRef(u32 first, u32 ref)
{
    // The caller has to ensure that this block contains free slots
    if (numDataBlockRefs() < maxDataBlockRefs()) {

        setFirstDataBlockRef(first);
        setDataBlockRef(numDataBlockRefs(), ref);
        incDataBlockRefs();
        return true;
    }

    return false;
}
