// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DMS_FILE_H
#define _DMS_FILE_H

#include "ADFFile.h"

class DMSFile : public DiskFile {
    
public:
    
    ADFFile *adf = nullptr;
    
    
    //
    // Class methods
    //
    
    // Returns true iff the provided buffer contains a DMS file
    static bool isDMSBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if the provided path points to a DMS file
    static bool isDMSFile(const char *path);
    
    
    //
    // Initializing
    //
    
    DMSFile();
    
    static DMSFile *makeWithBuffer(const u8 *buffer, size_t length);
    static DMSFile *makeWithFile(const char *path);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_DMS; }
    const char *typeAsString() override { return "DMS"; }
    u64 fnv() override { return adf->fnv(); }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isDMSBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isDMSFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;
    
    
    //
    // Methods from DiskFile
    //
    
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
