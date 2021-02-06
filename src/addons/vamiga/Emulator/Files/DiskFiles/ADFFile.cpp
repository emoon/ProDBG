// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

ADFFile::ADFFile()
{
    setDescription("ADFFile");
}

bool
ADFFile::isADFBuffer(const u8 *buffer, size_t length)
{
    // Some ADFs contain an additional byte at the end. Ignore it
    length &= ~1;
    
    // There are no magic bytes. Hence, we only check the file size
    return
    length == ADFSIZE_35_DD ||
    length == ADFSIZE_35_DD_81 ||
    length == ADFSIZE_35_DD_82 ||
    length == ADFSIZE_35_DD_83 ||
    length == ADFSIZE_35_DD_84 ||
    length == ADFSIZE_35_HD;
}

bool
ADFFile::isADFFile(const char *path)
{
    // There are no magic bytes. Hence, we only check the file size
    return
    checkFileSize(path, ADFSIZE_35_DD) ||
    checkFileSize(path, ADFSIZE_35_DD_81) ||
    checkFileSize(path, ADFSIZE_35_DD_82) ||
    checkFileSize(path, ADFSIZE_35_DD_83) ||
    checkFileSize(path, ADFSIZE_35_DD_84) ||
    checkFileSize(path, ADFSIZE_35_HD) ||
    
    checkFileSize(path, ADFSIZE_35_DD+1) ||
    checkFileSize(path, ADFSIZE_35_DD_81+1) ||
    checkFileSize(path, ADFSIZE_35_DD_82+1) ||
    checkFileSize(path, ADFSIZE_35_DD_83+1) ||
    checkFileSize(path, ADFSIZE_35_DD_84+1) ||
    checkFileSize(path, ADFSIZE_35_HD+1);
}

size_t
ADFFile::fileSize(DiskType t, DiskDensity d)
{
    assert(isDiskType(t));
    
    if (t == DISK_35 && d == DISK_DD) return ADFSIZE_35_DD;
    if (t == DISK_35 && d == DISK_HD) return ADFSIZE_35_HD;

    assert(false);
}

ADFFile *
ADFFile::makeWithDiskType(DiskType t, DiskDensity d)
{
    assert(isDiskType(t));
    
    ADFFile *adf = new ADFFile();
    
    if (!adf->alloc(fileSize(t, d))) {
        delete adf;
        return NULL;
    }
    
    memset(adf->data, 0, adf->size);
    return adf;
}

ADFFile *
ADFFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    ADFFile *adf = new ADFFile();
    
    if (!adf->readFromBuffer(buffer, length)) {
        delete adf;
        return NULL;
    }
    
    /*
    adf->dumpSector(880);
    adf->dumpSector(882);
    adf->dumpSector(883);
    */
    
    return adf;
}

ADFFile *
ADFFile::makeWithFile(const char *path)
{
    ADFFile *adf = new ADFFile();
    
    if (!adf->readFromFile(path)) {
        delete adf;
        return NULL;
    }
    
    return adf;
}

ADFFile *
ADFFile::makeWithFile(FILE *file)
{
    ADFFile *adf = new ADFFile();
    
    if (!adf->readFromFile(file)) {
        delete adf;
        return NULL;
    }
    
    return adf;
}

ADFFile *
ADFFile::makeWithDisk(Disk *disk)
{
    assert(disk != NULL);

    DiskType type = disk->getType();
    DiskDensity density = disk->getDensity();

    // Create empty ADF
    ADFFile *adf = makeWithDiskType(type, density);
    if (!adf) return nullptr;
    
    // Export disk
    assert(adf->numTracks() == 160);
    assert(adf->numSectors() == 11 || adf->numSectors() == 22);
    if (!adf->decodeDisk(disk)) {
        delete adf;
        return nullptr;
    }
    
    return adf;
}

ADFFile *
ADFFile::makeWithVolume(FSVolume &volume)
{
    ADFFile *adf = nullptr;
    assert(volume.getBlockSize() == 512);
    
    switch (volume.getCapacity()) {
            
        case 2 * 880:
            adf = makeWithDiskType(DISK_35, DISK_DD);
            break;
            
        case 4 * 880:
            adf = makeWithDiskType(DISK_35, DISK_HD);
            break;
            
        default:
            assert(false);
    }

    volume.exportVolume(adf->data, adf->size);
    return adf;
}

bool
ADFFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isADFBuffer(buffer, length);
}

DiskType
ADFFile::getDiskType()
{
    return DISK_35;
}

DiskDensity
ADFFile::getDiskDensity()
{
    return (size & ~1) == ADFSIZE_35_HD ? DISK_HD : DISK_DD;
}

long
ADFFile::numSides()
{
    return 2;
}

long
ADFFile::numCyclinders()
{
    switch(size & ~1) {
            
        case ADFSIZE_35_DD:    return 80;
        case ADFSIZE_35_DD_81: return 81;
        case ADFSIZE_35_DD_82: return 82;
        case ADFSIZE_35_DD_83: return 83;
        case ADFSIZE_35_DD_84: return 84;
        case ADFSIZE_35_HD:    return 80;
            
        default:
            assert(false);
            return 0;
    }
}

long
ADFFile::numSectors()
{
    switch (getDiskDensity()) {
            
        case DISK_DD: return 11;
        case DISK_HD: return 22;
            
        default:
            assert(false);
            return 0;
    }
}

// TODO: Replace by makeWith(EmptyDiskFormat ...)
//       Add DiskFile::numBlocks()
//
//
bool
ADFFile::formatDisk(FSVolumeType fs)
{
    assert(isFSType(fs));

    msg("Formatting disk with %d blocks (%s)\n", numBlocks(), sFSType(fs));

    // Only proceed if a file system is given
    if (fs == FS_NONE) return false;
    
    // Create an empty file system
    FSVolume vol = FSVolume(fs, "MyDisk", numBlocks());
    
    // Export the volume to the ADF
    return vol.exportVolume(data, size);

    return true;
}

bool
ADFFile::encodeDisk(Disk *disk)
{
    assert(disk != NULL);
    
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

    long tracks = numTracks();
    debug(MFM_DEBUG, "Encoding %d tracks\n", tracks);

    // Start with an unformatted disk
    disk->clearDisk();

    // Encode all tracks
    bool result = true;
    for (Track t = 0; t < tracks; t++) result &= encodeTrack(disk, t);

    // In debug mode, also run the decoder
    if (MFM_DEBUG) {
        debug("Amiga disk fully encoded (success = %d)\n", result);
        ADFFile *tmp = ADFFile::makeWithDisk(disk);
        if (tmp) {
            msg("Decoded image written to /tmp/debug.adf\n");
            tmp->writeToFile("/tmp/tmp.adf");
        }
    }

    return result;
}

bool
ADFFile::encodeTrack(Disk *disk, Track t)
{
    long sectors = numSectors();
    // assert(disk->geometry.sectors == sectors);
    
    trace(MFM_DEBUG, "Encoding Amiga track %d (%d sectors)\n", t, sectors);

    // Format track
    disk->clearTrack(t, 0xAA);

    // Encode all sectors
    bool result = true;
    for (Sector s = 0; s < sectors; s++) result &= encodeSector(disk, t, s);
    
    // Rectify the first clock bit (where buffer wraps over)
    if (disk->data.track[t][disk->length.track[t] - 1] & 1) {
        disk->data.track[t][0] &= 0x7F;
    }

    // Compute a debug checksum
    if (MFM_DEBUG) {
        u64 check = fnv_1a_32(disk->data.track[t], disk->length.track[t]);
        debug("Track %d checksum = %x\n", t, check);
    }

    return result;
}

bool
ADFFile::encodeSector(Disk *disk, Track t, Sector s)
{
    assert(t < disk->numTracks());
    
    debug(MFM_DEBUG, "Encoding sector %d\n", s);
    
    // Block header layout:
    //
    //                         Start  Size   Value
    //     Bytes before SYNC   00      4     0xAA 0xAA 0xAA 0xAA
    //     SYNC mark           04      4     0x44 0x89 0x44 0x89
    //     Track & sector info 08      8     Odd/Even encoded
    //     Unused area         16     32     0xAA
    //     Block checksum      48      8     Odd/Even encoded
    //     Data checksum       56      8     Odd/Even encoded
    
    // Determine the start of this sector
    u8 *p = disk->data.track[t] + 700 + (s * 1088);
    // u8 *p = disk->ptr(t, s);
    
    // Bytes before SYNC
    p[0] = (p[-1] & 1) ? 0x2A : 0xAA;
    p[1] = 0xAA;
    p[2] = 0xAA;
    p[3] = 0xAA;
    
    // SYNC mark
    u16 sync = 0x4489;
    p[4] = HI_BYTE(sync);
    p[5] = LO_BYTE(sync);
    p[6] = HI_BYTE(sync);
    p[7] = LO_BYTE(sync);
    
    // Track and sector information
    u8 info[4] = { 0xFF, (u8)t, (u8)s, (u8)(11 - s) };
    Disk::encodeOddEven(&p[8], info, sizeof(info));
    
    // Unused area
    for (unsigned i = 16; i < 48; i++)
    p[i] = 0xAA;
    
    // Data
    u8 bytes[512];
    readSector(bytes, t, s);
    Disk::encodeOddEven(&p[64], bytes, sizeof(bytes));
    
    // Block checksum
    u8 bcheck[4] = { 0, 0, 0, 0 };
    for(unsigned i = 8; i < 48; i += 4) {
        bcheck[0] ^= p[i];
        bcheck[1] ^= p[i+1];
        bcheck[2] ^= p[i+2];
        bcheck[3] ^= p[i+3];
    }
    Disk::encodeOddEven(&p[48], bcheck, sizeof(bcheck));
    
    // Data checksum
    u8 dcheck[4] = { 0, 0, 0, 0 };
    for(unsigned i = 64; i < 1088; i += 4) {
        dcheck[0] ^= p[i];
        dcheck[1] ^= p[i+1];
        dcheck[2] ^= p[i+2];
        dcheck[3] ^= p[i+3];
    }
    Disk::encodeOddEven(&p[56], dcheck, sizeof(bcheck));
    
    // Add clock bits
    for(unsigned i = 8; i < 1088; i++) {
        p[i] = Disk::addClockBits(p[i], p[i-1]);
    }
    
    return true;
}

void
ADFFile::dumpSector(int num)
{
    u8 *p = data + 512 * num;
    int cols = 32;

    printf("Sector %d\n", num);
    for (int y = 0; y < 512 / cols; y++) {
        for (int x = 0; x < cols; x++) {
            printf("%02X ", p[y*cols + x]);
            if ((x % 4) == 3) printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}

bool
ADFFile::decodeDisk(Disk *disk)
{
    long tracks = numTracks();
    
    debug(MFM_DEBUG, "Decoding Amiga disk with %d tracks\n", tracks);
    
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
ADFFile::decodeTrack(Disk *disk, Track t)
{ 
    long sectors = numSectors();

    trace(MFM_DEBUG, "Decoding track %d\n", t);
    
    u8 *src = disk->data.track[t];
    u8 *dst = data + t * sectors * 512;
    
    // Seek all sync marks
    int sectorStart[sectors], nr = 0; size_t index = 0;
    while (index < sizeof(disk->data.track[t]) && nr < sectors) {

        // Scan MFM stream for $4489 $4489
        if (src[index++] != 0x44) continue;
        if (src[index++] != 0x89) continue;
        if (src[index++] != 0x44) continue;
        if (src[index++] != 0x89) continue;

        // Make sure it's not a DOS track
        if (src[index + 1] == 0x89) continue;

        sectorStart[nr++] = index;
    }
    
    trace(MFM_DEBUG, "Found %d sectors (expected %d)\n", nr, sectors);

    if (nr != sectors) {
        warn("Found %d sectors, expected %d. Aborting.\n", nr, sectors);
        return false;
    }
    
    // Encode all sectors
    bool result = true;
    for (Sector s = 0; s < sectors; s++) {
        result &= decodeSector(dst, src + sectorStart[s]);
    }
    
    return result;
}

bool
ADFFile::decodeSector(u8 *dst, u8 *src)
{
    assert(dst != NULL);
    assert(src != NULL);
    
    // Decode sector info
    u8 info[4];
    Disk::decodeOddEven(info, src, 4);
    
    // Only proceed if the sector number is valid
    u8 sector = info[2];
    if (sector >= numSectors()) return false;
    
    // Skip sector header
    src += 56;
    
    // Decode sector data
    Disk::decodeOddEven(dst + sector * 512, src, 512);
    return true;
}
