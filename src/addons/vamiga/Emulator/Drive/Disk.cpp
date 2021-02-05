// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Disk::Disk(DiskDiameter type, DiskDensity density)
{    
    this->diameter = type;
    this->density = density;
    
    u32 trackLength = 0;
    if (type == INCH_35  && density == DISK_DD) trackLength = 12668;
    if (type == INCH_35  && density == DISK_HD) trackLength = 24636;
    if (type == INCH_525 && density == DISK_DD) trackLength = 12668;

    assert(trackLength != 0);
    for (isize i = 0; i < 168; i++) length.track[i] = trackLength;
    
    clearDisk();
}

Disk::~Disk()
{
}

Disk *
Disk::makeWithFile(DiskFile *file)
{
    Disk *disk = new Disk(file->getDiskDiameter(), file->getDiskDensity());
    
    if (!disk->encodeDisk(file)) {
        delete disk;
        return nullptr;
    }
    
    disk->fnv = file->fnv();
    
    return disk;
}

Disk *
Disk::makeWithReader(SerReader &reader, DiskDiameter type, DiskDensity density)
{
    Disk *disk = new Disk(type, density);
    disk->applyToPersistentItems(reader);
    
    return disk;
}

void
Disk::dump()
{
    msg("\nDisk:\n");
    msg("            type : %s\n", DiskDiameterEnum::key(diameter));
    msg("         density : %s\n", DiskDensityEnum::key(density));
    msg("       numCyls() : %ld\n", numCyls());
    msg("      numSides() : %ld\n", numSides());
    msg("     numTracks() : %ld\n", numTracks());
    msg("  track 0 length : %u\n", length.track[0]);
    msg("  writeProtected : %s\n", writeProtected ? "yes" : "no");
    msg("        modified : %s\n", modified ? "yes" : "no");
    msg("             fnv : %llu\n", fnv);
}

u8
Disk::readByte(Track t, u16 offset) const
{
    assert(t < numTracks());
    assert(offset < length.track[t]);

    return data.track[t][offset];
}

u8
Disk::readByte(Cylinder c, Side s, u16 offset) const
{
    assert(c < numCyls());
    assert(s < numSides());
    assert(offset < length.cylinder[c][s]);

    return data.cylinder[c][s][offset];
}

void
Disk::writeByte(u8 value, Track t, u16 offset)
{
    assert(t < numTracks());
    assert(offset < length.track[t]);

    data.track[t][offset] = value;
}

void
Disk::writeByte(u8 value, Cylinder c, Side s, u16 offset)
{
    assert(c < numCyls());
    assert(s < numSides());
    assert(offset < length.cylinder[c][s]);

    data.cylinder[c][s][offset] = value;
}

void
Disk::clearDisk()
{
    fnv = 0;

    // Initialize with random data
    srand(0);
    for (isize i = 0; i < isizeof(data.raw); i++) {
        data.raw[i] = rand() & 0xFF;
    }
    
    /* In order to make some copy protected game titles work, we smuggle in
     * some magic values. E.g., Crunch factory expects 0x44A2 on cylinder 80.
     */
    if (diameter == INCH_35 && density == DISK_DD) {
        
        for (isize t = 0; t < numTracks(); t++) {
            data.track[t][0] = 0x44;
            data.track[t][1] = 0xA2;
        }
    }
}

void
Disk::clearTrack(Track t)
{
    assert(t < numTracks());

    srand(0);
    for (isize i = 0; i < length.track[t]; i++) {
        data.track[t][i] = rand() & 0xFF;
    }
}

void
Disk::clearTrack(Track t, u8 value)
{
    assert(t < numTracks());

    for (isize i = 0; i < isizeof(data.track[t]); i++) {
        data.track[t][i] = value;
    }
}

void
Disk::clearTrack(Track t, u8 value1, u8 value2)
{
    assert(t < numTracks());

    for (isize i = 0; i < length.track[t]; i++) {
        data.track[t][i] = (i % 2) ? value2 : value1;
    }
}

bool
Disk::encodeDisk(DiskFile *df)
{
    assert(df != nullptr);
    assert(df->getDiskDiameter() == getDiameter());

    // Start with an unformatted disk
    clearDisk();

    // Call the MFM encoder
    return df->encodeDisk(this);
}

void
Disk::encodeMFM(u8 *dst, u8 *src, isize count)
{
    for(isize i = 0; i < count; i++) {
        
        u16 mfm =
        ((src[i] & 0b10000000) << 7) |
        ((src[i] & 0b01000000) << 6) |
        ((src[i] & 0b00100000) << 5) |
        ((src[i] & 0b00010000) << 4) |
        ((src[i] & 0b00001000) << 3) |
        ((src[i] & 0b00000100) << 2) |
        ((src[i] & 0b00000010) << 1) |
        ((src[i] & 0b00000001) << 0);
        
        dst[2*i+0] = HI_BYTE(mfm);
        dst[2*i+1] = LO_BYTE(mfm);
    }
}

void
Disk::decodeMFM(u8 *dst, u8 *src, isize count)
{
    for(isize i = 0; i < count; i++) {
        
        u16 mfm = HI_LO(src[2*i], src[2*i+1]);
        dst[i] =
        ((mfm & 0b0100000000000000) >> 7) |
        ((mfm & 0b0001000000000000) >> 6) |
        ((mfm & 0b0000010000000000) >> 5) |
        ((mfm & 0b0000000100000000) >> 4) |
        ((mfm & 0b0000000001000000) >> 3) |
        ((mfm & 0b0000000000010000) >> 2) |
        ((mfm & 0b0000000000000100) >> 1) |
        ((mfm & 0b0000000000000001) >> 0);
    }
}

void
Disk::encodeOddEven(u8 *dst, u8 *src, isize count)
{
    // Encode odd bits
    for(isize i = 0; i < count; i++)
        dst[i] = (src[i] >> 1) & 0x55;
    
    // Encode even bits
    for(isize i = 0; i < count; i++)
        dst[i + count] = src[i] & 0x55;
}

void
Disk::decodeOddEven(u8 *dst, u8 *src, isize count)
{
    // Decode odd bits
    for(isize i = 0; i < count; i++)
        dst[i] = (src[i] & 0x55) << 1;
    
    // Decode even bits
    for(isize i = 0; i < count; i++)
        dst[i] |= src[i + count] & 0x55;
}

void
Disk::addClockBits(u8 *dst, isize count)
{
    for (isize i = 0; i < count; i++) {
        dst[i] = addClockBits(dst[i], dst[i-1]);
    }
}

u8
Disk::addClockBits(u8 value, u8 previous)
{
    // Clear all previously set clock bits
    value &= 0x55;

    // Compute clock bits (clock bit values are inverted)
    u8 lShifted = (u8)(value << 1);
    u8 rShifted = (u8)(value >> 1 | previous << 7);
    u8 cBitsInv = (u8)(lShifted | rShifted);

    // Reverse the computed clock bits
    u64 cBits = cBitsInv ^ 0xAA;
    
    // Return original value with the clock bits added
    return value | cBits;
}

void
Disk::repeatTracks()
{
    for (Track t = 0; t < 168; t++) {
        
        long end = length.track[t];        
        for (isize i = end, j = 0; i < isizeof(data.track[t]); i++, j++) {
            data.track[t][i] = data.track[t][j];
        }
    }
}
