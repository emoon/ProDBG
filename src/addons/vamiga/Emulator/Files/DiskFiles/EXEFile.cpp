// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "EXEFile.h"
#include "FSDevice.h"

bool
EXEFile::isCompatibleName(const string &name)
{
    return name == "exe" || name == "EXE";
}

bool
EXEFile::isCompatibleStream(std::istream &stream)
{
    u8 signature[] = { 0x00, 0x00, 0x03, 0xF3 };
                                                                                            
    // Only accept the file if it fits onto a HD disk
    if (streamLength(stream) > 1710000) return false;

    return matchingStreamHeader(stream, signature, sizeof(signature));
}

isize
EXEFile::readFromStream(std::istream &stream)
{
    bool success = false;
    
    isize result = AmigaFile::readFromStream(stream);
    
    // Check if this file requires an HD disk
    bool hd = size > 853000;
        
    // Create a new file system
    FSDevice *volume = FSDevice::makeWithFormat(INCH_35, hd ? DISK_HD : DISK_DD);
    volume->setName(FSName("Disk"));
    
    // Make the volume bootable
    volume->makeBootable(0);
    
    // Add the executable
    FSBlock *file = volume->makeFile("file", data, size);
    success = file != nullptr;
    
    // Add a script directory
    volume->makeDir("s");
    volume->changeDir("s");
    
    // Add a startup sequence
    file = volume->makeFile("startup-sequence", "file");
    success &= file != nullptr;
    
    // Finalize
    volume->updateChecksums();
    
    // Check for file system errors
    volume->changeDir("/");
    volume->info();
    volume->printDirectory(true);

    // Check the file system for consistency
    FSErrorReport report = volume->check(true);
    if (report.corruptedBlocks > 0) {
        warn("Found %ld corrupted blocks\n", report.corruptedBlocks);
        volume->dump();
    }
    
    // Convert the volume into an ADF
    if (success) {
        ErrorCode fsError;
        assert(adf == nullptr);
        adf = ADFFile::makeWithVolume(*volume, &fsError);
        if (fsError != ERROR_OK) {
            warn("readFromBuffer: Cannot export volume (%s)\n",
                 ErrorCodeEnum::key(fsError));
        }
    }
    
    // REMOVE ASAP
    const char *path = "/tmp/test";
    msg("Doing a test export to %s\n", path);
    
    volume->exportDirectory(path);
    
    if (!adf) throw VAError(ERROR_UNKNOWN);
    return result;
}

