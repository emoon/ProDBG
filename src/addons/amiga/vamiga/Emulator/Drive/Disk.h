// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DiskTypes.h"
#include "HardwareComponent.h"

/* MFM encoded disk data of a standard 3.5" DD disk:
 *
 *    Cylinder  Track     Head      Sectors
 *    ---------------------------------------
 *    0         0         0          0 - 10
 *    0         1         1         11 - 21
 *    1         2         0         22 - 32
 *    1         3         1         33 - 43
 *                   ...
 *    79        158       0       1738 - 1748
 *    79        159       1       1749 - 1759
 *
 *    80        160       0       1760 - 1770   <--- beyond spec
 *    80        161       1       1771 - 1781
 *                   ...
 *    83        166       0       1826 - 1836
 *    83        167       1       1837 - 1847
 *
 * A single sector consists of
 *    - A sector header build up from 64 MFM bytes.
 *    - 512 bytes of data (1024 MFM bytes).
 *
 * Hence,
 *    - a sector consists of 64 + 2*512 = 1088 MFM bytes.
 *
 * A single track of a 3.5"DD disk consists
 *    - 11 * 1088 = 11.968 MFM bytes.
 *    - A track gap of about 700 MFM bytes (varies with drive speed).
 *
 * Hence,
 *    - a track usually occupies 11.968 + 700 = 12.668 MFM bytes.
 *    - a cylinder usually occupies 25.328 MFM bytes.
 *    - a disk usually occupies 84 * 2 * 12.664 =  2.127.552 MFM bytes
 */

class Disk : public AmigaObject {
    
    friend class Drive;
    friend class ADFFile;
    friend class IMGFile;
    
public:
    
    // The form factor of this disk
    DiskDiameter diameter;
    
    // The density of this disk
    DiskDensity density;
        
private:
    
    // The MFM encoded disk data
    union {
        u8 raw[168*32768];
        u8 cylinder[84][2][32768];
        u8 track[168][32768];
    } data;
        
    // Length of each track in bytes
    union {
        i32 cylinder[84][2];
        i32 track[168];
    } length;

    
    // Indicates if this disk is write protected
    bool writeProtected = false;
    
    // Indicates if the disk has been written to
    bool modified = false;
    
    // Checksum of this disk if it was created from an ADF file, 0 otherwise
    u64 fnv = 0;
    
    
    //
    // Initializing
    //
    
public:
    
    Disk(DiskDiameter type, DiskDensity density);
    ~Disk();

    const char *getDescription() const override { return "Disk"; }

    static Disk *makeWithFile(class DiskFile *file);
    static Disk *makeWithReader(util::SerReader &reader, DiskDiameter type, DiskDensity density);
        
    void dump();
    
    
    //
    // Serializing
    //

private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << diameter
        << density
        << data.raw
        << writeProtected
        << modified
        << fnv;
    }


    //
    // Accessing disk parameters
    //

public:

    DiskDiameter getDiameter() const { return diameter; }
    DiskDensity getDensity() const { return density; }

    isize numCyls() const { return diameter == INCH_525 ? 42 : 84; }
    isize numSides() const { return 2; }
    isize numTracks() const { return diameter == INCH_525 ? 84 : 168; }

    bool isWriteProtected() const { return writeProtected; }
    void setWriteProtection(bool value) { writeProtected = value; }
    
    bool isModified() const { return modified; }
    void setModified(bool value) { modified = value; }
    
    u64 getFnv() const { return fnv; }
    

    //
    // Reading and writing
    //
    
    // Reads a byte from disk
    u8 readByte(Track track, u16 offset) const;
    u8 readByte(Cylinder cylinder, Side side, u16 offset) const;

    // Writes a byte to disk
    void writeByte(u8 value, Track track, u16 offset);
    void writeByte(u8 value, Cylinder cylinder, Side side, u16 offset);
        
    
    //
    // Erasing disks
    //
    
public:

    // Initializes the disk with random data
    void clearDisk();

    // Initializes a single track with random data or a specific value
    void clearTrack(Track t);
    void clearTrack(Track t, u8 value);
    void clearTrack(Track t, u8 value1, u8 value2);

    
    //
    // Encoding
    //
    
public:
    
    // Encodes a disk
    bool encodeDisk(class DiskFile *df);
    
    
    //
    // Working with MFM encoded data streams
    //
    
public:
    
    static void encodeMFM(u8 *dst, u8 *src, isize count);
    static void decodeMFM(u8 *dst, u8 *src, isize count);

    static void encodeOddEven(u8 *dst, u8 *src, isize count);
    static void decodeOddEven(u8 *dst, u8 *src, isize count);

    static void addClockBits(u8 *dst, isize count);
    static u8 addClockBits(u8 value, u8 previous);

    // Repeats the MFM data inside the track buffer to ease decoding
    void repeatTracks(); 
};
