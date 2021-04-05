// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ADF_H
#define _ADF_H

#include "DiskFile.h"
#include "FSVolume.h"

#define ADFSIZE_35_DD     901120  //  880 KB
#define ADFSIZE_35_DD_81  912384  //  891 KB (1 extra cylinder)
#define ADFSIZE_35_DD_82  923648  //  902 KB (2 extra cylinders)
#define ADFSIZE_35_DD_83  934912  //  913 KB (3 extra cylinders)
#define ADFSIZE_35_DD_84  946176  //  924 KB (4 extra cylinders)
#define ADFSIZE_35_HD    1802240  // 1760 KB

class Disk;

class ADFFile : public DiskFile {
    
public:
    
    //
    // Class methods
    //
    
    // Returns true iff the provided buffer contains an ADF file
    static bool isADFBuffer(const u8 *buffer, size_t length);
    
    // Returns true iff if the provided path points to an ADF file
    static bool isADFFile(const char *path);
    
    // Returns the size of an ADF file of a given disk type in bytes
    static size_t fileSize(DiskType t, DiskDensity d);

    
    //
    // Initializing
    //

public:

    ADFFile();
    
    static ADFFile *makeWithDiskType(DiskType t, DiskDensity d);
    static ADFFile *makeWithBuffer(const u8 *buffer, size_t length);
    static ADFFile *makeWithFile(const char *path);
    static ADFFile *makeWithFile(FILE *file);
    static ADFFile *makeWithDisk(Disk *disk);
    static ADFFile *makeWithVolume(FSVolume &volume);

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    AmigaFileType fileType() override { return FILETYPE_ADF; }
    const char *typeAsString() override { return "ADF"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isADFBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isADFFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;
    
    
    //
    // Methods from DiskFile
    //
    
public:
    
    DiskType getDiskType() override;
    DiskDensity getDiskDensity() override;
    long numSides() override;
    long numCyclinders() override;
    long numSectors() override;
    bool encodeDisk(class Disk *disk) override;
    bool decodeDisk(class Disk *disk) override;

private:
    
    bool encodeTrack(class Disk *disk, Track t);
    bool encodeSector(class Disk *disk, Track t, Sector s);

    bool decodeTrack(class Disk *disk, Track t);
    bool decodeSector(u8 *dst, u8 *src);

    
    //
    // Formatting
    //
 
public:
    
    bool formatDisk(FSVolumeType fs); 

    
    //
    // Debugging
    //
 
public:
    
    void dumpSector(int num);
};

#endif
