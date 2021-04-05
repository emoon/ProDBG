// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSVolume.h"

FSUserDirBlock::FSUserDirBlock(FSVolume &ref, u32 nr) : FSBlock(ref, nr)
{
    data = new u8[ref.bsize]();
        
    set32(0, 2);                         // Type
    set32(1, nr);                        // Block pointer to itself
    setCreationDate(time(NULL));         // Creation date
    set32(-1, 2);                        // Sub type
}

FSUserDirBlock::FSUserDirBlock(FSVolume &ref, u32 nr, const char *name) : FSUserDirBlock(ref, nr)
{
    setName(FSName(name));
}

FSUserDirBlock::~FSUserDirBlock()
{
    delete [] data;
}

void
FSUserDirBlock::dump()
{
    printf("        Name: %s\n", getName().cStr);
    printf("        Path: ");    printPath(); printf("\n");
    printf("     Comment: %s\n", getComment().cStr);
    printf("     Created: ");    getCreationDate().print(); printf("\n");
    printf("      Parent: %d\n", getParentDirRef());
    printf("        Next: %d\n", getNextHashRef());
}

bool
FSUserDirBlock::check(bool verbose)
{
    bool result = FSBlock::check(verbose);
    result &= checkHashTable(verbose);
    return result;
}

void
FSUserDirBlock::updateChecksum()
{
    set32(5, 0);
    set32(5, checksum());
}
