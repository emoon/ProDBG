// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskFile.h"

#define IMGSIZE_35_DD     737280  //  720 KB PC formatted disk

class IMGFile : public DiskFile {
    
public:
        
    static bool isCompatiblePath(const string &path);
    static bool isCompatibleStream(std::istream &stream);

    static IMGFile *makeWithDiskType(DiskDiameter t, DiskDensity d);
    static IMGFile *makeWithDisk(class Disk *disk) throws;
    static IMGFile *makeWithDisk(class Disk *disk, ErrorCode *ec);

    
    //
    // Methods from AmigaObject
    //
    
    const char *getDescription() const override { return "IMG"; }
    
    
    //
    // Methods from AmigaFile
    //
        
    FileType type() const override { return FILETYPE_IMG; }
    
    
    //
    // Methods from DiskFile
    //
      
    FSVolumeType getDos() const override { return FS_NODOS; }
    void setDos(FSVolumeType dos) override { };
    DiskDiameter getDiskDiameter() const override { return INCH_35; }
    DiskDensity getDiskDensity() const override { return DISK_DD; }
    isize numSides() const override;
    isize numCyls() const override;
    isize numSectors() const override;
    bool encodeDisk(class Disk *disk) override;
    void decodeDisk(class Disk *disk) throws override;

private:
    
    bool encodeTrack(class Disk *disk, Track t);
    bool encodeSector(class Disk *disk, Track t, Sector s);

    bool decodeTrack(class Disk *disk, Track t);
    bool decodeSector(u8 *dst, u8 *src);
};
