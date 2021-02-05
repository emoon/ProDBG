// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaFile.h"

// Base class of all file types encoding a disk
class DiskFile : public AmigaFile {

    //
    // Initializing
    //

public:

    // Gets or sets the file system for this disk
    virtual FSVolumeType getDos() const = 0;
    virtual void setDos(FSVolumeType dos) = 0;
    
    
    //
    // Querying disk properties
    //
    
public:
        
    // Returns the layout parameters for this disk
    virtual DiskDiameter getDiskDiameter() const = 0;
    virtual DiskDensity getDiskDensity() const = 0;
    virtual isize numSides() const = 0;
    virtual isize numCyls() const = 0;
    virtual isize numSectors() const = 0;
    long numTracks() const { return numSides() * numCyls(); }
    long numBlocks() const { return numTracks() * numSectors(); }

    // Analyzes the boot block
    virtual BootBlockType bootBlockType() const { return BB_STANDARD; }
    virtual const char *bootBlockName() const { return ""; }
    bool hasVirus() const { return bootBlockType() == BB_VIRUS; }

    
    //
    // Reading data
    //
    
public:

    // Reads a single data byte
    virtual u8 readByte(long b, long offset) const;
    virtual u8 readByte(long t, long s, long offset) const;

    // Fills a buffer with the data of a single sector
    virtual void readSector(u8 *dst, long b) const;
    virtual void readSector(u8 *dst, long t, long s) const;

    // Writes a string representation into the provided buffer
    virtual void readSectorHex(char *dst, long b, isize count) const;
    virtual void readSectorHex(char *dst, long t, long s, isize count) const;

    
    //
    // Repairing
    //

    virtual void killVirus() { };

    
    //
    // Encoding
    //
 
public:
    
    virtual bool encodeDisk(class Disk *disk);
    virtual void decodeDisk(class Disk *disk) throws;
};
