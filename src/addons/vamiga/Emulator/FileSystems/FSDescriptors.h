// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSObjects.h"
#include "FSBlock.h"
#include "FSEmptyBlock.h"
#include "FSBootBlock.h"
#include "FSRootBlock.h"
#include "FSBitmapBlock.h"
#include "FSBitmapExtBlock.h"
#include "FSUserDirBlock.h"
#include "FSFileHeaderBlock.h"
#include "FSFileListBlock.h"
#include "FSDataBlock.h"

/* To create a FSDevice, the layout parameters of the represendet device have
 * to be provided. This is done by passing a structure of type FSDeviceLayout
 * which contains physical properties such as the number of cylinders and heads
 * and logical parameters such as the number of sectors per track. In addition,
 * the structure contains one or more elements of type FSPartitionLayout. They
 * provide the information how the device is partitioned.
 *
 * FSDeviceDescriptors can be obtained in several ways. If a descriptor for
 * diskette is needed, it can be created by specifiying the form factor and
 * density of the disk. Furthermore, a suitabe device constructor can be
 * extracted directly from an ADF or HDF.
 */

struct FSDeviceDescriptor : AmigaObject {
    
    // Physical device parameters
    isize numCyls = 0;
    isize numHeads = 0;
        
    // Logical device parameters
    isize numSectors = 0;
    isize numBlocks = 0;
    isize numReserved = 0;
    isize bsize = 0;
    
    // Partition parameters
    std::vector<struct FSPartitionDescriptor> partitions;
    
    
    //
    // Initializing
    //
    
    FSDeviceDescriptor() { }
    FSDeviceDescriptor(DiskDiameter type, DiskDensity density, FSVolumeType dos = FS_OFS);

    const char *getDescription() const override { return "FSLayout"; }
    void dump();
};

struct FSPartitionDescriptor : AmigaObject {
    
    // File system type
    FSVolumeType dos = FS_NODOS;
    
    // Cylinder boundaries
    isize lowCyl = 0;
    isize highCyl = 0;
        
    // Location of the root block
    u32 rootBlock = 0;
    
    // References to all bitmap blocks and bitmap extension blocks
    vector<u32> bmBlocks;
    vector<u32> bmExtBlocks;

    
    //
    // Initializing
    //
    
    FSPartitionDescriptor(FSVolumeType dos, isize firstCyl, isize lastCyl, u32 root);

    const char *getDescription() const override { return "FSPartition"; }
    
    void dump();

    
    //
    // Querying partition properties
    //
    
    // Returns the number of cylinders in this partition
    isize numCyls() { return highCyl - lowCyl + 1; }
};
