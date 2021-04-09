// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HDFFile.h"
#include "IO.h"

HDFFile::HDFFile()
{
}

bool
HDFFile::isCompatiblePath(const string &path)
{
    string suffix = util::extractSuffix(path);
    return suffix == "hdf" || suffix == "HDF";
}

bool
HDFFile::isCompatibleStream(std::istream &stream)
{
    return util::streamLength(stream) % 512 == 0;
}

bool
HDFFile::hasRDB() const
{
    // The rigid disk block must be among the first 16 blocks
    if (size >= 16 * 512) {
        for (isize i = 0; i < 16; i++) {
            if (strcmp((const char *)data + i * 512, "RDSK") == 0) return true;
        }
    }
    return false;
}

isize
HDFFile::numCyls() const
{
    assert(size % bsize() == 0);
    
    if (hasRDB()) warn("HDF RDB images are not supported");

    return size / bsize() / numSectors() / numSides();
}

isize
HDFFile::numSides() const
{
    if (hasRDB()) warn("HDF RDB images are not supported");
    return 1;
}

isize
HDFFile::numSectors() const
{
    if (hasRDB()) warn("HDF RDB images are not supported");
    return 32;
}

isize
HDFFile::numReserved() const
{
    if (hasRDB()) warn("HDF RDB images are not supported");
    return 2;
}

isize
HDFFile::numBlocks() const
{
    assert((long)size / bsize() == numCyls() * numSides() * numSectors());
    return size / bsize();
}

isize
HDFFile::bsize() const
{
    if (hasRDB()) warn("HDF RDB images are not supported");
    return 512;
}

FSDeviceDescriptor
HDFFile::layout()
{
    FSDeviceDescriptor result;
    
    result.numCyls     = numCyls();
    result.numHeads    = numSides();
    result.numSectors  = numSectors();
    result.numReserved = numReserved();
    result.bsize       = bsize();
    result.numBlocks   = result.numCyls * result.numHeads * result.numSectors;

    // Determine the location of the root block
    i64 highKey = result.numBlocks - 1;
    i64 rootKey = (result.numReserved + highKey) / 2;
    
    // Add partition
    result.partitions.push_back(FSPartitionDescriptor(dos(0),
                                                      0,
                                                      result.numCyls - 1,
                                                      (Block)rootKey));

    // Seek bitmap blocks
    Block ref = (Block)rootKey;
    isize cnt = 25;
    isize offset = bsize() - 49 * 4;
    
    while (ref && ref < (Block)result.numBlocks) {

        const u8 *p = data + (ref * bsize()) + offset;
    
        // Collect all references to bitmap blocks stored in this block
        for (isize i = 0; i < cnt; i++, p += 4) {
            if (Block bmb = FFSDataBlock::read32(p)) {
                if (bmb < result.numBlocks) {
                    result.partitions[0].bmBlocks.push_back(bmb);
                }
            }
        }
        
        // Continue collecting in the next extension bitmap block
        if ((ref = FFSDataBlock::read32(p))) {
            if (ref < result.numBlocks) result.partitions[0].bmExtBlocks.push_back(ref);
            cnt = (bsize() / 4) - 1;
            offset = 0;
        }
    }
    
    return result;
}

FSVolumeType
HDFFile::dos(isize blockNr)
{
    const char *p = (const char *)data + blockNr * 512;
    
    if (strncmp(p, "DOS", 3) || data[3] > 7) {
        return FS_NODOS;
    }

    return (FSVolumeType)p[3];
}
