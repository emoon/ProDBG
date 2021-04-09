// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Snapshot.h"
#include "Amiga.h"
#include "IO.h"

Thumbnail *
Thumbnail::makeWithAmiga(Amiga *amiga, isize dx, isize dy)
{
    Thumbnail *screenshot = new Thumbnail();
    screenshot->take(amiga, dx, dy);
    
    return screenshot;
}

void
Thumbnail::take(Amiga *amiga, isize dx, isize dy)
{
    u32 *source = (u32 *)amiga->denise.pixelEngine.getStableBuffer().data;
    u32 *target = screen;
    
    isize xStart = 4 * HBLANK_MAX + 1, xEnd = HPIXELS + 4 * HBLANK_MIN;
    isize yStart = VBLANK_CNT, yEnd = VPIXELS - 2;
    
    width  = (xEnd - xStart) / dx;
    height = (yEnd - yStart) / dy;
    
    source += xStart + yStart * HPIXELS;
    
    for (isize y = 0; y < height; y++) {
        for (isize x = 0; x < width; x++) {
            target[x] = source[x * dx];
        }
        source += dy * HPIXELS;
        target += width;
    }
    
    timestamp = time(nullptr);
}

bool
Snapshot::isCompatiblePath(const string &path)
{
    return true;
}

bool
Snapshot::isCompatibleStream(std::istream &stream)
{
    const u8 magicBytes[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    if (util::streamLength(stream) < 0x15) return false;
    return util::matchingStreamHeader(stream, magicBytes, sizeof(magicBytes));
}

Snapshot::Snapshot()
{
}

Snapshot::Snapshot(isize capacity)
{
    u8 signature[] = { 'V', 'A', 'S', 'N', 'A', 'P' };
    
    size = capacity + sizeof(SnapshotHeader);
    data = new u8[size];
    
    SnapshotHeader *header = (SnapshotHeader *)data;
    
    for (isize i = 0; i < isizeof(signature); i++)
        header->magic[i] = signature[i];
    header->major = V_MAJOR;
    header->minor = V_MINOR;
    header->subminor = V_SUBMINOR;
}

Snapshot *
Snapshot::makeWithAmiga(Amiga *amiga)
{
    Snapshot *snapshot = new Snapshot(amiga->size());

    snapshot->takeScreenshot(*amiga);
    amiga->save(snapshot->getData());

    return snapshot;
}

void
Snapshot::takeScreenshot(Amiga &amiga)
{
    ((SnapshotHeader *)data)->screenshot.take(&amiga);
}
