// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DiskFile *
DiskFile::makeWithFile(const char *path)
{
    ADFFile *adf = ADFFile::makeWithFile(path);
    if (adf) return adf;
    
    DMSFile *dms = DMSFile::makeWithFile(path);
    if (dms) return dms;

    IMGFile *img = IMGFile::makeWithFile(path);
    if (img) return img;

    return NULL;
}

void
DiskFile::readSector(u8 *dst, long t, long s)
{
    readSector(dst, t * numSectors() + s);
}

void
DiskFile::readSector(u8 *dst, long s)
{
    size_t sectorSize = 512;
    size_t offset = s * sectorSize;

    assert(dst != nullptr);
    assert(offset + sectorSize <= size);

    for (unsigned i = 0; i < 512; i++) {
        dst[i] = data[offset + i];
    }
}

bool
DiskFile::encodeDisk(class Disk *disk)
{
    assert(false);
    return false;
}

bool
DiskFile::decodeDisk(class Disk *disk)
{
    assert(false);
    return false;
}
