// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "EXEFile.h"
#include "FSVolume.h"

EXEFile::EXEFile()
{
    setDescription("EXEFile");
}

bool
EXEFile::isEXEBuffer(const u8 *buffer, size_t length)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };
                                                                                            
    assert(buffer != nullptr);
    
    // Only accept the file if it fits onto a HD disk
    if (length > 1710000) return false;

    return matchingBufferHeader(buffer, signature, sizeof(signature));
}

bool
EXEFile::isEXEFile(const char *path)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };
    
    assert(path != nullptr);
    
    return matchingFileHeader(path, signature, sizeof(signature));
}

EXEFile *
EXEFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    EXEFile *exe = new EXEFile();
    
    if (!exe->readFromBuffer(buffer, length)) {
        delete exe;
        return NULL;
    }
    
    return exe;
}

EXEFile *
EXEFile::makeWithFile(const char *path)
{
    EXEFile *exe = new EXEFile();
    
    if (!exe->readFromFile(path)) {
        delete exe;
        return NULL;
    }
    
    return exe;
}

bool
EXEFile::readFromBuffer(const u8 *buffer, size_t length)
{    
    bool success = false;
    
    if (!isEXEBuffer(buffer, length))
        return false;
    
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
        
    // Check if this file requires an HD disk
    bool hd = length > 853000;
    
    // Create a new file system 
    FSVolume volume = FSVolume(FS_OFS, "Disk", hd ? 4 * 880 : 2 * 880);
    
    // Make the volume bootable
    volume.installBootBlock();
    
    // Add the executable
    FSBlock *file = volume.makeFile("file", buffer, length);
    // if (file) success = file->append(buffer, length);
    success = file != nullptr;
    
    // Add a script directory
    volume.makeDir("s");
    volume.changeDir("s");
    
    // Add a startup sequence
    file = volume.makeFile("startup-sequence", "file");
    // if (success && file) success = file->append("file");
    success &= file != nullptr; 
    
    // Check for file system errors
    volume.changeDir("/");
    volume.info();
    volume.walk(true);
    
    if (!volume.check(MFM_DEBUG)) {
        warn("EXEFile::readFromBuffer: Files system is corrupted.\n");
        // volume.dump();
    }
    // volume.dump();
    
    // Convert the volume into an ADF
    assert(adf == nullptr);
    if (success) adf = ADFFile::makeWithVolume(volume);
    return adf != nullptr;
}
