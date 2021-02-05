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
#include "FSDevice.h"

class Disk;

class HDFFile : public AmigaFile {
    
public:
    
    //
    // Class methods
    //

    static bool isCompatibleName(const string &name);
    static bool isCompatibleStream(std::istream &stream);
    
    
    //
    // Initializing
    //

public:

    HDFFile();
    
    const char *getDescription() const override { return "HDF"; }

    
    //
    // Methods from AmigaFile
    //
    
public:
    
    FileType type() const override { return FILETYPE_HDF; }


    //
    // Querying volume information
    //

public:
    
    // Returns true if this image contains a rigid disk block
    bool hasRDB() const;
    
    // Returns the layout parameters of the hard drive
    isize numCyls() const;
    isize numSides() const;
    isize numSectors() const;
    isize numReserved() const;
    isize numBlocks() const;
    long bsize() const;
    struct FSDeviceDescriptor layout();

    
    //
    // Querying partition information
    //

private:
    
    // Extract the DOS revision number from a certain block
    FSVolumeType dos(isize blockNr);    
};
