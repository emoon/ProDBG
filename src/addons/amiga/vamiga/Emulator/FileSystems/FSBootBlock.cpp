// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "BootBlockImage.h"
#include "FSBootBlock.h"
#include "FSDevice.h"
#include "FSPartition.h"

FSBootBlock::FSBootBlock(FSPartition &p, Block nr) : FSBlock(p, nr)
{
    data = new u8[bsize()]();
    
    if (nr == p.firstBlock && p.dos != FS_NODOS) {
        data[0] = 'D';
        data[1] = 'O';
        data[2] = 'S';
        data[3] = (u8)p.dos;
    }
}

FSBootBlock::~FSBootBlock()
{
    delete [] data;
}


FSVolumeType
FSBootBlock::dos() const
{
    // Only proceed if the header begins with 'DOS'
    if (strncmp((const char *)data, "DOS", 3)) return FS_NODOS;
        
    // Only proceed if the DOS version number is valid
    if (data[3] > 7) return FS_NODOS;
    
    return (FSVolumeType)data[3];
}

FSItemType
FSBootBlock::itemType(isize byte) const
{
    if (nr == partition.firstBlock) {
        
        if (byte <= 2) return FSI_DOS_HEADER;
        if (byte == 3) return FSI_DOS_VERSION;
        if (byte <= 7) return FSI_CHECKSUM;
    }
    
    return FSI_BOOTCODE;
}

ErrorCode
FSBootBlock::check(isize byte, u8 *expected, bool strict) const
{
    if (nr == partition.firstBlock) {
 
        isize word = byte / 4;
        u32 value = data[byte];
        
        if (byte == 0) EXPECT_BYTE('D');
        if (byte == 1) EXPECT_BYTE('O');
        if (byte == 2) EXPECT_BYTE('S');
        if (byte == 3) EXPECT_DOS_REVISION;
        if (word == 1) { value = get32(1); EXPECT_CHECKSUM; }
    }
    
    return ERROR_OK;
}

isize
FSBootBlock::checksumLocation() const
{
    return (nr == partition.firstBlock) ? 1 : -1;
}

u32
FSBootBlock::checksum() const {
    
    // Only call this function for the first boot block in a partition
    assert(nr == partition.firstBlock);
        
    u32 result = get32(0), prec;

    // First boot block
    for (isize i = 2; i < bsize() / 4; i++) {
        
        prec = result;
        if ( (result += get32(i)) < prec) result++;
    }

    // Second boot block
    u8 *p = partition.dev.blocks[1]->data;
    
    for (isize i = 0; i < bsize() / 4; i++) {
        
        prec = result;
        if ( (result += FSBlock::read32(p + 4*i)) < prec) result++;
    }

    return ~result;
}

void
FSBootBlock::dump() const
{
    msg("       Header : ");
    for (isize i = 0; i < 8; i++) msg("%02X ", data[i]);
    msg("\n");
}

void
FSBootBlock::writeBootBlock(BootBlockId id, isize page)
{
    assert(page == 0 || page == 1);
    
    debug(FS_DEBUG, "writeBootBlock(%s, %zd)\n", BootBlockIdEnum::key(id), page);
    
    if (id != BB_NONE) {
        
        // Read boot block image from the database
        auto image = BootBlockImage(id);
        
        if (page == 0) {
            image.write(data + 4, 4, 511); // Write 508 bytes (skip header)
        } else {
            image.write(data, 512, 1023);  // Write 512 bytes
        }
    }
}
