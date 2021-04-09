// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSDataBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSDataBlock::FSDataBlock(FSPartition &p, u32 nr) : FSBlock(p, nr)
{
    data = new u8[p.dev.bsize]();
}

FSDataBlock::~FSDataBlock()
{
    delete [] data;
}


//
// Original File System (OFS)
//

OFSDataBlock::OFSDataBlock(FSPartition &p, u32 nr) : FSDataBlock(p, nr)
{
    data = new u8[bsize()]();
    
    set32(0, 8); // Block type
}

void
OFSDataBlock::dump() const
{
    msg("File header block : %d\n", getFileHeaderRef());
    msg("     Chain number : %d\n", getDataBlockNr());
    msg("       Data bytes : %d\n", getDataBytesInBlock());
    msg("  Next data block : %d\n", getNextDataBlockRef());
    msg("\n");
}

FSItemType
OFSDataBlock::itemType(isize pos) const
{
    if (pos < 24) {
        
        isize word = pos / 4;
        
        switch (word) {
                
            case 0: return FSI_TYPE_ID;
            case 1: return FSI_FILEHEADER_REF;
            case 2: return FSI_DATA_BLOCK_NUMBER;
            case 3: return FSI_DATA_COUNT;
            case 4: return FSI_NEXT_DATA_BLOCK_REF;
            case 5: return FSI_CHECKSUM;
        }
    }
    
    return FSI_DATA;
}

ErrorCode
OFSDataBlock::check(isize byte, u8 *expected, bool strict) const
{
    /* Note: At location 1, many disks store a reference to the bitmap block
     * instead of a reference to the file header block. We ignore to report
     * this common inconsistency if 'strict' is set to false.
     */

    if (byte < 24) {
        
        isize word = byte / 4;
        u32 value = get32(word);
                
        switch (word) {
                
            case 0: EXPECT_LONGWORD(8);                 break;
            case 1: if (strict) EXPECT_FILEHEADER_REF;  break;
            case 2: EXPECT_DATABLOCK_NUMBER;            break;
            case 3: EXPECT_LESS_OR_EQUAL(dsize());      break;
            case 4: EXPECT_OPTIONAL_DATABLOCK_REF;      break;
            case 5: EXPECT_CHECKSUM;                    break;
        }
    }
    
    return ERROR_OK;
}

isize
OFSDataBlock::writeData(FILE *file, isize size)
{
    assert(file != nullptr);
    
    isize count = std::min(dsize(), size);
    for (isize i = 0; i < count; i++) fputc(data[i + headerSize()], file);
    return count;
}

isize
OFSDataBlock::addData(const u8 *buffer, isize size)
{
    isize count = std::min(bsize() - headerSize(), size);

    memcpy(data + headerSize(), buffer, count);
    setDataBytesInBlock((u32)count);
    
    return count;
}

isize
OFSDataBlock::dsize() const
{
    return bsize() - headerSize();
}


//
// Fast File System (FFS)
//

FFSDataBlock::FFSDataBlock(FSPartition &p, u32 nr) : FSDataBlock(p, nr) { }

void
FFSDataBlock::dump() const
{
}

FSItemType
FFSDataBlock::itemType(isize pos) const
{
    return FSI_DATA;
}

isize
FFSDataBlock::writeData(FILE *file, isize size)
{
    assert(file != nullptr);
    
    isize count = std::min(dsize(), size);
    for (isize i = 0; i < count; i++) fputc(data[i + headerSize()], file);
    return count;
}

isize
FFSDataBlock::addData(const u8 *buffer, isize size)
{
    isize count = std::min(bsize(), size);
    memcpy(data, buffer, count);
    return count;
}

isize
FFSDataBlock::dsize() const
{
    return bsize() - headerSize();
}
