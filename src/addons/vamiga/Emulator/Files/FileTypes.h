// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FilePublicTypes.h"
#include "Reflection.h"

struct FileTypeEnum : Reflection<FileTypeEnum, FileType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < FILETYPE_COUNT;
    }
    
    static const char *prefix() { return "FILETYPE"; }
    static const char *key(FileType value)
    {
        switch (value) {
                
            case FILETYPE_UKNOWN:       return "UKNOWN";
            case FILETYPE_SNAPSHOT:     return "SNAPSHOT";
            case FILETYPE_ADF:          return "ADF";
            case FILETYPE_HDF:          return "HDF";
            case FILETYPE_EXT:          return "EXT";
            case FILETYPE_IMG:          return "IMG";
            case FILETYPE_DMS:          return "DMS";
            case FILETYPE_EXE:          return "EXE";
            case FILETYPE_DIR:          return "DIR";
            case FILETYPE_ROM:          return "ROM";
            case FILETYPE_EXTENDED_ROM: return "EXTENDED_ROM";
            case FILETYPE_COUNT:        return "???";
        }
        return "???";
    }
};

struct RomIdentifierEnum : Reflection<RomIdentifierEnum, RomIdentifier> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < ROM_COUNT;
    }
    
    static const char *prefix() { return "ROM"; }
    static const char *key(RomIdentifier value)
    {
        switch (value) {
                
            case ROM_MISSING:           return "MISSING";
            case ROM_UNKNOWN:           return "UNKNOWN";

            case ROM_BOOT_A1000_8K:     return "BOOT_A1000_8K";
            case ROM_BOOT_A1000_64K:    return "BOOT_A1000_64K";

            case ROM_KICK11_31_034:     return "KICK11_31_034";
            case ROM_KICK12_33_166:     return "KICK12_33_166";
            case ROM_KICK12_33_180:     return "KICK12_33_180";
            case ROM_KICK121_34_004:    return "KICK121_34_004";
            case ROM_KICK13_34_005:     return "KICK13_34_005";
            case ROM_KICK13_34_005_SK:  return "KICK13_34_005_SK";

            case ROM_KICK20_36_028:     return "KICK20_36_028";
            case ROM_KICK202_36_207:    return "KICK202_36_207";
            case ROM_KICK204_37_175:    return "KICK204_37_175";
            case ROM_KICK205_37_299:    return "KICK205_37_299";
            case ROM_KICK205_37_300:    return "KICK205_37_300";
            case ROM_KICK205_37_350:    return "KICK205_37_350";

            case ROM_KICK30_39_106:     return "KICK30_39_106";
            case ROM_KICK31_40_063:     return "KICK31_40_063";

            case ROM_HYP314_46_143:     return "HYP314_46_143";

            case ROM_AROS_55696:        return "AROS_55696";
            case ROM_AROS_55696_EXT:    return "AROS_55696_EXT";

            case ROM_DIAG11:            return "DIAG11";
            case ROM_DIAG12:            return "DIAG12";
            case ROM_DIAG121:           return "DIAG121";
            case ROM_LOGICA20:          return "LOGICA20";
                
            case ROM_COUNT:             return "???";
        }
        return "???";
    }
};

struct BootBlockTypeEnum : Reflection<BootBlockTypeEnum, BootBlockType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < BB_COUNT;
    }
    
    static const char *prefix() { return "BB"; }
    static const char *key(DiskDensity value)
    {
        switch (value) {
                
            case DISK_SD:     return "SD";
            case DISK_DD:     return "DD";
            case DISK_HD:     return "HD";
            case DISK_COUNT:  return "???";
        }
        return "???";
    }
};
