// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ROM_H
#define _ROM_H

#include "AmigaFile.h"

class RomFile : public AmigaFile {

    // Accepted header signatures
    static const u8 bootRomHeaders[1][8];
    static const u8 kickRomHeaders[6][7];

public:
    
    //
    // Class methods
    //
    
    // Returns true if buffer contains a Boot Rom or an Kickstart Rom image
    static bool isRomBuffer(const u8 *buffer, size_t length);
    
    // Returns true if path points to a Boot Rom file or a Kickstart Rom file
    static bool isRomFile(const char *path);
    
    // Translates a CRC-32 checksum into a ROM identifier
    static RomIdentifier identifier(u32 fingerprint);

    // Classifies a ROM identifier by type
    static bool isBootRom(RomIdentifier rev);
    static bool isArosRom(RomIdentifier rev);
    static bool isDiagRom(RomIdentifier rev);
    static bool isCommodoreRom(RomIdentifier rev);
    static bool isHyperionRom(RomIdentifier rev);

    // Translates a ROM indentifier into a textual description
    static const char *title(RomIdentifier rev);
    static const char *version(RomIdentifier rev);
    static const char *released(RomIdentifier rev);

    
    //
    // Initializing
    //
    
    RomFile();
    
    // Factory methods
    static RomFile *makeWithBuffer(const u8 *buffer, size_t length);
    static RomFile *makeWithFile(const char *path);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_ROM; }
    const char *typeAsString() override { return "Kickstart Rom"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isRomBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isRomFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;
};

#endif
