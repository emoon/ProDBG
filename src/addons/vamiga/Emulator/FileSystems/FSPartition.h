// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSPublicTypes.h"
#include "FSDescriptors.h"

struct FSPartition : AmigaObject {
    
    // The device this partition is part of
    class FSDevice &dev;
    
    // File system format
    FSVolumeType dos = FS_NODOS;
    
    // Cylinder boundaries
    isize lowCyl = 0;
    isize highCyl = 0;
    
    // Block boundaries
    u32 firstBlock = 0;
    u32 lastBlock = 0;
    
    // Location of the root block
    u32 rootBlock = 0;
    
    // Location of the bitmap blocks and extended bitmap blocks
    vector<u32> bmBlocks;
    vector<u32> bmExtBlocks;

    
    //
    // Factory methods
    //
    
public:

    // Creates a file system with a custom device descriptor
    static FSPartition *makeWithFormat(FSDevice &ref, FSPartitionDescriptor &layout);

    
    //
    // Initializing
    //
    
    FSPartition(FSDevice &ref);
    
    const char *getDescription() const override { return "FSPartition"; }

    // Prints a summary about this partition (called by FSDevice::info)
    void info() const;

    // Prints debug information about this partition
    void dump() const;

    // Predicts the type of a block by analyzing its number and data
    FSBlockType predictBlockType(u32 nr, const u8 *buffer) const;
    
    // Gets or sets the name of this partition
    FSName getName() const;
    void setName(FSName name);

    
    //
    // Querying partition properties
    //
    
    // Returns the file system category
    bool isOFS() const { return isOFSVolumeType(dos); }
    bool isFFS() const { return isFFSVolumeType(dos); }

    // Returns the size of a single block in bytes (usually 512)
    isize bsize() const;

    // Reports layout information about this partition
    isize numCyls() const { return highCyl - lowCyl + 1; }
    isize numBlocks() const;
    isize numBytes() const;
    
    // Reports usage information about this partition
    isize freeBlocks() const;
    isize usedBlocks() const;
    isize freeBytes() const;
    isize usedBytes() const;
    
    
    //
    // Creating and deleting blocks
    //
    
public:
    
    // Returns the number of required blocks to store a file of certain size
    isize requiredDataBlocks(isize fileSize) const;
    isize requiredFileListBlocks(isize fileSize) const;
    isize requiredBlocks(isize fileSize) const;

    // Seeks a free block and marks it as allocated
    u32 allocateBlock();
    u32 allocateBlockAbove(u32 ref);
    u32 allocateBlockBelow(u32 ref);

    // Deallocates a block
    void deallocateBlock(u32 ref);

    // Adds a new block of a certain kind
    u32 addFileListBlock(u32 head, u32 prev);
    u32 addDataBlock(u32 count, u32 head, u32 prev);
    
    // Creates a new block of a certain kind
    FSUserDirBlock *newUserDirBlock(const char *name);
    FSFileHeaderBlock *newFileHeaderBlock(const char *name);
    
    
    //
    // Working with the block allocation bitmap
    //

    // Returns the bitmap block storing the allocation bit for a certain block
    FSBitmapBlock *bmBlockForBlock(u32 relRef);

    // Checks if a block is marked as free in the allocation bitmap
    bool isFree(u32 ref) const;
    
    // Marks a block as allocated or free
    void markAsAllocated(u32 ref) { setAllocationBit(ref, 0); }
    void markAsFree(u32 ref) { setAllocationBit(ref, 1); }
    void setAllocationBit(u32 ref, bool value);

    
private:
    
    // Locates the allocation bit for a certain block
    FSBitmapBlock *locateAllocationBit(u32 ref, u32 *byte, u32 *bit) const;
    
    
    //
    // Working with boot blocks
    //
    
public:
    
    // Installs a boot block
    void makeBootable(long id);

    // Eliminates boot block virus (if any)
    void killVirus();
    
    
    //
    // Integrity checking
    //

public:
    
    // Performs several partition checks
    bool check(bool strict, FSErrorReport &report) const;

    // Checks if the block with the given number is part of this partition
    bool inRange(u32 nr) const { return nr >= firstBlock && nr <= lastBlock; }
};

typedef FSPartition* FSPartitionPtr;
