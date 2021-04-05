// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _EXTENDED_ROM_H
#define _EXTENDED_ROM_H

#include "AmigaFile.h"

class ExtendedRomFile : public AmigaFile {

private:

    // Accepted header signatures
    static const u8 magicBytes1[];
    static const u8 magicBytes2[];

public:

    //
    // Class methods
    //

    // Returns true iff buffer contains an Extended Rom image
    static bool isExtendedRomBuffer(const u8 *buffer, size_t length);

    // Returns true iff path points to a Extended Rom file
    static bool isExtendedRomFile(const char *path);


    //
    // Initializing
    //

    ExtendedRomFile();

    static ExtendedRomFile *makeWithBuffer(const u8 *buffer, size_t length);
    static ExtendedRomFile *makeWithFile(const char *path);


    //
    // Methods from AmigaFile
    //

    AmigaFileType fileType() override { return FILETYPE_EXTENDED_ROM; }
    const char *typeAsString() override { return "Extended Rom"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isExtendedRomBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isExtendedRomFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;

};

#endif
