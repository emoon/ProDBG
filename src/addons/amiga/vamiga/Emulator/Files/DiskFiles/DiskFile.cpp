// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "DiskFile.h"
#include "ADFFile.h"
#include "IMGFile.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "Folder.h"

DiskFile *
DiskFile::make(const string &path)
{
    std::ifstream stream(path);
    if (!stream.is_open()) throw VAError(ERROR_FILE_NOT_FOUND);
    
    switch (type(path)) {
            
        case FILETYPE_ADF:
            return AmigaFile::make <ADFFile> (path, stream);
            
        case FILETYPE_IMG:
            return AmigaFile::make <IMGFile> (path, stream);

        case FILETYPE_DMS:
            return AmigaFile::make <DMSFile> (path, stream);
            
        case FILETYPE_EXE:
            return AmigaFile::make <EXEFile> (path, stream);

        case FILETYPE_DIR:
            return Folder::make (path);

        default:
            break;
    }
    throw VAError(ERROR_FILE_TYPE_MISMATCH);
}

DiskFile *
DiskFile::make(const string &path, ErrorCode *err)
{
    *err = ERROR_OK;
    try { return make(path); }
    catch (VAError &exception) { *err = exception.data; }
    return nullptr;
}

u8
DiskFile::readByte(isize t, isize s, isize offset) const
{
    return readByte(t * numSectors() + s, offset);
}

u8
DiskFile::readByte(isize b, isize offset) const
{
    assert(offset < 512);
    return data[b * 512 + offset];
}

void
DiskFile::readSector(u8 *dst, isize t, isize s) const
{
    readSector(dst, t * numSectors() + s);
}

void
DiskFile::readSector(u8 *dst, isize s) const
{
    isize sectorSize = 512;
    isize offset = s * sectorSize;

    assert(dst != nullptr);
    assert(offset + sectorSize <= size);

    for (isize i = 0; i < sectorSize; i++) {
        dst[i] = data[offset + i];
    }
}

void
DiskFile::readSectorHex(char *dst, isize t, isize s, isize count) const
{
    readSectorHex(dst, t * numSectors() + s, count);
}

void
DiskFile::readSectorHex(char *dst, isize s, isize count) const
{
    isize sectorSize = 512;
    isize offset = s * sectorSize;

    assert(dst != nullptr);

    for (isize i = 0; i < count; i++) {
        sprintf(dst + 3*i, "%02X ", data[offset + i]);
    }
}

bool
DiskFile::encodeDisk(class Disk *disk)
{
    assert(false);
    return false;
}

void
DiskFile::decodeDisk(class Disk *disk)
{
    assert(false);
}
