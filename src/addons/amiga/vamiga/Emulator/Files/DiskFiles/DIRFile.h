// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DIR_FILE_H
#define _DIR_FILE_H

#include "ADFFile.h"

class DIRFile : public DiskFile {
    
public:
    
    ADFFile *adf = nullptr;
    
    
    //
    // Class methods
    //
        
    // Returns true iff if the provided path points to a suitable directory
    static bool isDIRFile(const char *path);
    
    
    //
    // Initializing
    //
    
    DIRFile();
    
    static DIRFile *makeWithFile(const char *path);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_DIR; }
    const char *typeAsString() override { return "DIR"; }
    u64 fnv() override { return adf->fnv(); }
    bool bufferHasSameType(const u8 *buffer, size_t length) override;
    bool fileHasSameType(const char *path) override { return isDIRFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;
    bool readFromFile(const char *filename) override;
    
    
    //
    // Methods from DiskFile
    //
    
public:
    
    DiskType getDiskType() override { return adf->getDiskType(); }
    DiskDensity getDiskDensity() override { return adf->getDiskDensity(); }
    long numSides() override { return adf->numSides(); }
    long numCyclinders() override { return adf->numCyclinders(); }
    long numSectors() override { return adf->numSectors(); }
    void readSector(u8 *target, long s) override { return adf->readSector(target, s); }
    void readSector(u8 *target, long t, long s) override { return adf->readSector(target, t, s); }
    bool encodeDisk(class Disk *disk) override { return adf->encodeDisk(disk); }
};

#endif
