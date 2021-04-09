// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSRootBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSRootBlock::FSRootBlock(FSPartition &p, Block nr) : FSBlock(p, nr)
{
    data = new u8[bsize()]();
    
    assert(hashTableSize() == 72);
    
    set32(0, 2);                         // Type
    set32(3, (u32)hashTableSize());      // Hash table size
    set32(-50, 0xFFFFFFFF);              // Bitmap validity
    setCreationDate(time(nullptr));      // Creation date
    setModificationDate(time(nullptr));  // Modification date
    set32(-1, 1);                        // Sub type    
}

FSRootBlock::~FSRootBlock()
{
    delete [] data;
}

FSItemType
FSRootBlock::itemType(isize byte) const
{
    // Intercept some special locations
    if (byte == 432) return FSI_BCPL_STRING_LENGTH;

    // Translate the byte index to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
    
    switch (word) {
        case 0:   return FSI_TYPE_ID;
        case 1:
        case 2:   return FSI_UNUSED;
        case 3:   return FSI_HASHTABLE_SIZE;
        case 4:   return FSI_UNUSED;
        case 5:   return FSI_CHECKSUM;
        case -50: return FSI_BITMAP_VALIDITY;
        case -24: return FSI_BITMAP_EXT_BLOCK_REF;
        case -23: return FSI_MODIFIED_DAY;
        case -22: return FSI_MODIFIED_MIN;
        case -21: return FSI_MODIFIED_TICKS;
        case -7:  return FSI_CREATED_DAY;
        case -6:  return FSI_CREATED_MIN;
        case -5:  return FSI_CREATED_TICKS;
        case -4:
        case -3:
        case -2:  return FSI_UNUSED;
        case -1:  return FSI_SUBTYPE_ID;
            
        default:
            
            if (word <= -51)                return FSI_HASH_REF;
            if (word <= -25)                return FSI_BITMAP_BLOCK_REF;
            if (word >= -20 && word <= -8)  return FSI_BCPL_DISK_NAME;
    }
    
    assert(false);
    return FSI_UNKNOWN;
}

ErrorCode
FSRootBlock::check(isize byte, u8 *expected, bool strict) const
{
    // Translate the byte index to a (signed) long word index
    isize word = byte / 4; if (word >= 6) word -= bsize() / 4;
    u32 value = get32(word);
    
    switch (word) {
            
        case 0:   EXPECT_LONGWORD(2);                break;
        case 1:
        case 2:   if (strict) EXPECT_LONGWORD(0);    break;
        case 3:   if (strict) EXPECT_HASHTABLE_SIZE; break;
        case 4:   EXPECT_LONGWORD(0);                break;
        case 5:   EXPECT_CHECKSUM;                   break;
        case -50:                                    break;
        case -49: EXPECT_BITMAP_REF;                 break;
        case -24: EXPECT_OPTIONAL_BITMAP_EXT_REF;    break;
        case -4:
        case -3:
        case -2:  if (strict) EXPECT_LONGWORD(0);    break;
        case -1:  EXPECT_LONGWORD(1);                break;

        default:
            
            // Hash table area
            if (word <= -51) { EXPECT_OPTIONAL_HASH_REF; break; }
            
            // Bitmap block area
            if (word <= -25) { EXPECT_OPTIONAL_BITMAP_REF; break; }
    }
    
    return ERROR_OK;
}

void
FSRootBlock::dump() const
{
    msg("         Name : %s\n", getName().c_str());
    msg("      Created : %s\n", getCreationDate().str().c_str());
    msg("     Modified : %s\n", getModificationDate().str().c_str());
    msg("   Hash table : "); dumpHashTable(); printf("\n");
    msg("Bitmap blocks : ");
    for (isize i = 0; i < 25; i++) {
        if (isize ref = getBmBlockRef(i)) msg("%zd ", ref);
    }
    msg("\n");
    msg("   Next BmExt : %d\n", getNextBmExtBlockRef());
}

bool
FSRootBlock::addBitmapBlockRefs(std::vector<Block> &refs)
{
    auto it = refs.begin();
     
    // Record the first 25 references in the root block
    for (isize i = 0; i < 25; i++, it++) {
        if (it == refs.end()) return true;
        setBmBlockRef(i, *it);
    }
            
    // Record the remaining references in bitmap extension blocks
    FSBitmapExtBlock *ext = getNextBmExtBlock();
    while (ext && it != refs.end()) {
        ext->addBitmapBlockRefs(refs, it);
        ext = getNextBmExtBlock();
    }
    
    return it == refs.end();
}
