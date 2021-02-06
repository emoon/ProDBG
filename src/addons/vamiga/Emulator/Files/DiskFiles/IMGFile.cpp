// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

IMGFile::IMGFile()
{
    setDescription("IMGFile");
}

bool
IMGFile::isIMGBuffer(const u8 *buffer, size_t length)
{
    // There are no magic bytes. We can only check the buffer size
    return
    length == IMGSIZE_35_DD;
}

bool
IMGFile::isIMGFile(const char *path)
{
    // There are no magic bytes. We can only check the file size
    return
    checkFileSize(path, IMGSIZE_35_DD);
}

IMGFile *
IMGFile::makeWithDiskType(DiskType t, DiskDensity d)
{
    assert(t == DISK_35);
    assert(d == DISK_DD);
    
    IMGFile *img = new IMGFile();
    
    if (!img->alloc(9 * 160 * 512)) {
        delete img;
        return NULL;
    }
    
    memset(img->data, 0, img->size);
    return img;
}

IMGFile *
IMGFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromBuffer(buffer, length)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithFile(const char *path)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromFile(path)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithFile(FILE *file)
{
    IMGFile *img = new IMGFile();
    
    if (!img->readFromFile(file)) {
        delete img;
        return NULL;
    }
    
    return img;
}

IMGFile *
IMGFile::makeWithDisk(Disk *disk)
{
    assert(disk != NULL);
        
    // We only support 3.5"DD disks at the moment
    if (disk->getType() != DISK_35 || disk->getDensity() != DISK_DD) { return NULL; }
    
    IMGFile *img = makeWithDiskType(DISK_35, DISK_DD);
    
    if (img) {
        if (!img->decodeDisk(disk)) {
            printf("Failed to decode DOS disk\n");
            delete img;
            return NULL;
        }
    }
    
    return img;
}

bool
IMGFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isIMGBuffer(buffer, length);
}

long
IMGFile::numSides()
{
    return 2;
}

long
IMGFile::numCyclinders()
{
    return 80;
}

long
IMGFile::numSectors()
{
    return 9;
}

bool
IMGFile::encodeDisk(Disk *disk)
{
    long tracks = numTracks();
    
    debug(MFM_DEBUG, "Encoding DOS disk with %d tracks\n", tracks);

    if (disk->getType() != getDiskType()) {
        warn("Incompatible disk types: %s %s\n",
             sDiskType(disk->getType()), sDiskType(getDiskType()));
        return false;
    }
    if (disk->getDensity() != getDiskDensity()) {
        warn("Incompatible disk densities: %s %s\n",
             sDiskDensity(disk->getDensity()), sDiskDensity(getDiskDensity()));
        return false;
    }
    
    // Encode all tracks
    bool result = true;
    for (Track t = 0; t < tracks; t++) {
        result &= encodeTrack(disk, t);
    }
    
    // In debug mode, also run the decoder
    if (MFM_DEBUG) {
        debug("DOS disk fully encoded (success = %d)\n", result);
        IMGFile *tmp = IMGFile::makeWithDisk(disk);
        if (tmp) {
            msg("Decoded image written to /tmp/debug.img\n");
            tmp->writeToFile("/tmp/tmp.img");
        }
    }

    return result;
}

bool
IMGFile::encodeTrack(Disk *disk, Track t)
{
    long sectors = numSectors();

    debug(MFM_DEBUG, "Encoding DOS track %d with %d sectors\n", t, sectors);

    u8 *p = disk->data.track[t];

    // Clear track
    disk->clearTrack(t, 0x92, 0x54);

    // Encode track header
    p += 82;                                        // GAP
    for (int i = 0; i < 24; i++) { p[i] = 0xAA; }   // SYNC
    p += 24;
    p[0] = 0x52; p[1] = 0x24;                       // IAM
    p[2] = 0x52; p[3] = 0x24;
    p[4] = 0x52; p[5] = 0x24;
    p[6] = 0x55; p[7] = 0x52;
    p += 8;
    p += 80;                                        // GAP
        
    // Encode all sectors
    bool result = true;
    for (Sector s = 0; s < sectors; s++) result &= encodeSector(disk, t, s);
    
    // Compute a checksum for debugging
    if (MFM_DEBUG) {
        u64 check = fnv_1a_32(disk->data.track[t], disk->length.track[t]);
        debug("Track %d checksum = %x\n", t, check);
    }

    return result;
}

bool
IMGFile::encodeSector(Disk *disk, Track t, Sector s)
{
    u8 buf[60 + 512 + 2 + 109]; // Header + Data + CRC + Gap
        
    debug(MFM_DEBUG, "  Encoding DOS sector %d\n", s);
    
    // Write SYNC
    for (int i = 0; i < 12; i++) { buf[i] = 0x00; }
    
    // Write IDAM
    buf[12] = 0xA1;
    buf[13] = 0xA1;
    buf[14] = 0xA1;
    buf[15] = 0xFE;
    
    // Write CHRN
    buf[16] = (u8)(t / 2);
    buf[17] = (u8)(t % 2);
    buf[18] = (u8)(s + 1);
    buf[19] = 2;
    
    // Compute and write CRC
    u16 crc = crc16(&buf[12], 8);
    buf[20] = HI_BYTE(crc);
    buf[21] = LO_BYTE(crc);

    // Write GAP
    for (int i = 22; i < 44; i++) { buf[i] = 0x4E; }

    // Write SYNC
    for (int i = 44; i < 56; i++) { buf[i] = 0x00; }

    // Write DATA AM
    buf[56] = 0xA1;
    buf[57] = 0xA1;
    buf[58] = 0xA1;
    buf[59] = 0xFB;

    // Write DATA
    readSector(&buf[60], t, s);
    
    // Compute and write CRC
    crc = crc16(&buf[56], 516);
    buf[572] = HI_BYTE(crc);
    buf[573] = LO_BYTE(crc);

    // Write GAP
    for (size_t i = 574; i < sizeof(buf); i++) { buf[i] = 0x4E; }

    // Determine the start of this sector
    u8 *p = disk->data.track[t] + 194 + s * 1300;

    // Create the MFM data stream
    Disk::encodeMFM(p, buf, sizeof(buf));
    Disk::addClockBits(p, 2 * sizeof(buf));
    
    // Remove certain clock bits in IDAM block
    p[2*12+1] &= 0xDF;
    p[2*13+1] &= 0xDF;
    p[2*14+1] &= 0xDF;
    
    // Remove certain clock bits in DATA AM block
    p[2*56+1] &= 0xDF;
    p[2*57+1] &= 0xDF;
    p[2*58+1] &= 0xDF;

    return true;
}

bool
IMGFile::decodeDisk(Disk *disk)
{
    long tracks = numTracks();
    
    trace(MFM_DEBUG, "Decoding DOS disk (%d tracks)\n", tracks);
    
    if (disk->getType() != getDiskType()) {
        warn("Incompatible disk types: %s %s\n",
             sDiskType(disk->getType()), sDiskType(getDiskType()));
        return false;
    }
    if (disk->getDensity() != getDiskDensity()) {
        warn("Incompatible disk densities: %s %s\n",
             sDiskDensity(disk->getDensity()), sDiskDensity(getDiskDensity()));
        return false;
    }
    
    // Make the MFM stream scannable beyond the track end
    disk->repeatTracks();

    for (Track t = 0; t < tracks; t++) {
        if (!decodeTrack(disk, t)) return false;
    }
    
    return true;
}

bool
IMGFile::decodeTrack(Disk *disk, Track t)
{
    assert(t < disk->numTracks());
        
    long numSectors = 9;
    u8 *src = disk->data.track[t];
    u8 *dst = data + t * numSectors * 512;
    
    trace(MFM_DEBUG, "Decoding DOS track %d\n", t);
    
    // Create a local (double) copy of the track to simply the analysis
    /*
    u8 local[2 * disk->trackLength(t)];
    memcpy(local, disk->data.track[t], disk->trackLength(t));
    memcpy(local + disk->trackLength(t), disk->data.track[t], disk->trackLength(t));
    */
    
    // Determine the start of all sectors contained in this track
    int sectorStart[numSectors];
    for (int i = 0; i < numSectors; i++) {
        sectorStart[i] = 0;
    }
    int cnt = 0;
    for (size_t i = 0; i < sizeof(disk->data.track[t]) - 16;) {
        
        // Seek IDAM block
        if (src[i++] != 0x44) continue;
        if (src[i++] != 0x89) continue;
        if (src[i++] != 0x44) continue;
        if (src[i++] != 0x89) continue;
        if (src[i++] != 0x44) continue;
        if (src[i++] != 0x89) continue;
        if (src[i++] != 0x55) continue;
        if (src[i++] != 0x54) continue;

        // Decode CHRN block
        struct { u8 c; u8 h; u8 r; u8 n; } chrn;
        Disk::decodeMFM((u8 *)&chrn, &src[i], 4);
        trace(MFM_DEBUG, "c: %d h: %d r: %d n: %d\n", chrn.c, chrn.h, chrn.r, chrn.n);
        
        if (chrn.r >= 1 && chrn.r <= numSectors) {
            
            // Break the loop once we see the same sector twice
            if (sectorStart[chrn.r - 1] != 0) {
                break;
            }
            sectorStart[chrn.r - 1] = i + 88;
            cnt++;

        } else {
            warn("Invalid sector number %d. Aborting", chrn.r);
            return false;
        }
    }

    if (cnt != numSectors) {
        warn("Found %d sectors, expected %d. Aborting", cnt, numSectors);
        return false;
    }
        
    // Do some consistency checking
    for (int i = 0; i < numSectors; i++) assert(sectorStart[i] != 0);
    
    // Encode all sectors
    bool result = true;
    for (Sector s = 0; s < numSectors; s++) {
        result &= decodeSector(dst, src + sectorStart[s]);
        dst += 512;
    }
    
    return result;
}

bool
IMGFile::decodeSector(u8 *dst, u8 *src)
{
    Disk::decodeMFM(dst, src, 512);
    return true;
}
