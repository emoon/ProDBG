// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"

FSFileListBlock::FSFileListBlock(FSPartition &p, u32 nr) : FSBlock(p, nr)
{
    data = new u8[bsize()]();

    set32(0, 16);                         // Type
    set32(1, nr);                         // Block pointer to itself
    set32(-1, (u32)-3);                   // Sub type
}

FSFileListBlock::~FSFileListBlock()
{
    delete [] data;
}

void
FSFileListBlock::dump() const
{
    msg(" Block count : %zd / %zd\n", getNumDataBlockRefs(), getMaxDataBlockRefs());
    msg("       First : %d\n", getFirstDataBlockRef());
    msg("Header block : %d\n", getFileHeaderRef());
    msg("   Extension : %d\n", getNextListBlockRef());
    msg(" Data blocks : ");
    for (u32 i = 0; i < getNumDataBlockRefs(); i++) msg("%d ", getDataBlockRef(i));
    msg("\n");
}

FSItemType
FSFileListBlock::itemType(isize byte) const
{
    // Intercept some special locations
    if (byte == 328) return FSI_BCPL_STRING_LENGTH;
    if (byte == 432) return FSI_BCPL_STRING_LENGTH;

    // Translate 'pos' to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;

    switch (word) {
            
        case 0:   return FSI_TYPE_ID;
        case 1:   return FSI_SELF_REF;
        case 2:   return FSI_DATA_BLOCK_REF_COUNT;
        case 3:   return FSI_UNUSED;
        case 4:   return FSI_FIRST_DATA_BLOCK_REF;
        case 5:   return FSI_CHECKSUM;
        case -50:
        case -49:
        case -4:  return FSI_UNUSED;
        case -3:  return FSI_FILEHEADER_REF;
        case -2:  return FSI_EXT_BLOCK_REF;
        case -1:  return FSI_SUBTYPE_ID;
    }
    
    return word <= -51 ? FSI_DATA_BLOCK_REF : FSI_UNUSED;
}

ErrorCode
FSFileListBlock::check(isize byte, u8 *expected, bool strict) const
{
    /* Note: At location -3, many disks reference the bitmap block instead of
     * the file header block. We ignore to report this common inconsistency if
     * 'strict' is set to false.
     */

    // Translate 'pos' to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
    u32 value = get32(word);

    switch (word) {
            
        case   0: EXPECT_LONGWORD(16);                break;
        case   1: EXPECT_SELFREF;                     break;
        case   3: EXPECT_BYTE(0);                     break;
        case   4: EXPECT_OPTIONAL_DATABLOCK_REF;      break;
        case   5: EXPECT_CHECKSUM;                    break;
        case -50:
        case  -4: EXPECT_BYTE(0);                     break;
        case  -3: if (strict) EXPECT_FILEHEADER_REF;  break;
        case  -2: EXPECT_OPTIONAL_FILELIST_REF;       break;
        case  -1: EXPECT_LONGWORD(-3);                break;
    }
    
    // Data block references
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

bool
FSFileListBlock::addDataBlockRef(u32 first, u32 ref)
{
    // The caller has to ensure that this block contains free slots
    if (getNumDataBlockRefs() < getMaxDataBlockRefs()) {

        setFirstDataBlockRef(first);
        setDataBlockRef(getNumDataBlockRefs(), ref);
        incNumDataBlockRefs();
        return true;
    }

    return false;
}
