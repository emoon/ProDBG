// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSUserDirBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"
#include "IO.h"

FSUserDirBlock::FSUserDirBlock(FSPartition &p, Block nr) : FSBlock(p, nr)
{
    data = new u8[bsize()]();
        
    set32(0, 2);                         // Type
    set32(1, nr);                        // Block pointer to itself
    setCreationDate(time(nullptr));      // Creation date
    set32(-1, 2);                        // Sub type
}

FSUserDirBlock::FSUserDirBlock(FSPartition &p, Block nr, const char *name) :
FSUserDirBlock(p, nr)
{
    setName(FSName(name));
}

FSUserDirBlock::~FSUserDirBlock()
{
    delete [] data;
}

FSItemType
FSUserDirBlock::itemType(isize byte) const
{
    // Intercept some special locations
    if (byte == 328) return FSI_BCPL_STRING_LENGTH;
    if (byte == 432) return FSI_BCPL_STRING_LENGTH;

    // Translate the byte index to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;

    switch (word) {
        case 0:   return FSI_TYPE_ID;
        case 1:   return FSI_SELF_REF;
        case 2:
        case 3:
        case 4:   return FSI_UNUSED;
        case 5:   return FSI_CHECKSUM;
        case -50:
        case -49: return FSI_UNUSED;
        case -48: return FSI_PROT_BITS;
        case -47: return FSI_UNUSED;
        case -23: return FSI_CREATED_DAY;
        case -22: return FSI_CREATED_MIN;
        case -21: return FSI_CREATED_TICKS;
        case -4:  return FSI_NEXT_HASH_REF;
        case -3:  return FSI_PARENT_DIR_REF;
        case -2:  return FSI_UNUSED;
        case -1:  return FSI_SUBTYPE_ID;
    }
    
    if (word <= -51)                return FSI_HASH_REF;
    if (word >= -46 && word <= -24) return FSI_BCPL_COMMENT;
    if (word >= -20 && word <= -5)  return FSI_BCPL_DIR_NAME;

    assert(false);
    return FSI_UNKNOWN;
}

ErrorCode
FSUserDirBlock::check(isize byte, u8 *expected, bool strict) const
{
    // Translate the byte index to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
    u32 value = get32(word);
    
    switch (word) {
        case  0: EXPECT_LONGWORD(2);        break;
        case  1: EXPECT_SELFREF;            break;
        case  2:
        case  3:
        case  4: EXPECT_BYTE(0);            break;
        case  5: EXPECT_CHECKSUM;           break;
        case -4: EXPECT_OPTIONAL_HASH_REF;  break;
        case -3: EXPECT_PARENT_DIR_REF;     break;
        case -2: EXPECT_BYTE(0);            break;
        case -1: EXPECT_LONGWORD(2);        break;
    }
    if (word <= -51) EXPECT_OPTIONAL_HASH_REF;
    
    return ERROR_OK;
}

void
FSUserDirBlock::dump() const
{
    printf("        Name: %s\n", getName().c_str());
    printf("     Comment: %s\n", getComment().c_str());
    printf("     Created: %s\n", getCreationDate().str().c_str());
    printf("      Parent: %d\n", getParentDirRef());
    printf("        Next: %d\n", getNextHashRef());
}

ErrorCode
FSUserDirBlock::exportBlock(const char *exportDir)
{
    string path = exportDir;
    path += "/" + partition.dev.getPath(this);

    printf("Creating directory %s\n", path.c_str());
    
    // Try to create a directory on the host file system
    if (mkdir(path.c_str(), 0777) != 0) return ERROR_FS_CANNOT_CREATE_DIR;
    
    return ERROR_OK;
}
