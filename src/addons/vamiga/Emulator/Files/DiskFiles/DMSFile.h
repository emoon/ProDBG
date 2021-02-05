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

class DMSFile : public DiskFile {
        
public:

    ADFFile *adf = nullptr;
        
    static bool isCompatibleName(const string &name);
    static bool isCompatibleStream(std::istream &stream);
    
    
    //
    //  Methods from AmigaObject
    //
        
    const char *getDescription() const override { return "DMS"; }
        
    
    //
    // Methods from AmigaFile
    //
    
    FileType type() const override { return FILETYPE_DMS; }
    u64 fnv() const override { return adf->fnv(); }
    isize readFromStream(std::istream &stream) override;

    
    //
    // Methods from DiskFile
    //
    
    FSVolumeType getDos() const override { return adf->getDos(); }
    void setDos(FSVolumeType dos) override { adf->setDos(dos); }
    DiskDiameter getDiskDiameter() const override { return adf->getDiskDiameter(); }
    DiskDensity getDiskDensity() const override { return adf->getDiskDensity(); }
    isize numSides() const override { return adf->numSides(); }
    isize numCyls() const override { return adf->numCyls(); }
    isize numSectors() const override { return adf->numSectors(); }
    BootBlockType bootBlockType() const override { return adf->bootBlockType(); }
    const char *bootBlockName() const override { return adf->bootBlockName(); }
    void readSector(u8 *target, long s) const override { return adf->readSector(target, s); }
    void readSector(u8 *target, long t, long s) const override { return adf->readSector(target, t, s); }
    bool encodeDisk(class Disk *disk) override { return adf->encodeDisk(disk); }
};
