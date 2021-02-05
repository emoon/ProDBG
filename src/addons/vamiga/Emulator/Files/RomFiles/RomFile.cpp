// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "RomFile.h"

//
// Boot Roms
//

const u8 RomFile::bootRomHeaders[1][8] = {

    // Amiga 1000 Bootstrap (1985)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00, 0x8A }
};

//
// Kickstart Roms
//

const u8 RomFile::kickRomHeaders[6][7] = {

    // AROS Kickstart replacement
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },

    // Kickstart 1.2 and 1.3
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xFC, 0x00 },

    // Kickstart 2.04
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },

    // Kickstart 3.1
    { 0x11, 0x14, 0x4E, 0xF9, 0x00, 0xF8, 0x00 },
    // { 0x11, 0x16, 0x4E, 0xF9, 0x00, 0x20, 0x00 }, not working
    
    // Diagnostic v2.0 (Logica)
    { 0x11, 0x11, 0x4E, 0xF9, 0x00, 0xF8, 0x04 }
};

//
// Encrypted Kickstart Roms
//

const u8 RomFile::encrRomHeaders[1][11] = {

    // Cloanto Rom Header Signature
    { 'A', 'M', 'I', 'R', 'O', 'M', 'T', 'Y', 'P', 'E', '1' }
};

RomIdentifier
RomFile::identifier(u32 fingerprint)
{
    switch(fingerprint) {

        case 0x00000000: return ROM_MISSING;
            
        case 0x62F11C04: return ROM_BOOT_A1000_8K;
        case 0x0B1AD2D0: return ROM_BOOT_A1000_64K;

        case 0xEC86DAE2: return ROM_KICK11_31_034;
        case 0x9ED783D0: return ROM_KICK12_33_166;
        case 0xA6CE1636: return ROM_KICK12_33_180;
        case 0xDB4C8033: return ROM_KICK121_34_004;
        case 0xC4F0F55F: return ROM_KICK13_34_005;
        case 0xE0F37258: return ROM_KICK13_34_005_SK;

        case 0xB4113910: return ROM_KICK20_36_028;

        case 0x9A15519D: return ROM_KICK202_36_207;
        case 0xC3BDB240: return ROM_KICK204_37_175;
        case 0x83028FB5: return ROM_KICK205_37_299;
        case 0x64466C2A: return ROM_KICK205_37_300;
        case 0x43B0DF7B: return ROM_KICK205_37_350;

        case 0x6C9B07D2: return ROM_KICK30_39_106;
        case 0xFC24AE0D: return ROM_KICK31_40_063;

        case 0xD52B52FD: return ROM_HYP314_46_143;

        case 0x3F4FCC0A: return ROM_AROS_55696;
        case 0xF2E52B07: return ROM_AROS_55696_EXT;

        case 0x4C4B5C05: return ROM_DIAG11;
        case 0x771CD0EA: return ROM_DIAG12;
        case 0x850209CD: return ROM_DIAG121;
        case 0x8484F426: return ROM_LOGICA20;

        default: return ROM_UNKNOWN;
    }
}

bool
RomFile::isCompatibleName(const string &name)
{
    return true;
}

bool
RomFile::isBootRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_BOOT_A1000_8K:
        case ROM_BOOT_A1000_64K: return true;

        default: return false;
    }
}

bool
RomFile::isArosRom(RomIdentifier rev)
{
    switch (rev) {

        case 0x00000000: return ROM_MISSING;

        case ROM_AROS_55696:
        case ROM_AROS_55696_EXT: return true;

        default: return false;
    }
}

bool
RomFile::isDiagRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_DIAG11:
        case ROM_DIAG12:
        case ROM_DIAG121:
        case ROM_LOGICA20: return true;

        default: return false;
    }
}

bool
RomFile::isCommodoreRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_BOOT_A1000_8K:
        case ROM_BOOT_A1000_64K:

        case ROM_KICK11_31_034:
        case ROM_KICK12_33_166:
        case ROM_KICK12_33_180:
        case ROM_KICK121_34_004:
        case ROM_KICK13_34_005:
        case ROM_KICK13_34_005_SK:

        case ROM_KICK20_36_028:
        case ROM_KICK202_36_207:
        case ROM_KICK204_37_175:
        case ROM_KICK205_37_299:
        case ROM_KICK205_37_300:
        case ROM_KICK205_37_350:

        case ROM_KICK30_39_106:
        case ROM_KICK31_40_063: return true;

        default: return false;
    }
}

bool
RomFile::isHyperionRom(RomIdentifier rev)
{
    switch (rev) {

        case ROM_HYP314_46_143: return true;

        default: return false;
    }
}

const char *
RomFile::title(RomIdentifier rev)
{
    switch (rev) {

        case ROM_UNKNOWN:          return "Unknown or patched Rom";

        case ROM_BOOT_A1000_8K:
        case ROM_BOOT_A1000_64K:   return "Amiga 1000 Boot Rom";

        case ROM_KICK11_31_034:
        case ROM_KICK12_33_166:
        case ROM_KICK12_33_180:    return "Kickstart 1.2";
        case ROM_KICK121_34_004:   return "Kickstart 1.21";
        case ROM_KICK13_34_005:
        case ROM_KICK13_34_005_SK: return "Kickstart 1.3";

        case ROM_KICK20_36_028:    return "Kickstart 2.0";
        case ROM_KICK202_36_207:   return "Kickstart 2.02";
        case ROM_KICK204_37_175:   return "Kickstart 2.04";
        case ROM_KICK205_37_299:
        case ROM_KICK205_37_300:
        case ROM_KICK205_37_350:   return "Kickstart 2.05";

        case ROM_KICK30_39_106:    return "Kickstart 3.0";
        case ROM_KICK31_40_063:    return "Kickstart 3.1";

        case ROM_HYP314_46_143:    return "Kickstart 3.14 (Hyperion)";

        case ROM_AROS_55696:       return "AROS Kickstart replacement";
        case ROM_AROS_55696_EXT:   return "AROS Kickstart extension";

        case ROM_DIAG11:
        case ROM_DIAG12:
        case ROM_DIAG121:          return "Amiga DiagROM";
        case ROM_LOGICA20:         return "Logica Diagnostic";

        default:                   return "";
    }
}

const char *
RomFile::version(RomIdentifier rev)
{
    switch (rev) {
            
        case ROM_BOOT_A1000_8K:     return "8KB";
        case ROM_BOOT_A1000_64K:    return "64KB";

        case ROM_KICK11_31_034:     return "Rev 31.034";
        case ROM_KICK12_33_166:     return "Rev 31.034";
        case ROM_KICK12_33_180:     return "Rev 33.180";
        case ROM_KICK121_34_004:    return "Rev 34.004";
        case ROM_KICK13_34_005:     return "Rev 34.005";
        case ROM_KICK13_34_005_SK:  return "Rev 34.005 (A3000 SK)";

        case ROM_KICK20_36_028:     return "Rev 36.028";
        case ROM_KICK202_36_207:    return "Rev 36.207";
        case ROM_KICK204_37_175:    return "Rev 37.175";
        case ROM_KICK205_37_299:    return "Rev 37.299";
        case ROM_KICK205_37_300:    return "Rev 37.300";
        case ROM_KICK205_37_350:    return "Rev 37.350";

        case ROM_KICK30_39_106:     return "Rev 39.106";
        case ROM_KICK31_40_063:     return "Rev 40.063";

        case ROM_HYP314_46_143:     return "Rev 46.143";

        case ROM_AROS_55696:        return "SVN 55696";
        case ROM_AROS_55696_EXT:    return "SVN 55696";

        case ROM_DIAG11:            return "Version 1.1";
        case ROM_DIAG12:            return "Version 1.2";
        case ROM_DIAG121:           return "Version 1.2.1";
        case ROM_LOGICA20:          return "Version 2.0";

        default:                    return "";
    }
}

const char *
RomFile::released(RomIdentifier rev)
{
    switch (rev) {

        case ROM_BOOT_A1000_8K:     return "1985";
        case ROM_BOOT_A1000_64K:    return "1985";

        case ROM_KICK11_31_034:     return "February 1986";
        case ROM_KICK12_33_166:     return "September 1986";
        case ROM_KICK12_33_180:     return "October 1986";
        case ROM_KICK121_34_004:    return "November 1987";
        case ROM_KICK13_34_005:     return "December 1987";
        case ROM_KICK13_34_005_SK:  return "December 1987";

        case ROM_KICK20_36_028:     return "March 1990";
        case ROM_KICK202_36_207:    return "October 1990";
        case ROM_KICK204_37_175:    return "May 1991";
        case ROM_KICK205_37_299:    return "November 1991";
        case ROM_KICK205_37_300:    return "November 1991";
        case ROM_KICK205_37_350:    return "April 1992";

        case ROM_KICK30_39_106:     return "September 1992";
        case ROM_KICK31_40_063:     return "July 1993";

        case ROM_HYP314_46_143:     return "2018";

        case ROM_AROS_55696:        return "February 2019";
        case ROM_AROS_55696_EXT:    return "February 2019";

        case ROM_DIAG11:            return "October 2018";
        case ROM_DIAG12:            return "August 2019";
        case ROM_DIAG121:           return "July 2020";
        case ROM_LOGICA20:          return "";

        default:                    return "";
    }
}

RomFile::RomFile()
{
}

bool
RomFile::isCompatibleStream(std::istream &stream)
{
    isize length = streamLength(stream);
    
    // Boot Roms
    if (length == KB(8) || length == KB(16)) {

        isize len = isizeof(bootRomHeaders[0]);
        isize cnt = isizeof(bootRomHeaders) / len;

        for (isize i = 0; i < cnt; i++) {
            if (matchingStreamHeader(stream, bootRomHeaders[i], len)) return true;
        }
        return false;
    }

    // Kickstart Roms
    if (length == KB(256) || length == KB(512)) {

        isize len = isizeof(kickRomHeaders[0]);
        isize cnt = isizeof(kickRomHeaders) / len;

        for (isize i = 0; i < cnt; i++) {
            if (matchingStreamHeader(stream, kickRomHeaders[i], len)) return true;
        }
        return false;
    }

    // Encrypted Kickstart Roms
    if (length == KB(256) + 11 || length == KB(512) + 11) {
        
        isize len = isizeof(encrRomHeaders[0]);
        isize cnt = isizeof(encrRomHeaders) / len;
        
        for (isize i = 0; i < cnt; i++) {
            if (matchingStreamHeader(stream, encrRomHeaders[i], len)) return true;
        }
    }
    
    return false;
}

bool
RomFile::isRomBuffer(const u8 *buf, isize len)
{
    std::stringstream stream;
    stream.write((const char *)buf, len);
    
    return isCompatibleStream(stream);
}

bool
RomFile::isRomFile(const char *path)
{
    std::ifstream stream(path);
    return stream.is_open() ? isCompatibleStream(stream) : false;
}

bool
RomFile::isEncrypted()
{
    return matchingBufferHeader(data, encrRomHeaders[0], sizeof(encrRomHeaders[0]));
}

void
RomFile::decrypt()
{
    const isize headerSize = 11;
    u8 *encryptedData = nullptr;
    u8 *decryptedData = nullptr;
    u8 *romKeyData = nullptr;
    long romKeySize = 0;
        
    // Only proceed if the file is encrypted
    if (!isEncrypted()) return;
    
    // Locate the rom.key file
    romKeyPath = extractPath(path) + "rom.key";
    
    // Load the rom.key file
    if (!loadFile(romKeyPath.c_str(), &romKeyData, &romKeySize)) {
        throw VAError(ERROR_MISSING_ROM_KEY);
    }
    
    // Create a buffer for the decrypted data
    encryptedData = data + headerSize;
    decryptedData = new u8[size - headerSize];
        
    // Decrypt
    for (isize i = 0; i < size - headerSize; i++) {
        decryptedData[i] = encryptedData[i] ^ romKeyData[i % romKeySize];
    }
    delete [] romKeyData;

    // Replace the old data by the decrypted data
    delete [] data;
    data = decryptedData;
    size -= headerSize;
    
    // Check if we've got a valid ROM
    if (!isRomBuffer(data, size)) {
        throw VAError(ERROR_INVALID_ROM_KEY);
    }
}
