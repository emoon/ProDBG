// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _AMIGA_DISK_H
#define _AMIGA_DISK_H

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
    DiskType type;
    
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
        u32 cylinder[84][2];
        u32 track[168];
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
    
    Disk(DiskType type, DiskDensity density);
    ~Disk();

    static Disk *makeWithFile(class DiskFile *file);
    static Disk *makeWithReader(SerReader &reader, DiskType type, DiskDensity density);
        
    void dump();
    
    
    //
    // Serializing
    //

private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & type
        & density
        & data.raw
        & writeProtected
        & modified
        & fnv;
    }


    //
    // Accessing disk parameters
    //

public:

    DiskType getType() { return type; }
    DiskDensity getDensity() { return density; }

    long numCylinders() { return type == DISK_525 ? 42 : 84; }
    long numSides() { return 2; }
    long numTracks() { return type == DISK_525 ? 84 : 168; }
    // long trackLength(Track t) { return length.track[t]; }
    // long trackLength(Cylinder c, Side s) { return length.cylinder[c][s]; }

    bool isWriteProtected() { return writeProtected; }
    void setWriteProtection(bool value) { writeProtected = value; }
    
    bool isModified() { return modified; }
    void setModified(bool value) { modified = value; }
    
    u64 getFnv() { return fnv; }
    

    //
    // Reading and writing
    //
    
    // Reads a byte from disk
    u8 readByte(Track track, u16 offset);
    u8 readByte(Cylinder cylinder, Side side, u16 offset);

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
    
private:
    
    // Encodes a disk, track, or sector in Amiga format
    /*
    bool encodeAmigaDisk(class DiskFile *df);
    bool encodeAmigaTrack(class DiskFile *df, Track t);
    bool encodeAmigaSector(class DiskFile *df, Track t, Sector s);
    */
    
    // Encodes a disk, track, or sector in DOS format
    /*
    bool encodeDosDisk(class DiskFile *df);
    bool encodeDosTrack(class DiskFile *df, Track t);
    bool encodeDosSector(class DiskFile *df, Track t, Sector s);
    */
    
    //
    // Decoding
    //

public:
    
    // Decodes a disk, track, or sector in Amiga format
    /*
    bool decodeAmigaDisk(u8 *dst, long numTracks, long numSectors);
    bool decodeAmigaTrack(u8 *dst, Track t, long numSectors);
    bool decodeAmigaSector(u8 *dst, u8 *src);
    */
    
    // Decodes a disk, track, or sector in Amiga format
    /*
    bool decodeDOSDisk(u8 *dst, long numTracks, long numSectors);
    bool decodeDOSTrack(u8 *dst, Track t, long numSectors);
    void decodeDOSSector(u8 *dst, u8 *src);
    */
    
    //
    // Working with MFM encoded data streams
    //
    
public:
    
    static void encodeMFM(u8 *dst, u8 *src, size_t count);
    static void decodeMFM(u8 *dst, u8 *src, size_t count);

    static void encodeOddEven(u8 *dst, u8 *src, size_t count);
    static void decodeOddEven(u8 *dst, u8 *src, size_t count);

    static void addClockBits(u8 *dst, size_t count);
    static u8 addClockBits(u8 value, u8 previous);

    // Repeats the MFM data inside the track buffer to ease decoding
    void repeatTracks(); 
};

#endif
