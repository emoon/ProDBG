// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"
#include "FSVolume.h"

u8 *
FSBlock::addr(int nr)
{
    return (data + 4 * nr) + (nr < 0 ? volume.bsize : 0);
}

u32
FSBlock::read32(const u8 *p)
{
    return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}
    
void
FSBlock::write32(u8 *p, u32 value)
{
    p[0] = (value >> 24) & 0xFF;
    p[1] = (value >> 16) & 0xFF;
    p[2] = (value >>  8) & 0xFF;
    p[3] = (value >>  0) & 0xFF;
}

/*
time_t
FSBlock::readTimeStamp(u8 *p)
{
    const u32 secPerDay = 24 * 60 * 60;

    u32 days = read32(p + 0);
    u32 mins = read32(p + 4);
    u32 ticks = read32(p + 8);
    
    time_t t = days * secPerDay + mins * 60 + ticks / 50;
    
    // Shift reference point from  Jan 1, 1978 (Amiga) to Jan 1, 1970 (Unix)
    t += (8 * 365 + 2) * secPerDay - 60 * 60;
    
    return t;
}

void
FSBlock::writeTimeStamp(u8 *p, time_t t)
{
    const u32 secPerDay = 24 * 60 * 60;
    
    // Shift reference point from Jan 1, 1970 (Unix) to Jan 1, 1978 (Amiga)
    t -= (8 * 365 + 2) * secPerDay - 60 * 60;
    
    u32 days = t / secPerDay;
    u32 mins = (t % secPerDay) / 60;
    u32 ticks = (t % secPerDay % 60) * 50;

    write32(p + 0, days);
    write32(p + 4, mins);
    write32(p + 8, ticks);
}
*/

u32
FSBlock::checksum()
{
    u32 result = 0;
    u32 numLongWords = volume.bsize / 4;
    
    for (u32 i = 0; i < numLongWords; i++) {
        result += get32(i);
    }
    
    return ~result + 1;
}

u32
FSBlock::bsize()
{
    return volume.bsize;
}

char *
FSBlock::assemblePath()
{
    FSBlock *parent = getParentBlock();
    if (!parent) return strdup("");
    
    FSName name = getName();
    
    char *prefix = parent->assemblePath();
    char *result = new char [strlen(prefix) + strlen(name.cStr) + 2];

    strcpy(result, prefix);
    strcat(result, "/");
    strcat(result, name.cStr);

    delete [] prefix;
    return result;
}

void
FSBlock::printPath()
{
    char *path = assemblePath();
    printf("%s", path);
    delete [] path;
}

bool
FSBlock::check(bool verbose)
{
    return assertSelfRef(nr, verbose);
}

bool
FSBlock::assertNotNull(u32 ref, bool verbose)
{
    if (ref != 0) return true;
    
    if (verbose) fprintf(stderr, "Block reference is missing.\n");
    return false;
}

bool
FSBlock::assertInRange(u32 ref, bool verbose)
{
    if (volume.isBlockNumber(ref)) return true;

    if (verbose) fprintf(stderr, "Block reference %d is invalid\n", ref);
    return false;
}

bool
FSBlock::assertHasType(u32 ref, FSBlockType type, bool verbose)
{
    return assertHasType(ref, type, type, verbose);
}

bool
FSBlock::assertHasType(u32 ref, FSBlockType type1, FSBlockType type2, bool verbose)
{
    assert(isFSBlockType(type1));
    assert(isFSBlockType(type2));

    FSBlock *block = volume.block(ref);
    FSBlockType type = block ? block->type() : FS_EMPTY_BLOCK;
    
    if (!isFSBlockType(type)) {
        if (verbose) fprintf(stderr, "Block type %ld is not a known type.\n", type);
        return false;
    }
    
    if (block && (type == type1 || type == type2)) return true;
    
    if (verbose && type1 == type2) {
        fprintf(stderr, "Block %d has type %s. Expected %s.\n",
                ref,
                fsBlockTypeName(type),
                fsBlockTypeName(type1));
    }
    
    if (verbose && type1 != type2) {
        fprintf(stderr, "Block %d has type %s. Expected %s or %s.\n",
                ref,
                fsBlockTypeName(type),
                fsBlockTypeName(type1),
                fsBlockTypeName(type2));
    }

    return false;
}

bool
FSBlock::assertSelfRef(u32 ref, bool verbose)
{
    if (ref == nr && volume.block(ref) == this) return true;

    if (ref != nr && verbose) {
        fprintf(stderr, "%d is not a self-reference.\n", ref);
    }
    
    if (volume.block(ref) != this && verbose) {
        fprintf(stderr, "Array element %d references an invalid block\n", ref);
    }
    
    return false;
}

void
FSBlock::importBlock(u8 *p, size_t bsize)
{
    assert(bsize == volume.bsize);
}

void
FSBlock::exportBlock(u8 *p, size_t bsize)
{
    assert(bsize == volume.bsize);
            
    // Rectify the checksum
    updateChecksum();

    // Export the block
    assert(p && data);
    memcpy(p, data, bsize);
}

FSBlock *
FSBlock::getParentBlock()
{
    u32 ref = getParentDirRef();
    return ref ? volume.block(ref) : nullptr;
}

FSFileHeaderBlock *
FSBlock::getFileHeaderBlock()
{
    u32 ref = getFileHeaderRef();
    return ref ? volume.fileHeaderBlock(ref) : nullptr;
}

FSDataBlock *
FSBlock::getFirstDataBlock()
{
    u32 ref = getFirstDataBlockRef();
    return ref ? volume.dataBlock(ref) : nullptr;
}

FSDataBlock *
FSBlock::getNextDataBlock()
{
    u32 ref = getNextDataBlockRef();
    return ref ? volume.dataBlock(ref) : nullptr;
}

FSBlock *
FSBlock::getNextHashBlock()
{
    u32 ref = getNextHashRef();
    return ref ? volume.block(ref) : nullptr;
}

FSFileListBlock *
FSBlock::getNextExtensionBlock()
{
    u32 ref = getNextListBlockRef();
    return ref ? volume.fileListBlock(ref) : nullptr;
}

u32
FSBlock::lookup(u32 nr)
{
    return (nr < hashTableSize()) ? read32(data + 24 + 4 * nr) : 0;
}

FSBlock *
FSBlock::lookup(FSName name)
{
    // Don't call this function if no hash table is present
    assert(hashTableSize() != 0);

    // Compute hash value and table position
    u32 hash = name.hashValue() % hashTableSize();
    u8 *tableEntry = data + 24 + 4 * hash;
    
    // Read the entry
    u32 blockRef = read32(tableEntry);
    assert(lookup(hash) == blockRef);
    FSBlock *block = blockRef ? volume.block(blockRef) : nullptr;
    
    // Traverse the linked list until the item has been found
    for (int i = 0; block && i < searchLimit; i++) {

        if (block->matches(name)) return block;
        block = block->getNextHashBlock();
    }

    return nullptr;
}

void
FSBlock::addToHashTable(u32 ref)
{
    FSBlock *block = volume.block(ref);
    if (block == nullptr) return;
    
    // Don't call this function if no hash table is present
    assert(hashTableSize() != 0);
        
    // Compute hash value and table position
    u32 hash = block->hashValue() % hashTableSize();
    u8 *tableEntry = data + 24 + 4 * hash;
    
    // If the hash table slot is empty, put the reference there
    if (read32(tableEntry) == 0) { write32(tableEntry, ref); return; }
    
    // Otherwise, add the reference at the end of the linked list
    if (auto item = volume.block(read32(tableEntry))) {
        
        for (int i = 0; i < searchLimit; i++) {
            
            if (item->getNextHashBlock() == nullptr) {
                item->setNextHashRef(ref);
                return;
            }
            
            item = item->getNextHashBlock();
        }
    }
}

bool
FSBlock::checkHashTable(bool verbose)
{
    bool result = true;
    
    for (u32 i = 0; i < hashTableSize(); i++) {
        
        if (u32 ref = read32(data + 24 + 4 * i)) {
            result &= assertInRange(ref, verbose);
            result &= assertHasType(ref, FS_USERDIR_BLOCK, FS_FILEHEADER_BLOCK, verbose);
        }
    }
    return result;
}

void
FSBlock::dumpHashTable()
{
    for (u32 i = 0; i < hashTableSize(); i++) {
        
        u32 value = read32(data + 24 + 4 * i);
        if (value) {
            printf("%d: %d ", i, value);
        }
    }
}
