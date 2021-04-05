// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "DIRFile.h"
#include "FSVolume.h"

DIRFile::DIRFile()
{
    setDescription("DIRFile");
}


bool
DIRFile::isDIRFile(const char *path)
{
    assert(path != nullptr);
    return isDirectory(path);
}

bool
DIRFile::bufferHasSameType(const u8 *buffer, size_t length)
{
    assert(false);
    return false;
}

DIRFile *
DIRFile::makeWithFile(const char *path)
{
    DIRFile *dir = new DIRFile();
    
    if (!dir->readFromFile(path)) {
        delete dir;
        return nullptr;
    }
    
    return dir;
}

bool
DIRFile::readFromBuffer(const u8 *buffer, size_t length)
{
    assert(false);
    return false;
}

bool
DIRFile::readFromFile(const char *filename)
{
    debug("DIRFile::readFromFile(%s)\n", filename);
              
    // Only proceed if the provided filename points to a directory
    if (!isDIRFile(filename)) {
        warn("%s is not a directory\n", filename);
        return false;
    }
    
    // Create a file system and import the directory
    FSVolume *volume = FSVolume::make(FS_OFS, "Disk", filename);
    if (!volume) {
        warn("Contents of %s does not fit on a disk\n", filename);
        return false;
    }
    
    // Check the file system for errors
    volume->info();
    volume->walk(true);

    if (!volume->check(MFM_DEBUG)) {
        warn("DIRFile::readFromFile: Files system is corrupted.\n");
    }
    volume->dump();
    
    // Convert the file system into an ADF
    assert(adf == nullptr);
    adf = ADFFile::makeWithVolume(*volume);
    debug("adf = %p\n", adf); 

    delete volume;
    return adf != nullptr;
}
