// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "EncryptedRomFile.h"

//
// Encrypted Kickstart Roms
//

const u8 EncryptedRomFile::headers[1][11] = {

    // Cloanto Rom Header Signature
    { 'A', 'M', 'I', 'R', 'O', 'M', 'T', 'Y', 'P', 'E', '1' }
};

EncryptedRomFile::EncryptedRomFile()
{
    setDescription("EncryptedRom");
}

bool
EncryptedRomFile::isEncryptedRomBuffer(const u8 *buffer, size_t length)
{
    if (length == KB(256)+11 || length == KB(512)+11) {

        int len = sizeof(headers[0]);
        int cnt = sizeof(headers) / len;

        for (int i = 0; i < cnt; i++)
            if (matchingBufferHeader(buffer, headers[i], len)) return true;

        return false;
    }

    return false;
}

bool
EncryptedRomFile::isEncryptedRomFile(const char *path)
{
     if (checkFileSize(path, KB(256)+11) || checkFileSize(path, KB(512)+11)) {

         int len = sizeof(headers[0]);
         int cnt = sizeof(headers) / len;

         for (int i = 0; i < cnt; i++)
             if (matchingFileHeader(path, headers[i], len)) return true;

         return false;
     }

    return false;
}

EncryptedRomFile *
EncryptedRomFile::makeWithBuffer(const u8 *buffer, size_t length)
{
    EncryptedRomFile *rom = new EncryptedRomFile();
    
    if (!rom->readFromBuffer(buffer, length)) {
        delete rom;
        return NULL;
    }
    
    return rom;
}

EncryptedRomFile *
EncryptedRomFile::makeWithFile(const char *path)
{
    EncryptedRomFile *rom = new EncryptedRomFile();
    
    if (!rom->readFromFile(path)) {
        delete rom;
        return NULL;
    }
    
    printf("Made encr Rom at %p\n", rom);    
    return rom;
}

bool
EncryptedRomFile::readFromBuffer(const u8 *buffer, size_t length)
{
    if (!AmigaFile::readFromBuffer(buffer, length))
        return false;
    
    return isEncryptedRomBuffer(buffer, length);
}

RomFile *
EncryptedRomFile::decrypt(DecryptionError *error)
{
    const size_t headerSize = 11;

    RomFile *rom = NULL;
    u8 *encryptedData = NULL;
    u8 *decryptedData = NULL;
    u8 *romKeyData = NULL;
    long romKeySize = 0;
    
    //  Load the rom.key file
    assert(path != NULL);
    char *romKeyPath = replaceFilename(path, "rom.key");
    if (romKeyPath == NULL) {
        if (error) *error = DECRYPT_MISSING_ROM_KEY_FILE;
        goto exit;
    }
    if (!loadFile(romKeyPath, &romKeyData, &romKeySize)) {
        if (error) *error = DECRYPT_MISSING_ROM_KEY_FILE;
        goto exit;
    }
    
    // Create a buffer for the decrypted data
    encryptedData = data + headerSize;
    decryptedData = new u8[size - headerSize];
        
    // Decrypt
    for (size_t i = 0; i < size - headerSize; i++) {
        decryptedData[i] = encryptedData[i] ^ romKeyData[i % romKeySize];
    }
    
    // Convert decrypted data into a Rom
    rom = RomFile::makeWithBuffer(decryptedData, size - headerSize);
    if (rom == NULL) {
        if (error) *error = DECRYPT_INVALID_ROM_KEY_FILE;
    }
    
exit:
    if (romKeyPath) free(romKeyPath);
    if (decryptedData) delete [] decryptedData;
    if (romKeyData) delete [] romKeyData;
    return rom;
}
