// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FSVOLUME_H
#define _FSVOLUME_H

#include <dirent.h>

#include "FSBlock.h"
#include "FSEmptyBlock.h"
#include "FSBootBlock.h"
#include "FSRootBlock.h"
#include "FSBitmapBlock.h"
#include "FSUserDirBlock.h"
#include "FSFileHeaderBlock.h"
#include "FSFileListBlock.h"
#include "FSDataBlock.h"
#include "ADFFile.h"

/* This class provides the basic functionality of the Amiga File Systems OFS
 * and FFS. Starting from an empty volume, files can be added or removed,
 * and boot blocks can be installed. Furthermore, functionality is provided to
 * import and export the file system from and to ADF files.
 */

class FSVolume : AmigaObject {
    
    friend class FSBlock;
    friend class FSEmptyBlock;
    friend class FSBootBlock;
    friend class FSRootBlock;
    friend class FSBitmapBlock;
    friend class FSUserDirBlock;
    friend class FSFileHeaderBlock;
    friend class FSFileListBlock;
    friend class FSDataBlock;
    friend class OFSDataBlock;
    friend class FFSDataBlock;
    friend class FSHashTable;

protected:
        
    // The type of this volume
    FSVolumeType type;

    // Total capacity of this volume in blocks
    u32 capacity;
    
    // Size of a single block in bytes
    u32 bsize;
        
    // Number of data bytes in a single data block
    u32 dsize;
    
    // Block storage
    BlockPtr *blocks;
    
    // The directory where new files and subdirectories are added
    u32 currentDir = 0;

    
    //
    // Class methods
    //
    
public:
    
    // Checks if the block with the given number is part of the volume
    bool isBlockNumber(u32 nr) { return nr < capacity; }


    //
    // Factory methods
    //
    
    // Creates a file system with the contents of a host file system directory
    static FSVolume *make(FSVolumeType type, const char *name, const char *path, u32 capacity);
    static FSVolume *make(FSVolumeType type, const char *name, const char *path);

    
    //
    // Initializing
    //
    
public:

    FSVolume(FSVolumeType type, const char *name, u32 capacity, u32 bsize = 512);
    ~FSVolume();
    
    // Prints information about this volume
    void info();
    
    // Prints debug information about this volume
    virtual void dump();
    
    // Checks the integrity of this volume
    virtual bool check(bool verbose);
    
    
    //
    // Querying file system properties
    //
    
    FSVolumeType getType() { return type; }
    bool isOFS() { return type == FS_OFS; }
    bool isFFF() { return type == FS_FFS; }
    FSName getName() { return rootBlock()->getName(); }
    u32 getBlockSize() { return bsize; }
    u32 bytesInDataBlock() { return bsize - (isOFS() ? 24 : 0); }

    u32 getCapacity() { return capacity; }

    // Reports information about the capacity and usage of this volume
    u32 totalBlocks() { return capacity; }
    u32 freeBlocks();
    u32 usedBlocks() { return totalBlocks() - freeBlocks(); }
    u32 totalBytes() { return totalBlocks() * bsize; }
    u32 freeBytes() { return freeBlocks() * bsize; }
    u32 usedBytes() { return usedBlocks() *bsize; }

    
    //
    // Accessing blocks
    //
        
    // Returns the location of the root block and the bitmap block
    u32 rootBlockNr() { return capacity / 2; }
    u32 bitmapBlockNr() { return capacity / 2 + 1; }
    
    // Queries a pointer to a block of a certain type (may return nullptr)
    FSBlock *block(u32 nr);
    FSBootBlock *bootBlock(u32 nr);
    FSRootBlock *rootBlock(u32 nr);
    FSRootBlock *rootBlock() { return rootBlock(rootBlockNr()); }
    FSBitmapBlock *bitmapBlock(u32 nr);
    FSBitmapBlock *bitmapBlock() { return bitmapBlock(bitmapBlockNr()); }
    FSUserDirBlock *userDirBlock(u32 nr);
    FSFileHeaderBlock *fileHeaderBlock(u32 nr);
    FSFileListBlock *fileListBlock(u32 nr);
    FSDataBlock *dataBlock(u32 nr);

    
    //
    // Creating and deleting blocks
    //
    
    // Seeks and free block and marks it as allocated
    u32 allocateBlock();

    // Deallocates a block
    void deallocateBlock(u32 ref);

    // Adds a new block of a certain kind
    u32 addFileListBlock(u32 head, u32 prev);
    u32 addDataBlock(u32 count, u32 head, u32 prev);
    
    // Creates a new block of a certain kind
    FSUserDirBlock *newUserDirBlock(const char *name);
    FSFileHeaderBlock *newFileHeaderBlock(const char *name);
            
    // Installs a boot block
    void installBootBlock();

    
    //
    // Managing directories and files
    //
    
    // Returns the block representing the current directory
    FSBlock *currentDirBlock();
    
    // Changes the current directory
    FSBlock *changeDir(const char *name);

    // Seeks an item inside the current directory
    FSBlock *seek(const char *name);
    FSBlock *seekDir(const char *name);
    FSBlock *seekFile(const char *name);

    // Creates a new directory entry
    FSBlock *makeDir(const char *name);
    FSBlock *makeFile(const char *name);
    FSBlock *makeFile(const char *name, const u8 *buffer, size_t size);
    FSBlock *makeFile(const char *name, const char *str);

        
    //
    // Crawling through the file system
    //

    // Walks through all files in the current directory or a given directory
    int walk(bool recursive);
    int walk(FSBlock *dir, int(FSVolume::*walker)(FSBlock *, int), int value, bool recursive);

    // Walker callbacks
    int listWalker(FSBlock *block, int value);
    
    
    //
    // Importing and exporting
    //
    
    // Exports the volume to a buffer compatible with the ADF format
    bool importVolume(u8 *dst, size_t size);

    // Imports the volume from a buffer compatible with the ADF format
    bool exportVolume(u8 *dst, size_t size);

    // Imports a directory from the host file system
    bool importDirectory(const char *path, bool recursive = true);
    bool importDirectory(const char *path, DIR *dir, bool recursive = true);
};

#endif
