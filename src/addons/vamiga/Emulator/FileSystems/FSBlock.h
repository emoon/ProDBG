// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _FS_BLOCKS_H
#define _FS_BLOCKS_H

#include "Utils.h"
#include "FSTypes.h"
#include "FSObjects.h"

struct FSBlock {
    
    // The volume this block belongs to
    class FSVolume &volume;
    
    // The sector number of this block
    u32 nr;

    // The block data
    u8 *data = nullptr;
    
    
    //
    // Constants and static methods
    //

    // Search limit for avoiding infinite loops in list walks
    static const long searchLimit = 255;
        
    // Reads or writes a long word in Big Endian format
    static  u32 read32(const u8 *p);
    static void write32(u8 *p, u32 value);
    static void inc32(u8 *p) { write32(p, read32(p) + 1); }
    static void dec32(u8 *p) { write32(p, read32(p) - 1); }

        
    //
    // Constructing and destructing
    //
    
    FSBlock(FSVolume &ref, u32 nr) : volume(ref) { this->nr = nr; }
    virtual ~FSBlock() { }

    
    //
    // Accessing block parameters
    //

    // Returns the type of this block
    virtual FSBlockType type() = 0; 

    // Returns the size of this block
    u32 bsize();
    
    // Returns the name or path of this block
    char *assemblePath();

    
    //
    // Reading and writing block data
    //

    // Computes the address of a long word inside the block
    u8 *addr(int nr); 
    
    // Reads, writes, or modifies the n-th long word
    u32 get32(i32 n) { return read32(addr(n)); }
    void set32(i32 n, u32 val) { write32(addr(n), val); }
    void inc32(i32 n) { inc32(addr(n)); }
    void dec32(i32 n) { dec32(addr(n)); }

    // Computes a checksum for this block
    u32 checksum();

    
    //
    // Debugging
    //
    
    // Prints the full path of this block
    void printPath();

    // Prints a debug summary for this block
    virtual void dump() { };
    
    
    //
    // Verifying
    //
    
    // Checks the integrity of this block
    virtual bool check(bool verbose);

protected:
    
    // Performs a certain integrity check on a block reference
    bool assertNotNull(u32 ref, bool verbose);
    bool assertInRange(u32 ref, bool verbose);
    bool assertHasType(u32 ref, FSBlockType type, bool verbose);
    bool assertHasType(u32 ref, FSBlockType type, FSBlockType optType, bool verbose);
    bool assertSelfRef(u32 ref, bool verbose);

    
    //
    // Importing and exporting
    //
    
public:
    
    // Imports this block from a buffer (bsize must match the volume block size)
    virtual void importBlock(u8 *p, size_t bsize);

    // Exports this block to a buffer (bsize must match the volume block size)
    virtual void exportBlock(u8 *p, size_t bsize);

private:
    
    // Updates the checksum for this block (called prior to exporting)
    virtual void updateChecksum() { }
    
    
    //
    // Method stubs for blocks representing file items
    //
    
public:
    
    // Return true if the name of this block matches the given name
    virtual bool matches(FSName &otherName) { return false; }
        
    
    //
    // Method stubs for blocks maintaining a data block list
    //

public:

    virtual u32 blockListCapacity() { return 0; }
    virtual u32 blockListSize() { return 0; }
    virtual bool addDataBlockRef(u32 ref) { return false; }
    virtual bool addDataBlockRef(u32 first, u32 ref) { return false; }
    virtual void deleteDataBlockRefs() { }

    //
    // Getting and setting names and comments
    //
    
    virtual FSName getName() { return FSName(""); }
    virtual void setName(FSName name) { }

    virtual FSComment getComment() { return FSComment(""); }
    virtual void setComment(FSComment name) { }

    
    //
    // Getting and settting date and time
    //
    
    virtual FSTime getCreationDate() { return FSTime((time_t)0); }
    virtual void setCreationDate(FSTime t) { }

    virtual FSTime getModificationDate() { return FSTime((time_t)0); }
    virtual void setModificationDate(FSTime t) { }
    
    
    //
    // Getting and setting protection bits
    //
    
    virtual u32 getProtectionBits() { return 0; }
    virtual void setProtectionBits(u32 val) { }

    virtual u32 getFileSize() { return 0; }
    virtual void setFileSize(u32 val) { }

    
    //
    // Chaining blocks
    //

    // Gets or sets a reference to a the parent directory block
    virtual u32 getParentDirRef() { return 0; }
    virtual void setParentDirRef(u32 ref) { }
    FSBlock *getParentBlock();
    
    // Gets or sets a reference to a file header block
    virtual u32 getFileHeaderRef() { return 0; }
    virtual void setFileHeaderRef(u32 ref) { }
    struct FSFileHeaderBlock *getFileHeaderBlock();

    // Gets or sets a reference to the first data block
    virtual u32 getFirstDataBlockRef() { return 0; }
    virtual void setFirstDataBlockRef(u32 ref) { }
    struct FSDataBlock *getFirstDataBlock();

    // Gets or sets a reference to the next data block
    virtual u32 getNextDataBlockRef() { return 0; }
    virtual void setNextDataBlockRef(u32 ref) { }
    struct FSDataBlock *getNextDataBlock();

    // Gets or sets a reference to the next block with the same hash
    virtual u32 getNextHashRef() { return 0; }
    virtual void setNextHashRef(u32 ref) { }
    FSBlock *getNextHashBlock();

    // Returns a reference or a pointer to the next extension block
    virtual u32 getNextListBlockRef() { return 0; }
    virtual void setNextListBlockRef(u32 ref) { }
    struct FSFileListBlock *getNextExtensionBlock();

    
    //
    // Working with hash tables
    //
    
    // Returns the hash table size
    virtual u32 hashTableSize() { return 0; }

    // Returns a hash value for this block
    virtual u32 hashValue() { return 0; }

    // Looks up an item in the hash table
    u32 lookup(u32 nr);
    FSBlock *lookup(FSName name);

    // Adds a reference to the hash table
    void addToHashTable(u32 ref);
    
    // Checks the integrity of the hash table
    bool checkHashTable(bool verbose);
    
    // Dumps the contents of the hash table for debugging
    void dumpHashTable();

    
    //
    // Working with data blocks and file data
    //
    
    // Returns the number of data block references in this block
    virtual u32 numDataBlockRefs() { return 0; }

    // Returns the maximum number of storable data block references
    virtual u32 maxDataBlockRefs() { return 0; }
    
    // Returns the maximum number of storable data block references
    virtual void incDataBlockRefs() { }

    // Sets the data block number (first block is numbered 1)
    // virtual void setDataBlockNr(u32 nr) { }
    
    // Adds raw file data to this block
    virtual size_t addData(const u8 *buffer, size_t size) { return 0; }
};

typedef FSBlock* BlockPtr;

#endif
