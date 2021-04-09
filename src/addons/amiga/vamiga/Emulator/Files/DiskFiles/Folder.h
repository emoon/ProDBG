// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "ADFFile.h"

class Folder : public DiskFile {
        
public:

    ADFFile *adf = nullptr;
    
    static bool isFolder(const char *path);
    
    
    //
    // Constructing
    //
    
    static Folder *makeWithFolder(const string &path) throws;
    static Folder *makeWithFolder(const string &path, ErrorCode *err);

    
    //
    // Methods from AmigaObject
    //
    
    const char *getDescription() const override { return "Folder"; }
        
    
    //
    // Methods from AmigaFile
    //
    
    FileType type() const override { return FILETYPE_DIR; }
    u64 fnv() const override { return adf->fnv(); }
    
    
    //
    // Methods from DiskFile
    //
    
public:
    
    FSVolumeType getDos() const override { return adf->getDos(); }
    void setDos(FSVolumeType dos) override { adf->setDos(dos); }
    DiskDiameter getDiskDiameter() const override { return adf->getDiskDiameter(); }
    DiskDensity getDiskDensity() const override { return adf->getDiskDensity(); }
    isize numSides() const override { return adf->numSides(); }
    isize numCyls() const override { return adf->numCyls(); }
    isize numSectors() const override { return adf->numSectors(); }
    BootBlockType bootBlockType() const override { return adf->bootBlockType(); }
    const char *bootBlockName() const override { return adf->bootBlockName(); }
    void killVirus() override { adf->killVirus(); }
    void readSector(u8 *target, isize s) const override { return adf->readSector(target, s); }
    void readSector(u8 *target, isize t, isize s) const override { return adf->readSector(target, t, s); }
    bool encodeDisk(class Disk *disk) override { return adf->encodeDisk(disk); }
};
