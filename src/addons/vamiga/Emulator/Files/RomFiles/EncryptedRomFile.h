// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ENCRYPTED_ROM_H
#define _ENCRYPTED_ROM_H

#include "RomFile.h"

class EncryptedRomFile : public AmigaFile {

    // Accepted header signatures
    static const u8 headers[1][11];

public:
    
    //
    // Class methods
    //
    
    // Returns true if buffer contains an encrypted Rom
    static bool isEncryptedRomBuffer(const u8 *buffer, size_t length);
    
    // Returns true if path points to an encrypted Rom file
    static bool isEncryptedRomFile(const char *path);
    
    
    //
    // Initializing
    //
    
    EncryptedRomFile();
    
    static EncryptedRomFile *makeWithBuffer(const u8 *buffer, size_t length);
    static EncryptedRomFile *makeWithFile(const char *path);
    
    
    //
    // Methods from AmigaFile
    //
    
    AmigaFileType fileType() override { return FILETYPE_ENCRYPTED_ROM; }
    const char *typeAsString() override { return "Encrypted Kickstart Rom"; }
    bool bufferHasSameType(const u8 *buffer, size_t length) override {
        return isEncryptedRomBuffer(buffer, length); }
    bool fileHasSameType(const char *path) override { return isEncryptedRomFile(path); }
    bool readFromBuffer(const u8 *buffer, size_t length) override;
    
    //
    // Decrypting
    //
    
    // Decrypts this Rom. The method seeks a rom.key file in the directory
    // the encrypted Rom was loaded from and applies it to the encrypted data.
    // If the encryption was successful, an (unencrypted) Rom is returned.
    // Otherwise, NULL is returned.
    RomFile *decrypt(DecryptionError *error);
    RomFile *decrypt() { return decrypt(NULL); }
};

#endif
