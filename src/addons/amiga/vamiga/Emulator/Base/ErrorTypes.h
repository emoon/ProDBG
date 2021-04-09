// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "Reflection.h"

enum_long(ERROR_CODE)
{
    ERROR_OK,
    ERROR_UNKNOWN,
    
    // General
    ERROR_FILE_NOT_FOUND,
    ERROR_FILE_TYPE_MISMATCH,
    ERROR_FILE_CANT_READ,
    ERROR_FILE_CANT_WRITE,
    ERROR_FILE_CANT_CREATE,

    // Memory
    ERROR_OUT_OF_MEMORY,
    ERROR_CHIP_RAM_LIMIT,
    ERROR_AROS_RAM_LIMIT,

    // Roms
    ERROR_ROM_MISSING,
    ERROR_AROS_NO_EXTROM,
    
    // Floppy disks
    ERROR_DISK_CANT_DECODE,
    ERROR_DISK_INVALID_DIAMETER,
    ERROR_DISK_INVALID_DENSITY,
    
    // Snapshots
    ERROR_SNP_TOO_OLD,
    ERROR_SNP_TOO_NEW,
    ERROR_UNSUPPORTED_SNAPSHOT,  // DEPRECATED
    
    // Encrypted Roms
    ERROR_MISSING_ROM_KEY,
    ERROR_INVALID_ROM_KEY,
    
    // File system
    ERROR_FS_UNKNOWN,
    ERROR_FS_UNSUPPORTED,
    ERROR_FS_WRONG_BSIZE,
    ERROR_FS_WRONG_CAPACITY,
    ERROR_FS_HAS_CYCLES,
    ERROR_FS_CORRUPTED,

    // File system (export errors)
    ERROR_FS_DIRECTORY_NOT_EMPTY,
    ERROR_FS_CANNOT_CREATE_DIR,
    ERROR_FS_CANNOT_CREATE_FILE,

    // File system (block errors)
    ERROR_FS_EXPECTED_VALUE,
    ERROR_FS_EXPECTED_SMALLER_VALUE,
    ERROR_FS_EXPECTED_DOS_REVISION,
    ERROR_FS_EXPECTED_NO_REF,
    ERROR_FS_EXPECTED_REF,
    ERROR_FS_EXPECTED_SELFREF,
    ERROR_FS_PTR_TO_UNKNOWN_BLOCK,
    ERROR_FS_PTR_TO_EMPTY_BLOCK,
    ERROR_FS_PTR_TO_BOOT_BLOCK,
    ERROR_FS_PTR_TO_ROOT_BLOCK,
    ERROR_FS_PTR_TO_BITMAP_BLOCK,
    ERROR_FS_PTR_TO_BITMAP_EXT_BLOCK,
    ERROR_FS_PTR_TO_USERDIR_BLOCK,
    ERROR_FS_PTR_TO_FILEHEADER_BLOCK,
    ERROR_FS_PTR_TO_FILELIST_BLOCK,
    ERROR_FS_PTR_TO_DATA_BLOCK,
    ERROR_FS_EXPECTED_DATABLOCK_NR,
    ERROR_FS_INVALID_HASHTABLE_SIZE,
    
    ERROR_COUNT
};
typedef ERROR_CODE ErrorCode;

#ifdef __cplusplus
struct ErrorCodeEnum : util::Reflection<ErrorCodeEnum, ErrorCode> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < ERROR_COUNT;
    }
    
    static const char *prefix() { return "ERROR"; }
    static const char *key(ErrorCode value)
    {
        switch (value) {
                
            case ERROR_OK:                          return "OK";
            case ERROR_UNKNOWN:                     return "UNKNOWN";
                
            case ERROR_FILE_NOT_FOUND:              return "FILE_NOT_FOUND";
            case ERROR_FILE_TYPE_MISMATCH:          return "INVALID_TYPE";
            case ERROR_FILE_CANT_READ:              return "CANT_READ";
            case ERROR_FILE_CANT_WRITE:             return "CANT_WRITE";
            case ERROR_FILE_CANT_CREATE:            return "CANT_CREATE";

            case ERROR_OUT_OF_MEMORY:               return "OUT_OF_MEMORY";
            case ERROR_CHIP_RAM_LIMIT:              return "CHIP_RAM_LIMIT";
            case ERROR_AROS_RAM_LIMIT:              return "AROS_RAM_LIMIT";

            case ERROR_ROM_MISSING:                 return "ROM_MISSING";
            case ERROR_AROS_NO_EXTROM:              return "AROS_NO_EXTROM";

            case ERROR_DISK_CANT_DECODE:            return "DISK_CANT_DECODE";
            case ERROR_DISK_INVALID_DIAMETER:       return "DISK_INVALID_DIAMETER";
            case ERROR_DISK_INVALID_DENSITY:        return "DISK_INVALID_DENSITY";
                
            case ERROR_SNP_TOO_OLD:                 return "SNP_TOO_OLD";
            case ERROR_SNP_TOO_NEW:                 return "SNP_TOO_NEW";
            case ERROR_UNSUPPORTED_SNAPSHOT:        return "UNSUPPORTED_SNAPSHOT";
                
            case ERROR_MISSING_ROM_KEY:             return "MISSING_ROM_KEY";
            case ERROR_INVALID_ROM_KEY:             return "INVALID_ROM_KEY";
                
            case ERROR_FS_UNKNOWN:                  return "FS_UNKNOWN";
            case ERROR_FS_UNSUPPORTED:              return "FS_UNSUPPORTED";
            case ERROR_FS_WRONG_BSIZE:              return "FS_WRONG_BSIZE";
            case ERROR_FS_WRONG_CAPACITY:           return "FS_WRONG_CAPACITY";
            case ERROR_FS_HAS_CYCLES:               return "FS_HAS_CYCLES";
            case ERROR_FS_CORRUPTED:                return "FS_CORRUPTED";

            case ERROR_FS_DIRECTORY_NOT_EMPTY:      return "FS_DIRECTORY_NOT_EMPTY";
            case ERROR_FS_CANNOT_CREATE_DIR:        return "FS_CANNOT_CREATE_DIR";
            case ERROR_FS_CANNOT_CREATE_FILE:       return "FS_CANNOT_CREATE_FILE";

            case ERROR_FS_EXPECTED_VALUE:           return "FS_EXPECTED_VALUE";
            case ERROR_FS_EXPECTED_SMALLER_VALUE:   return "FS_EXPECTED_SMALLER_VALUE";
            case ERROR_FS_EXPECTED_DOS_REVISION:    return "FS_EXPECTED_DOS_REVISION";
            case ERROR_FS_EXPECTED_NO_REF:          return "FS_EXPECTED_NO_REF";
            case ERROR_FS_EXPECTED_REF:             return "FS_EXPECTED_REF";
            case ERROR_FS_EXPECTED_SELFREF:         return "FS_EXPECTED_SELFREF";
            case ERROR_FS_PTR_TO_UNKNOWN_BLOCK:     return "FS_PTR_TO_UNKNOWN_BLOCK";
            case ERROR_FS_PTR_TO_EMPTY_BLOCK:       return "FS_PTR_TO_EMPTY_BLOCK";
            case ERROR_FS_PTR_TO_BOOT_BLOCK:        return "FS_PTR_TO_BOOT_BLOCK";
            case ERROR_FS_PTR_TO_ROOT_BLOCK:        return "FS_PTR_TO_ROOT_BLOCK";
            case ERROR_FS_PTR_TO_BITMAP_BLOCK:      return "FS_PTR_TO_BITMAP_BLOCK";
            case ERROR_FS_PTR_TO_BITMAP_EXT_BLOCK:  return "FS_PTR_TO_BITMAP_EXT_BLOCK";
            case ERROR_FS_PTR_TO_USERDIR_BLOCK:     return "FS_PTR_TO_USERDIR_BLOCK";
            case ERROR_FS_PTR_TO_FILEHEADER_BLOCK:  return "FS_PTR_TO_FILEHEADER_BLOCK";
            case ERROR_FS_PTR_TO_FILELIST_BLOCK:    return "FS_PTR_TO_FILELIST_BLOCK";
            case ERROR_FS_PTR_TO_DATA_BLOCK:        return "FS_PTR_TO_DATA_BLOCK";
            case ERROR_FS_EXPECTED_DATABLOCK_NR:    return "FS_EXPECTED_DATABLOCK_NR";
            case ERROR_FS_INVALID_HASHTABLE_SIZE:   return "FS_INVALID_HASHTABLE_SIZE";
                
            case ERROR_COUNT:                       return "???";
        }
        return "???";
    }
};
#endif
