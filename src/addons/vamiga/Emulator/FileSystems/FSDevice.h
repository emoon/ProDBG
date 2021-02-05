// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaTypes.h"
#include "FSObjects.h"
#include "FSPartition.h"
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
#include "ADFFile.h"
#include "HDFFile.h"

#include <dirent.h>

/* This class provides the basic functionality of the Amiga File Systems OFS
 * and FFS. Starting from an empty volume, files can be added or removed,
 * and boot blocks can be installed. Furthermore, functionality is provided to
 * import and export the file system from and to ADF files.
 */

class FSDevice : AmigaObject {
    
    friend class FSPartition;
    friend class FSBlock;
    friend class FSEmptyBlock;
    friend class FSBootBlock;
    friend class FSRootBlock;
    friend class FSBitmapBlock;
    friend class FSBitmapExtBlock;
    friend class FSUserDirBlock;
    friend class FSFileHeaderBlock;
    friend class FSFileListBlock;
    friend class FSDataBlock;
    friend class OFSDataBlock;
    friend class FFSDataBlock;
    friend class FSHashTable;

protected:
            
    // Physical device parameters
    isize numCyls = 0;
    isize numHeads = 0;
    
    // Logical device parameters
    isize numSectors = 0;
    isize numBlocks = 0;
    isize numReserved = 0;
    isize bsize = 0;
            
    // The partition table
    std::vector<FSPartitionPtr> partitions;
    
    // The block storage
    std::vector<BlockPtr> blocks;
            
    // The currently selected partition
    isize cp = 0;
    
    // The currently selected directory
    u32 cd = 0;
    

    //
    // Factory methods
    //
    
public:

    // Creates a file system with a custom device descriptor
    static FSDevice *makeWithFormat(FSDeviceDescriptor &layout);

    // Creates a file system for a standard floppy disk
    static FSDevice *makeWithFormat(DiskDiameter type, DiskDensity density);

    // Creates a file system from an ADF or HDF
    static FSDevice *makeWithADF(class ADFFile *adf, ErrorCode *error);
    static FSDevice *makeWithHDF(class HDFFile *hdf, ErrorCode *error);
    
    // Creates a file system with the contents of a host file system directory
    static FSDevice *make(DiskDiameter type, DiskDensity density, const char *path);
    static FSDevice *make(FSVolumeType type, const char *path);
    
    
    //
    // Initializing
    //
    
public:

    FSDevice(isize capacity);
    ~FSDevice();
    
    const char *getDescription() const override { return "FSVolume"; }
        
    // Prints information about this volume
    void info();
    
    // Prints debug information about this volume
    void dump();
        
    
    //
    // Querying file system properties
    //
    
public:
            
    // Returns the total device capacity in blocks
    isize getCapacity() { return numBlocks; }
    
    // Reports layout information
    isize getNumCyls() { return numCyls; }
    isize getNumHeads() { return numHeads; }
    isize getNumTracks() { return getNumCyls() * getNumHeads(); }
    isize getNumSectors() { return numSectors; }
    isize getNumBlocks() { return numBlocks; }

    
    //
    // Querying properties of the current partition
    //
    
    // Returns the DOS version of the current partition
    FSVolumeType dos() const { return partitions[cp]->dos; }
    bool isOFS() const { return partitions[cp]->isOFS(); }
    bool isFFS() const { return partitions[cp]->isFFS(); }
    
    
    //
    // Working with partitions
    //
    
public:
    
    // Returns the number of partitions
    isize numPartitions() { return (isize)partitions.size(); }
    
    // Returns the partition a certain block belongs to
    isize partitionForBlock(u32 ref);

    // Gets or sets the name of the current partition
    FSName getName() { return partitions[cp]->getName(); }
    void setName(FSName name) { partitions[cp]->setName(name); }
    
    
    //
    // Working with boot blocks
    //
    
public:
    // Installs a boot block
    void makeBootable(long bootBlockID) { partitions[cp]->makeBootable(bootBlockID); }

    // Removes a boot block virus from the current partition (if any)
    void killVirus() { partitions[cp]->killVirus(); }
    
    
    //
    // Accessing blocks
    //
    
public:
    
    // Returns the type of a certain block
    FSBlockType blockType(u32 nr);

    // Returns the usage type of a certain byte in a certain block
    FSItemType itemType(u32 nr, isize pos) const;
    
    // Queries a pointer from the block storage (may return nullptr)
    FSBlock *blockPtr(u32 nr) const;

    // Queries a pointer to a block of a certain type (may return nullptr)
    FSBootBlock *bootBlockPtr(u32 nr);
    FSRootBlock *rootBlockPtr(u32 nr);
    FSBitmapBlock *bitmapBlockPtr(u32 nr);
    FSBitmapExtBlock *bitmapExtBlockPtr(u32 nr);
    FSUserDirBlock *userDirBlockPtr(u32 nr);
    FSFileHeaderBlock *fileHeaderBlockPtr(u32 nr);
    FSFileListBlock *fileListBlockPtr(u32 nr);
    FSDataBlock *dataBlockPtr(u32 nr);
    FSBlock *hashableBlockPtr(u32 nr);
    
    
    //
    // Creating and deleting blocks
    //
    
public:
    
    // Updates the checksums in all blocks
    void updateChecksums();
    
    
    //
    // Managing directories and files
    //
    
public:
    
    // Returns the block representing the current directory
    FSBlock *currentDirBlock();
    
    // Changes the current directory
    FSBlock *changeDir(const char *name);

    // Prints a directory listing
    void printDirectory(bool recursive);

    // Returns the path of a file system item
    string getPath(FSBlock *block);
    string getPath(u32 ref) { return getPath(blockPtr(ref)); }
    string getPath() { return getPath(currentDirBlock()); }

    // Seeks an item inside the current directory
    u32 seekRef(FSName name);
    u32 seekRef(const char *name) { return seekRef(FSName(name)); }
    FSBlock *seek(const char *name) { return blockPtr(seekRef(name)); }
    FSBlock *seekDir(const char *name) { return userDirBlockPtr(seekRef(name)); }
    FSBlock *seekFile(const char *name) { return fileHeaderBlockPtr(seekRef(name)); }

    // Adds a reference to the current directory
    void addHashRef(u32 ref);
    void addHashRef(FSBlock *block);
    
    // Creates a new directory
    FSBlock *makeDir(const char *name);

    // Creates a new file
    FSBlock *makeFile(const char *name);
    FSBlock *makeFile(const char *name, const u8 *buffer, isize size);
    FSBlock *makeFile(const char *name, const char *str);
        
    
    //
    // Traversing linked lists
    //
    
    // Returns the last element in the list of extension blocks
    FSBlock *lastFileListBlockInChain(u32 start);
    FSBlock *lastFileListBlockInChain(FSBlock *block);

    // Returns the last element in the list of blocks with the same hash
    FSBlock *lastHashBlockInChain(u32 start);
    FSBlock *lastHashBlockInChain(FSBlock *block);

    
    //
    // Traversing the file system
    //
    
public:
    
    // Returns a collections of nodes for all items in the current directory
    ErrorCode collect(u32 ref, std::vector<u32> &list, bool recursive = true);

private:
    
    // Collects all references stored in a hash table
    ErrorCode collectHashedRefs(u32 ref, std::stack<u32> &list, std::set<u32> &visited);

    // Collects all references with the same hash value
    ErrorCode collectRefsWithSameHashValue(u32 ref, std::stack<u32> &list, std::set<u32> &visited);

 
    //
    // Integrity checking
    //

public:
    
    // Checks all blocks in this volume
    FSErrorReport check(bool strict) const;

    // Checks a single byte in a certain block
    ErrorCode check(u32 blockNr, isize pos, u8 *expected, bool strict) const;

    // Checks if the block with the given number is part of the volume
    bool isBlockNumber(u32 nr) { return nr < numBlocks; }

    // Checks if the type of a block matches one of the provides types
    ErrorCode checkBlockType(u32, FSBlockType type);
    ErrorCode checkBlockType(u32, FSBlockType type, FSBlockType altType);

    // Checks if a certain block is corrupted
    bool isCorrupted(u32 blockNr) { return getCorrupted(blockNr) != 0; }

    // Returns the position in the corrupted block list (0 = OK)
    isize getCorrupted(u32 blockNr);

    // Returns the number of the next or previous corrupted block
    isize nextCorrupted(u32 blockNr);
    isize prevCorrupted(u32 blockNr);

    // Checks if a certain block is the n-th corrupted block
    bool isCorrupted(u32 blockNr, isize n);

    // Returns the number of the the n-th corrupted block
    u32 seekCorruptedBlock(isize n);
    
    
    //
    // Importing and exporting
    //
    
public:
        
    // Reads a single byte from a block
    u8 readByte(u32 block, isize offset) const;

    // Predicts the type of a block by analyzing its number and data (DEPRECATED)
    FSBlockType predictBlockType(u32 nr, const u8 *buffer);

    // Imports the volume from a buffer compatible with the ADF format
    bool importVolume(const u8 *src, isize size);
    bool importVolume(const u8 *src, isize size, ErrorCode *error);

    // Imports a directory from the host file system
    bool importDirectory(const char *path, bool recursive = true);
    bool importDirectory(const char *path, DIR *dir, bool recursive = true);

    // Exports the volume to a buffer compatible with the ADF format
    bool exportVolume(u8 *dst, isize size);
    bool exportVolume(u8 *dst, isize size, ErrorCode *error);

    // Exports a single block or a range of blocks
    bool exportBlock(isize nr, u8 *dst, isize size);
    bool exportBlock(isize nr, u8 *dst, isize size, ErrorCode *error);
    bool exportBlocks(isize first, isize last, u8 *dst, isize size);
    bool exportBlocks(isize first, isize last, u8 *dst, isize size, ErrorCode *error);

    // Exports the volume to a directory of the host file system
    ErrorCode exportDirectory(const char *path);
};
