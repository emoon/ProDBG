// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _SNAPSHOT_H
#define _SNAPSHOT_H

#include "AmigaFile.h"

class Amiga;

struct Thumbnail {
    
    // Image size
    u16 width, height;
    
    // Raw texture data
    u32 screen[(HPIXELS / 2) * (VPIXELS / 1)];
    
    // Date and time of screenshot creation
    time_t timestamp;
    
    // Factory methods
    static Thumbnail *makeWithAmiga(Amiga *amiga, int dx = 2, int dy = 1);
    
    // Takes a screenshot from a given Amiga
    void take(Amiga *amiga, int dx = 2, int dy = 1);
};

struct SnapshotHeader {
    
    // Magic bytes ('V','A','S','N','A','P')
    char magic[6];
    
    // Version number (V major.minor.subminor)
    u8 major;
    u8 minor;
    u8 subminor;
    
    // Screenshot
    Thumbnail screenshot;
};

class Snapshot : public AmigaFile {
 
    //
    // Class methods
    //
    
public:
    
    // Returns true iff buffer contains a snapshot.
    static bool isSnapshot(const u8 *buffer, size_t length);
    
    // Returns true iff buffer contains a snapshot of a specific version.
    static bool isSnapshot(const u8 *buffer, size_t length,
                           u8 major, u8 minor, u8 subminor);
    
    // Returns true iff buffer contains a snapshot with a supported version number.
    static bool isSupportedSnapshot(const u8 *buffer, size_t length);
    
    // Returns true iff buffer contains a snapshot with an outdated version number.
    static bool isUnsupportedSnapshot(const u8 *buffer, size_t length);
    
    // Returns true if path points to a snapshot file.
    static bool isSnapshotFile(const char *path);
    
    // Returns true if file points to a snapshot file of a specific version.
    static bool isSnapshotFile(const char *path,
                               u8 major, u8 minor, u8 subminor);
    
    // Returns true if file is a snapshot with a supported version number.
    static bool isSupportedSnapshotFile(const char *path);
    
    // Returns true if file is a snapshot with an outdated version number.
    static bool isUnsupportedSnapshotFile(const char *path);
    
    
    //
    // Initializing
    //
    
    Snapshot();
    Snapshot(size_t capacity);
    
    bool setCapacity(size_t size);
    
    static Snapshot *makeWithFile(const char *filename);
    static Snapshot *makeWithBuffer(const u8 *buffer, size_t size);
    static Snapshot *makeWithAmiga(Amiga *amiga);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_SNAPSHOT; }
    const char *typeAsString() override { return "VAMIGA"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override;
    bool fileHasSameType(const char *filename) override;
    
    
    //
    // Accessing snapshot properties
    //
    
public:
    
    // Returns pointer to header data
    SnapshotHeader *getHeader() { return (SnapshotHeader *)data; }
    
    // Returns pointer to core data
    u8 *getData() { return data + sizeof(SnapshotHeader); }
    
    // Returns the timestamp
    // GET DIRECTLY FROM SCREENSHOT
    time_t getTimestamp() { return getHeader()->screenshot.timestamp; }
    
    // Returns a pointer to the screenshot data
    // DEPRECATED
    unsigned char *getImageData() { return (unsigned char *)(getHeader()->screenshot.screen); }
    
    // Returns the screenshot image width
    // DEPRECATED
    unsigned getImageWidth() { return getHeader()->screenshot.width; }
    
    // Returns the screenshot image height
    // DEPRECATED
    unsigned getImageHeight() { return getHeader()->screenshot.height; }
};

#endif
