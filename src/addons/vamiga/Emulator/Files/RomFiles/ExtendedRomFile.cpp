// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "ExtendedRomFile.h"

// AROS Extended ROM
const u8 ExtendedRomFile::magicBytes1[] = { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x02 };
const u8 ExtendedRomFile::magicBytes2[] = { 0x4E, 0x71, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x02 };

ExtendedRomFile::ExtendedRomFile()
{
}

bool
ExtendedRomFile::isCompatibleName(const string &name)
{
    return true;
}

bool
ExtendedRomFile::isCompatibleStream(std::istream &stream)
{
    if (streamLength(stream) != KB(512)) return false;
    
    return
    matchingStreamHeader(stream, magicBytes1, sizeof(magicBytes1)) ||
    matchingStreamHeader(stream, magicBytes2, sizeof(magicBytes2));
}

bool
ExtendedRomFile::isExtendedRomFile(const char *path)
{
    std::ifstream stream(path);
    return stream.is_open() ? isCompatibleStream(stream) : false;
}
