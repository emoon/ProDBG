// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "FSDescriptors.h"

FSDeviceDescriptor::FSDeviceDescriptor(DiskDiameter type, DiskDensity density, FSVolumeType dos)
{
    if (type == INCH_525 && density == DISK_DD) {
        numCyls = 40; numSectors = 11;

    } else if (type == INCH_35 && density == DISK_DD) {
        numCyls = 80; numSectors = 11;
    
    } else if (type == INCH_35 && density == DISK_HD) {
        numCyls = 80; numSectors = 22;

    } else {
        assert(false);
    }

    numHeads    = 2;
    numBlocks   = numCyls * numHeads * numSectors;
    numReserved = 2;
    bsize       = 512;
    
    // Determine the location of the root block and the bitmap block
    Block root   = (Block)(numBlocks / 2);
    Block bitmap = root + 1;

    partitions.push_back(FSPartitionDescriptor(dos, 0, numCyls - 1, root));
    partitions[0].bmBlocks.push_back(bitmap);
}

void
FSDeviceDescriptor::dump()
{
    msg("       Cylinders : %zd\n", numCyls);
    msg("           Heads : %zd\n", numHeads);
    msg("         Sectors : %zd\n", numSectors);
    msg("          Blocks : %lld\n", numBlocks);
    msg("        Reserved : %zd\n", numReserved);
    msg("           BSize : %zd\n", bsize);
    msg("\n");
    
    for (auto& p : partitions) { p.dump(); }
        
    /*
    for (isize i = 0; i < (isize)partitions.size(); i++) {
        partitions[i].dump();
    }
    */
}

FSPartitionDescriptor::FSPartitionDescriptor(FSVolumeType dos,
                                             isize firstCyl, isize lastCyl,
                                             Block root)
{
    this->dos = dos;
    this->lowCyl = firstCyl;
    this->highCyl = lastCyl;
    this->rootBlock = root;

    assert(bmBlocks.size() == 0);
    assert(bmExtBlocks.size() == 0);
}

void
FSPartitionDescriptor::dump()
{
    msg("       Partition : %zd - %zd\n", lowCyl, highCyl);
    msg("     File system : %s\n", FSVolumeTypeEnum::key(dos));
    msg("      Root block : %d\n", rootBlock);
    msg("   Bitmap blocks : ");
    for (auto& it : bmBlocks) { msg("%d ", it); }
    msg("\n");
    msg("Extension blocks : ");
    for (auto& it : bmExtBlocks) { msg("%d ", it); }
    msg("\n\n");
}
