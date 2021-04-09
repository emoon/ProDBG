// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaObject.h"
#include "FSTypes.h"
#include "FSObjects.h"

struct FSBlock : AmigaObject {
        
    // The partition this block belongs to
    struct FSPartition &partition;
    
    // The sector number of this block
    Block nr;
    
    // Outcome of the last integrity check (0 = OK, n = n-th corrupted block)
    isize corrupted = 0;
        
    // The actual block data
    u8 *data = nullptr;

    
    //
    // Constructing
    //
    
    FSBlock(FSPartition &p, Block nr) : partition(p) { this->nr = nr; }
    virtual ~FSBlock() { }

    static FSBlock *makeWithType(FSPartition &p, Block nr, FSBlockType type);
    
    
    //
    // Querying block properties
    //

    // Returns the type of this block
    virtual FSBlockType type() const = 0; 

    // Returns the size of this block in bytes (usually 512)
    isize bsize() const;

    // Extract the file system type from the block header
    virtual FSVolumeType dos() const { return FS_NODOS; }
    
    // Returns the role of a certain byte in this block
    virtual FSItemType itemType(isize byte) const { return FSI_UNKNOWN; }
    
    // Returns the type and subtype identifiers of this block
    virtual u32 typeID() const;
    virtual u32 subtypeID() const;
    
    
    //
    // Integrity checking
    //

    // Scans all long words in this block and returns the number of errors
    isize check(bool strict) const;

    // Checks the integrity of a certain byte in this block
    virtual ErrorCode check(isize pos, u8 *expected, bool strict) const { return ERROR_OK; }
        
    
    //
    // Reading and writing block data
    //

    // Reads or writes a long word in Big Endian format
    static u32 read32(const u8 *p);
    static void write32(u8 *p, u32 value);
    static void inc32(u8 *p) { write32(p, read32(p) + 1); }
    static void dec32(u8 *p) { write32(p, read32(p) - 1); }

    // Computes the address of a long word inside the block
    u8 *addr32(isize nr) const;
    
    // Reads, writes, or modifies the n-th long word
    u32 get32(isize n) const { return read32(addr32(n)); }
    void set32(isize n, u32 val) const { write32(addr32(n), val); }
    void inc32(isize n) const { inc32(addr32(n)); }
    void dec32(isize n) const { dec32(addr32(n)); }

    // Returns the location of the checksum inside this block
    virtual isize checksumLocation() const { return -1; }
    
    // Computes a checksum for this block
    virtual u32 checksum() const;
    
    // Updates the checksum in this block
    void updateChecksum();
    
    
    //
    // Debugging
    //
    
    // Prints some debug information for this block
    virtual void dump() const { };
    virtual void dumpData() const;

    
    //
    // Importing and exporting
    //
    
public:
    
    // Imports this block from a buffer (bsize must match the volume block size)
    virtual void importBlock(const u8 *src, isize bsize);

    // Exports this block to a buffer (bsize must match the volume block size)
    virtual void exportBlock(u8 *dst, isize bsize);
    
    // Exports this block to the host file system
    virtual ErrorCode exportBlock(const char *path) { return ERROR_OK; }
        
                
    //
    // Geting and setting names and comments
    //
    
public:
    
    virtual FSName getName() const { return FSName(""); }
    virtual void setName(FSName name) { }
    virtual bool isNamed(FSName &other) const { return false; }

    virtual FSComment getComment() const { return FSComment(""); }
    virtual void setComment(FSComment name) { }

    
    //
    // Getting and settting date and time
    //
    
    virtual FSTime getCreationDate() const { return FSTime((time_t)0); }
    virtual void setCreationDate(FSTime t) { }

    virtual FSTime getModificationDate() const { return FSTime((time_t)0); }
    virtual void setModificationDate(FSTime t) { }
    
    
    //
    // Getting and setting file properties
    //
    
    virtual u32 getProtectionBits() const { return 0; }
    virtual void setProtectionBits(u32 val) { }

    virtual u32 getFileSize() const { return 0; }
    virtual void setFileSize(u32 val) { }

    
    //
    // Chaining blocks
    //

    // Link to the parent directory block
    virtual Block getParentDirRef() const { return 0; }
    virtual void setParentDirRef(Block ref) { }
    struct FSBlock *getParentDirBlock();
    
    // Link to the file header block
    virtual Block getFileHeaderRef() const { return 0; }
    virtual void setFileHeaderRef(Block ref) { }
    struct FSFileHeaderBlock *getFileHeaderBlock();

    // Link to the next block with the same hash
    virtual Block getNextHashRef() const { return 0; }
    virtual void setNextHashRef(Block ref) { }
    struct FSBlock *getNextHashBlock();

    // Link to the next extension block
    virtual Block getNextListBlockRef() const { return 0; }
    virtual void setNextListBlockRef(Block ref) { }
    struct FSFileListBlock *getNextListBlock();

    // Link to the next bitmap extension block
    virtual Block getNextBmExtBlockRef() const { return 0; }
    virtual void setNextBmExtBlockRef(Block ref) { }
    struct FSBitmapExtBlock *getNextBmExtBlock();
    
    // Link to the first data block
    virtual Block getFirstDataBlockRef() const { return 0; }
    virtual void setFirstDataBlockRef(Block ref) { }
    struct FSDataBlock *getFirstDataBlock();

    // Link to the next data block
    virtual Block getNextDataBlockRef() const { return 0; }
    virtual void setNextDataBlockRef(Block ref) { }
    struct FSDataBlock *getNextDataBlock();

        
    //
    // Working with hash tables
    //
    
    // Returns the hash table size
    virtual isize hashTableSize() const { return 0; }

    // Returns a hash value for this block
    virtual u32 hashValue() const { return 0; }

    // Looks up an item in the hash table
    u32 getHashRef(u32 nr) const;
    void setHashRef(u32 nr, u32 ref);

    // Dumps the contents of the hash table for debugging
    void dumpHashTable() const;


    //
    // Working with bitmap blocks
    //

    
    
    //
    // Working with data blocks
    //
    
    // Returns the maximum number of storable data block references
    isize getMaxDataBlockRefs() const;

    // Gets or sets the number of data block references in this block
    virtual isize getNumDataBlockRefs() const { return 0; }
    virtual void setNumDataBlockRefs(u32 val) { }
    virtual void incNumDataBlockRefs() { }

    // Adds a data block reference to this block
    virtual bool addDataBlockRef(u32 first, u32 ref) { return false; }

    // Adds data bytes to this block
    virtual isize addData(const u8 *buffer, isize size) { return 0; }
};

typedef FSBlock* BlockPtr;


//
// Convenience macros used inside the check() methods
//

#define EXPECT_BYTE(exp) { \
if (value != (exp)) { *expected = (exp); return ERROR_FS_EXPECTED_VALUE; } }

#define EXPECT_LONGWORD(exp) { \
if ((byte % 4) == 0 && BYTE3(value) != BYTE3((u32)exp)) \
    { *expected = (BYTE3((u32)exp)); return ERROR_FS_EXPECTED_VALUE; } \
if ((byte % 4) == 1 && BYTE2(value) != BYTE2((u32)exp)) \
    { *expected = (BYTE2((u32)exp)); return ERROR_FS_EXPECTED_VALUE; } \
if ((byte % 4) == 2 && BYTE1(value) != BYTE1((u32)exp)) \
    { *expected = (BYTE1((u32)exp)); return ERROR_FS_EXPECTED_VALUE; } \
if ((byte % 4) == 3 && BYTE0(value) != BYTE0((u32)exp)) \
    { *expected = (BYTE0((u32)exp)); return ERROR_FS_EXPECTED_VALUE; } }

#define EXPECT_CHECKSUM EXPECT_LONGWORD(checksum())

#define EXPECT_LESS_OR_EQUAL(exp) { \
if (value > (u32)exp) \
{ *expected = (u8)(exp); return ERROR_FS_EXPECTED_SMALLER_VALUE; } }

#define EXPECT_DOS_REVISION { \
if (!FSVolumeTypeEnum::isValid(value)) return ERROR_FS_EXPECTED_DOS_REVISION; }

#define EXPECT_REF { \
if (!partition.dev.block(value)) return ERROR_FS_EXPECTED_REF; }

#define EXPECT_SELFREF { \
if (value != nr) return ERROR_FS_EXPECTED_SELFREF; }

#define EXPECT_FILEHEADER_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_FILEHEADER_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_HASH_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_FILEHEADER_BLOCK, FS_USERDIR_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_HASH_REF { \
if (value) { EXPECT_HASH_REF } }

#define EXPECT_PARENT_DIR_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_ROOT_BLOCK, FS_USERDIR_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_FILELIST_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_FILELIST_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_FILELIST_REF { \
if (value) { EXPECT_FILELIST_REF } }

#define EXPECT_BITMAP_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_BITMAP_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_REF { \
if (value) { EXPECT_BITMAP_REF } }

#define EXPECT_BITMAP_EXT_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_BITMAP_EXT_BLOCK); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_BITMAP_EXT_REF { \
if (value) { EXPECT_BITMAP_EXT_REF } }

#define EXPECT_DATABLOCK_REF { \
if (ErrorCode e = partition.dev.checkBlockType(value, FS_DATA_BLOCK_OFS, FS_DATA_BLOCK_FFS); e != ERROR_OK) return e; }

#define EXPECT_OPTIONAL_DATABLOCK_REF { \
if (value) { EXPECT_DATABLOCK_REF } }

#define EXPECT_DATABLOCK_NUMBER { \
if (value == 0) return ERROR_FS_EXPECTED_DATABLOCK_NR; }

#define EXPECT_HASHTABLE_SIZE { \
if (value != 72) return ERROR_FS_INVALID_HASHTABLE_SIZE; }
