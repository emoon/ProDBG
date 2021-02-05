// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "FSDevice.h"

FSDevice *
FSDevice::makeWithFormat(FSDeviceDescriptor &layout)
{
    FSDevice *dev = new FSDevice(layout.numBlocks);
    
    if (FS_DEBUG) { layout.dump(); }
    
    // Copy layout parameters from descriptor
    dev->numCyls    = layout.numCyls;
    dev->numHeads   = layout.numHeads;
    dev->numSectors = layout.numSectors;
    dev->bsize      = layout.bsize;
    dev->numBlocks  = layout.numBlocks;
        
    // Create all partitions
    for (auto& descriptor : layout.partitions) {
        
        FSPartition *p = FSPartition::makeWithFormat(*dev, descriptor);
        dev->partitions.push_back(p);
    }

    // Compute checksums for all blocks
    dev->updateChecksums();
    
    // Set the current directory to '/'
    dev->cd = dev->partitions[0]->rootBlock;
    
    // Do some consistency checking
    for (u32 i = 0; i < dev->numBlocks; i++) assert(dev->blocks[i] != nullptr);
    
    if (FS_DEBUG) {
        printf("cd = %d\n", dev->cd);
        dev->info();
        dev->dump();
    }
    
    return dev;
}

FSDevice *
FSDevice::makeWithFormat(DiskDiameter type, DiskDensity density)
{
    FSDeviceDescriptor layout = FSDeviceDescriptor(type, density);
    return makeWithFormat(layout);
}

FSDevice *
FSDevice::makeWithADF(ADFFile *adf, ErrorCode *error)
{
    assert(adf != nullptr);

    // Get a device descriptor for the ADF
    FSDeviceDescriptor descriptor = adf->layout();
        
    // Create the device
    FSDevice *volume = makeWithFormat(descriptor);

    // Import file system from ADF
    if (!volume->importVolume(adf->data, adf->size, error)) {
        delete volume;
        return nullptr;
    }
    
    return volume;
}

FSDevice *
FSDevice::makeWithHDF(HDFFile *hdf, ErrorCode *error)
{
    assert(hdf != nullptr);

    // Get a device descriptor for the ADF
    FSDeviceDescriptor descriptor = hdf->layout();

    // Create the device
    FSDevice *volume = makeWithFormat(descriptor);

    volume->info();
    
    // Import file system from HDF
    /*
    if (!volume->importVolume(hdf->getData(), hdf->getSize(), error)) {
        delete volume;
        return nullptr;
    }
    */
    
    return volume;
}

FSDevice *
FSDevice::make(DiskDiameter type, DiskDensity density, const char *path)
{
    FSDevice *device = makeWithFormat(type, density);
    
    if (device) {
        
        // Try to import directory
        if (!device->importDirectory(path)) { delete device; return nullptr; }

        // Assign device name
        device->setName(FSName("Directory")); // TODO: Use last path component

        // Change to the root directory
        device->changeDir("/");
    }

    return device;
}

FSDevice *
FSDevice::make(FSVolumeType type, const char *path)
{
    // Try to fit the directory into files system with DD disk capacity
    if (FSDevice *device = make(INCH_35, DISK_DD, path)) return device;

    // Try to fit the directory into files system with HD disk capacity
    if (FSDevice *device = make(INCH_35, DISK_HD, path)) return device;

    return nullptr;
}
 
FSDevice::FSDevice(isize capacity)
{
    // Initialize the block storage
    blocks.reserve(capacity);
    blocks.assign(capacity, 0);
}

/*
FSDevice::FSDevice(FSDeviceDescriptor &layout)
{
    if (FS_DEBUG) {
        debug("Creating FSDevice...\n");
        layout.dump();
    }
        
    // Copy layout parameters from descriptor
    numCyls    = layout.numCyls;
    numHeads   = layout.numHeads;
    numSectors = layout.numSectors;
    bsize      = layout.bsize;
    numBlocks  = layout.numBlocks;
    
    // Initialize the block storage
    blocks.reserve(layout.numBlocks);
    blocks.assign(layout.numBlocks, 0);
    
    // Create all partitions
    for (auto& descriptor : layout.partitions) {
        partitions.push_back(new FSPartition(*this, descriptor));
    }

    // Compute checksums for all blocks
    updateChecksums();
    
    // Set the current directory to '/'
    cd = partitions[0]->rootBlock;
    
    // Do some consistency checking
    for (u32 i = 0; i < numBlocks; i++) assert(blocks[i] != nullptr);
    
    if (FS_DEBUG) {
        printf("cd = %d\n", cd);
        info();
        dump();
    }
}
*/

FSDevice::~FSDevice()
{
    for (auto &p : partitions) delete p;
    for (auto &b : blocks) delete b;
}

void
FSDevice::info()
{
    msg("Type    Size           Used    Free   Full   Name\n");
    for (auto& p : partitions) p->info();
}

void
FSDevice::dump()
{
    // Dump all partitions
    for (auto &p : partitions) {
        p->dump();
    }
    msg("\n");

    // Dump all blocks
    for (isize i = 0; i < numBlocks; i++)  {
        
        if (blocks[i]->type() == FS_EMPTY_BLOCK) continue;
        
        msg("\nBlock %zu (%d):", i, blocks[i]->nr);
        msg(" %s\n", FSBlockTypeEnum::key(blocks[i]->type()));
                
        blocks[i]->dump(); 
    }
}

isize
FSDevice::partitionForBlock(u32 ref)
{
    for (isize i = 0; i < (isize)partitions.size(); i++) {
        if (ref >= partitions[i]->firstBlock && ref <= partitions[i]->lastBlock) return i;
    }

    assert(false);
    return 0;
}

FSBlockType
FSDevice::blockType(u32 nr)
{
    return blockPtr(nr) ? blocks[nr]->type() : FS_UNKNOWN_BLOCK;
}

FSItemType
FSDevice::itemType(u32 nr, isize pos) const
{
    return blockPtr(nr) ? blocks[nr]->itemType(pos) : FSI_UNUSED;
}

FSBlock *
FSDevice::blockPtr(u32 nr) const 
{
    return nr < blocks.size() ? blocks[nr] : nullptr;
}

FSBootBlock *
FSDevice::bootBlockPtr(u32 nr)
{
    if (nr < blocks.size() && blocks[nr]->type() == FS_BOOT_BLOCK) {
        return (FSBootBlock *)blocks[nr];
    }
    return nullptr;
}

FSRootBlock *
FSDevice::rootBlockPtr(u32 nr)
{
    if (nr < blocks.size() && blocks[nr]->type() == FS_ROOT_BLOCK) {
        return (FSRootBlock *)blocks[nr];
    }
    return nullptr;
}

FSBitmapBlock *
FSDevice::bitmapBlockPtr(u32 nr)
{
    if (nr < blocks.size() && blocks[nr]->type() == FS_BITMAP_BLOCK) {
        return (FSBitmapBlock *)blocks[nr];
    }
    return nullptr;
}

FSBitmapExtBlock *
FSDevice::bitmapExtBlockPtr(u32 nr)
{
    if (nr < blocks.size() && blocks[nr]->type() == FS_BITMAP_EXT_BLOCK) {
        return (FSBitmapExtBlock *)blocks[nr];
    }
    return nullptr;
}

FSUserDirBlock *
FSDevice::userDirBlockPtr(u32 nr)
{
    if (nr < blocks.size() && blocks[nr]->type() == FS_USERDIR_BLOCK) {
        return (FSUserDirBlock *)blocks[nr];
    }
    return nullptr;
}

FSFileHeaderBlock *
FSDevice::fileHeaderBlockPtr(u32 nr)
{
    if (nr < blocks.size() && blocks[nr]->type() == FS_FILEHEADER_BLOCK) {
        return (FSFileHeaderBlock *)blocks[nr];
    }
    return nullptr;
}

FSFileListBlock *
FSDevice::fileListBlockPtr(u32 nr)
{
    if (nr < blocks.size() && blocks[nr]->type() == FS_FILELIST_BLOCK) {
        return (FSFileListBlock *)blocks[nr];
    }
    return nullptr;
}

FSDataBlock *
FSDevice::dataBlockPtr(u32 nr)
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type() : FS_UNKNOWN_BLOCK;

    if (t == FS_DATA_BLOCK_OFS || t == FS_DATA_BLOCK_FFS) {
        return (FSDataBlock *)blocks[nr];
    }
    return nullptr;
}

FSBlock *
FSDevice::hashableBlockPtr(u32 nr)
{
    FSBlockType t = nr < blocks.size() ? blocks[nr]->type() : FS_UNKNOWN_BLOCK;
    
    if (t == FS_USERDIR_BLOCK || t == FS_FILEHEADER_BLOCK) {
        return blocks[nr];
    }
    return nullptr;
}

void
FSDevice::updateChecksums()
{
    for (u32 i = 0; i < numBlocks; i++) {
        blocks[i]->updateChecksum();
    }
}

FSBlock *
FSDevice::currentDirBlock()
{
    FSBlock *cdb = blockPtr(cd);
    
    if (cdb) {
        if (cdb->type() == FS_ROOT_BLOCK || cdb->type() == FS_USERDIR_BLOCK) {
            return cdb;
        }
    }
    
    // The block reference is invalid. Switch back to the root directory
    cd = partitions[cp]->rootBlock;
    return blockPtr(cd);
}

FSBlock *
FSDevice::changeDir(const char *name)
{
    assert(name != nullptr);

    FSBlock *cdb = currentDirBlock();

    if (strcmp(name, "/") == 0) {
                
        // Move to top level
        cd = partitions[cp]->rootBlock;
        return currentDirBlock();
    }

    if (strcmp(name, "..") == 0) {
                
        // Move one level up
        cd = cdb->getParentDirRef();
        return currentDirBlock();
    }
    
    FSBlock *subdir = seekDir(name);
    if (subdir == nullptr) return cdb;
    
    // Move one level down
    cd = subdir->nr;
    return currentDirBlock();
}

string
FSDevice::getPath(FSBlock *block)
{
    string result = "";
    std::set<u32> visited;
 
    while(block) {

        // Break the loop if this block has an invalid type
        if (!hashableBlockPtr(block->nr)) break;

        // Break the loop if this block was visited before
        if (visited.find(block->nr) != visited.end()) break;
        
        // Add the block to the set of visited blocks
        visited.insert(block->nr);
                
        // Expand the path
        string name = block->getName().c_str();
        result = (result == "") ? name : name + "/" + result;
        
        // Continue with the parent block
        block = block->getParentDirBlock();
    }
    
    return result;
}

FSBlock *
FSDevice::makeDir(const char *name)
{
    FSBlock *cdb = currentDirBlock();
    FSUserDirBlock *block = cdb->partition.newUserDirBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);
    
    return block;
}

FSBlock *
FSDevice::makeFile(const char *name)
{
    assert(name != nullptr);
 
    FSBlock *cdb = currentDirBlock();
    FSFileHeaderBlock *block = cdb->partition.newFileHeaderBlock(name);
    if (block == nullptr) return nullptr;
    
    block->setParentDirRef(cdb->nr);
    addHashRef(block->nr);

    return block;
}

FSBlock *
FSDevice::makeFile(const char *name, const u8 *buf, isize size)
{
    assert(buf);

    FSBlock *block = makeFile(name);
    
    if (block) {
        assert(block->type() == FS_FILEHEADER_BLOCK);
        ((FSFileHeaderBlock *)block)->addData(buf, size);
    }
    
    return block;
}

FSBlock *
FSDevice::makeFile(const char *name, const char *str)
{
    assert(str != nullptr);
    
    return makeFile(name, (const u8 *)str, strlen(str));
}

u32
FSDevice::seekRef(FSName name)
{
    std::set<u32> visited;
    
    // Only proceed if a hash table is present
    FSBlock *cdb = currentDirBlock();
    if (!cdb || cdb->hashTableSize() == 0) return 0;
    
    // Compute the table position and read the item
    u32 hash = name.hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);
    
    // Traverse the linked list until the item has been found
    while (ref && visited.find(ref) == visited.end())  {
        
        FSBlock *item = hashableBlockPtr(ref);
        if (item == nullptr) break;
        
        if (item->isNamed(name)) return item->nr;

        visited.insert(ref);
        ref = item->getNextHashRef();
    }

    return 0;
}

void
FSDevice::addHashRef(u32 ref)
{
    if (FSBlock *block = hashableBlockPtr(ref)) {
        addHashRef(block);
    }
}

void
FSDevice::addHashRef(FSBlock *newBlock)
{
    // Only proceed if a hash table is present
    FSBlock *cdb = currentDirBlock();
    if (!cdb || cdb->hashTableSize() == 0) { return; }

    // Read the item at the proper hash table location
    u32 hash = newBlock->hashValue() % cdb->hashTableSize();
    u32 ref = cdb->getHashRef(hash);

    // If the slot is empty, put the reference there
    if (ref == 0) { cdb->setHashRef(hash, newBlock->nr); return; }

    // Otherwise, put it into the last element of the block list chain
    FSBlock *last = lastHashBlockInChain(ref);
    if (last) last->setNextHashRef(newBlock->nr);
}

void
FSDevice::printDirectory(bool recursive)
{
    std::vector<u32> items;
    collect(cd, items);
    
    for (auto const& i : items) {
        msg("%s\n", getPath(i).c_str());
    }
    msg("%lu items\n", items.size());
}


FSBlock *
FSDevice::lastHashBlockInChain(u32 start)
{
    FSBlock *block = hashableBlockPtr(start);
    return block ? lastHashBlockInChain(block) : nullptr;
}

FSBlock *
FSDevice::lastHashBlockInChain(FSBlock *block)
{
    std::set<u32> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSBlock *next = block->getNextHashBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block =next;
    }
    return nullptr;
}

FSBlock *
FSDevice::lastFileListBlockInChain(u32 start)
{
    FSBlock *block = fileListBlockPtr(start);
    return block ? lastFileListBlockInChain(block) : nullptr;
}

FSBlock *
FSDevice::lastFileListBlockInChain(FSBlock *block)
{
    std::set<u32> visited;

    while (block && visited.find(block->nr) == visited.end()) {

        FSFileListBlock *next = block->getNextListBlock();
        if (next == nullptr) return block;

        visited.insert(block->nr);
        block = next;
    }
    return nullptr;
}

ErrorCode
FSDevice::collect(u32 ref, std::vector<u32> &result, bool recursive)
{
    std::stack<u32> remainingItems;
    std::set<u32> visited;
    
    // Start with the items in this block
    collectHashedRefs(ref, remainingItems, visited);
    
    // Move the collected items to the result list
    while (remainingItems.size() > 0) {
        
        u32 item = remainingItems.top();
        remainingItems.pop();
        result.push_back(item);

        // Add subdirectory items to the queue
        if (userDirBlockPtr(item) && recursive) {
            collectHashedRefs(item, remainingItems, visited);
        }
    }

    return ERROR_OK;
}

ErrorCode
FSDevice::collectHashedRefs(u32 ref, std::stack<u32> &result, std::set<u32> &visited)
{
    if (FSBlock *b = blockPtr(ref)) {
        
        // Walk through the hash table in reverse order
        for (isize i = (isize)b->hashTableSize(); i >= 0; i--) {
            collectRefsWithSameHashValue(b->getHashRef((u32)i), result, visited);
        }
    }
    
    return ERROR_OK;
}

ErrorCode
FSDevice::collectRefsWithSameHashValue(u32 ref, std::stack<u32> &result, std::set<u32> &visited)
{
    std::stack<u32> refs;
    
    // Walk down the linked list
    for (FSBlock *b = hashableBlockPtr(ref); b; b = b->getNextHashBlock()) {

        // Break the loop if we've already seen this block
        if (visited.find(b->nr) != visited.end()) return ERROR_FS_HAS_CYCLES;
        visited.insert(b->nr);

        refs.push(b->nr);
    }
  
    // Push the collected elements onto the result stack
    while (refs.size() > 0) { result.push(refs.top()); refs.pop(); }
    
    return ERROR_OK;
}

FSErrorReport
FSDevice::check(bool strict) const
{
    FSErrorReport result;

    isize total = 0, min = LONG_MAX, max = 0;
    
    // Analyze all partions
    for (auto &p : partitions) p->check(strict, result);

    // Analyze all blocks
    for (u32 i = 0; i < numBlocks; i++) {

        if (blocks[i]->check(strict) > 0) {
            min = MIN(min, i);
            max = MAX(max, i);
            blocks[i]->corrupted = ++total;
        } else {
            blocks[i]->corrupted = 0;
        }
    }

    // Record findings
    if (total) {
        result.corruptedBlocks = total;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    } else {
        result.corruptedBlocks = 0;
        result.firstErrorBlock = min;
        result.lastErrorBlock = max;
    }
    
    return result;
}

ErrorCode
FSDevice::check(u32 blockNr, isize pos, u8 *expected, bool strict) const
{
    return blocks[blockNr]->check(pos, expected, strict);
}

ErrorCode
FSDevice::checkBlockType(u32 nr, FSBlockType type)
{
    return checkBlockType(nr, type, type);
}

ErrorCode
FSDevice::checkBlockType(u32 nr, FSBlockType type, FSBlockType altType)
{
    FSBlockType t = blockType(nr);
    
    if (t != type && t != altType) {
        
        switch (t) {
                
            case FS_EMPTY_BLOCK:      return ERROR_FS_PTR_TO_EMPTY_BLOCK;
            case FS_BOOT_BLOCK:       return ERROR_FS_PTR_TO_BOOT_BLOCK;
            case FS_ROOT_BLOCK:       return ERROR_FS_PTR_TO_ROOT_BLOCK;
            case FS_BITMAP_BLOCK:     return ERROR_FS_PTR_TO_BITMAP_BLOCK;
            case FS_BITMAP_EXT_BLOCK: return ERROR_FS_PTR_TO_BITMAP_EXT_BLOCK;
            case FS_USERDIR_BLOCK:    return ERROR_FS_PTR_TO_USERDIR_BLOCK;
            case FS_FILEHEADER_BLOCK: return ERROR_FS_PTR_TO_FILEHEADER_BLOCK;
            case FS_FILELIST_BLOCK:   return ERROR_FS_PTR_TO_FILELIST_BLOCK;
            case FS_DATA_BLOCK_OFS:   return ERROR_FS_PTR_TO_DATA_BLOCK;
            case FS_DATA_BLOCK_FFS:   return ERROR_FS_PTR_TO_DATA_BLOCK;
            default:                  return ERROR_FS_PTR_TO_UNKNOWN_BLOCK;
        }
    }

    return ERROR_OK;
}

isize
FSDevice::getCorrupted(u32 blockNr)
{
    return blockPtr(blockNr) ? blocks[blockNr]->corrupted : 0;
}

bool
FSDevice::isCorrupted(u32 blockNr, isize n)
{
    for (u32 i = 0, cnt = 0; i < numBlocks; i++) {
        
        if (isCorrupted(i)) {
            cnt++;
            if (blockNr == i) return cnt == n;
        }
    }
    return false;
}

isize
FSDevice::nextCorrupted(u32 blockNr)
{
    isize i = (isize)blockNr;
    while (++i < numBlocks) { if (isCorrupted((u32)i)) return i; }
    return blockNr;
}

isize
FSDevice::prevCorrupted(u32 blockNr)
{
    isize i = (isize)blockNr - 1;
    while (i-- >= 0) { if (isCorrupted((u32)i)) return i; }
    return blockNr;
}

u32
FSDevice::seekCorruptedBlock(isize n)
{
    for (u32 i = 0, cnt = 0; i < numBlocks; i++) {

        if (isCorrupted(i)) {
            cnt++;
            if (cnt == n) return i;
        }
    }
    return (u32)-1;
}

u8
FSDevice::readByte(u32 block, isize offset) const
{
    assert(offset < bsize);

    if (block < numBlocks) {
        return blocks[block]->data ? blocks[block]->data[offset] : 0;
    }
    
    return 0;
}

FSBlockType
FSDevice::predictBlockType(u32 nr, const u8 *buffer)
{
    assert(buffer != nullptr);
    
    for (auto &p : partitions) {
        if (FSBlockType t = p->predictBlockType(nr, buffer); t != FS_UNKNOWN_BLOCK) {
            return t;
        }
    }
    return FS_UNKNOWN_BLOCK;
}

bool
FSDevice::importVolume(const u8 *src, isize size)
{
    ErrorCode error;
    bool result = importVolume(src, size, &error);
    
    assert(result == (error == ERROR_OK));
    return result;
}

bool
FSDevice::importVolume(const u8 *src, isize size, ErrorCode *err)
{
    assert(src != nullptr);

    debug(FS_DEBUG, "Importing file system...\n");

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) {
        if (err) *err = ERROR_FS_WRONG_BSIZE;
        return false;
    }
    // Only proceed if the source buffer contains the right amount of data
    if (numBlocks * bsize != size) {
        if (err) *err = ERROR_FS_WRONG_CAPACITY;
        return false;
    }
    // Only proceed if all partitions contain a valid file system
    for (auto &it : partitions) {
        if (it->dos == FS_NODOS) {
            if (err) *err = ERROR_FS_UNSUPPORTED;
            return false;
        }
    }
        
    // Import all blocks
    for (u32 i = 0; i < numBlocks; i++) {
        
        const u8 *data = src + i * bsize;
        
        // Get the partition this block belongs to
        FSPartition &p = blocks[i]->partition;
        
        // Determine the type of the new block
        FSBlockType type = p.predictBlockType(i, data);
        
        // Create new block
        FSBlock *newBlock = FSBlock::makeWithType(p, i, type);
        if (newBlock == nullptr) return false;

        // Import block data
        newBlock->importBlock(data, bsize);

        // Replace the existing block
        assert(blocks[i] != nullptr);
        delete blocks[i];
        blocks[i] = newBlock;
    }
    
    if (err) *err = ERROR_OK;
    debug(FS_DEBUG, "Success\n");
    info();
    dump();
    hexdump(blocks[0]->data, 512);
    printDirectory(true);
    return true;
}

bool
FSDevice::exportVolume(u8 *dst, isize size)
{
    return exportBlocks(0, numBlocks - 1, dst, size);
}

bool
FSDevice::exportVolume(u8 *dst, isize size, ErrorCode *err)
{
    return exportBlocks(0, numBlocks - 1, dst, size, err);
}

bool
FSDevice::exportBlock(isize nr, u8 *dst, isize size)
{
    return exportBlocks(nr, nr, dst, size);
}

bool
FSDevice::exportBlock(isize nr, u8 *dst, isize size, ErrorCode *error)
{
    return exportBlocks(nr, nr, dst, size, error);
}

bool
FSDevice::exportBlocks(isize first, isize last, u8 *dst, isize size)
{
    ErrorCode error;
    bool result = exportBlocks(first, last, dst, size, &error);
    
    assert(result == (error == ERROR_OK));
    return result;
}

bool
FSDevice::exportBlocks(isize first, isize last, u8 *dst, isize size, ErrorCode *err)
{
    assert(last < numBlocks);
    assert(first <= last);
    assert(dst);
    
    isize count = last - first + 1;
    
    debug(FS_DEBUG, "Exporting %zd blocks (%zd - %zd)\n", count, first, last);

    // Only proceed if the (predicted) block size matches
    if (size % bsize != 0) {
        if (err) *err = ERROR_FS_WRONG_BSIZE;
        return false;
    }

    // Only proceed if the source buffer contains the right amount of data
    if (count * bsize != size) {
        if (err) *err = ERROR_FS_WRONG_CAPACITY;
        return false;
    }
        
    // Wipe out the target buffer
    memset(dst, 0, size);
    
    // Export all blocks
    for (u32 i = 0; i < count; i++) {
        
        blocks[first + i]->exportBlock(dst + i * bsize, bsize);
    }

    debug(FS_DEBUG, "Success\n");

    if (err) *err = ERROR_OK;
    return true;
}

bool
FSDevice::importDirectory(const char *path, bool recursive)
{
    assert(path);
    
    if (DIR *dir = opendir(path)) {
        
        bool result = importDirectory(path, dir, recursive);
        closedir(dir);
        return result;
    }

    warn("Error opening directory %s\n", path);
    return false;
}

bool
FSDevice::importDirectory(const char *path, DIR *dir, bool recursive)
{
    assert(dir);
    
    struct dirent *item;
    bool result = true;
    
    while ((item = readdir(dir))) {

        // Skip '.', '..' and all hidden files
        if (item->d_name[0] == '.') continue;

        // Assemble file name
        char *name = new char [strlen(path) + strlen(item->d_name) + 2];
        strcpy(name, path);
        strcat(name, "/");
        strcat(name, item->d_name);

        msg("importDirectory: Processing %s\n", name);
        
        if (item->d_type == DT_DIR) {
            
            // Add directory
            result &= makeDir(item->d_name) != nullptr;
            if (recursive && result) {
                changeDir(item->d_name);
                result &= importDirectory(name, recursive);
            }
            
        } else {
            
            // Add file
            u8 *buffer; long size;
            if (loadFile(name, &buffer, &size)) {
                FSBlock *file = makeFile(item->d_name, buffer, size);
                // result &= file ? (file->append(buffer, size)) : false;
                result &= file != nullptr;
                delete(buffer);
            }
        }
        
        delete [] name;
    }

    return result;
}

ErrorCode
FSDevice::exportDirectory(const char *path)
{
    assert(path != nullptr);
        
    // Only proceed if path points to an empty directory
    long numItems = numDirectoryItems(path);
    if (numItems != 0) return ERROR_FS_DIRECTORY_NOT_EMPTY;
    
    // Collect files and directories
    std::vector<u32> items;
    collect(cd, items);
    
    // Export all items
    for (auto const& i : items) {
        if (ErrorCode error = blockPtr(i)->exportBlock(path); error != ERROR_OK) {
            msg("Export error: %lld\n", error);
            return error; 
        }
    }
    
    msg("Exported %lu items", items.size());
    return ERROR_OK;
}
