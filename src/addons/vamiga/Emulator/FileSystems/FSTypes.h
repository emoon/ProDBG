// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "FSPublicTypes.h"
#include "Reflection.h"

struct FSVolumeTypeEnum : Reflection<FSVolumeTypeEnum, FSVolumeType> {
    
    static bool isValid(long value)
    {
        return value >= FS_NODOS && value <= FS_FFS_LNFS;
    }
    
    static const char *prefix() { return "FS"; }
    static const char *key(FSVolumeType value)
    {
        switch (value) {
                
            case FS_NODOS:     return "NODOS";
            case FS_OFS:       return "OFS";
            case FS_FFS:       return "FFS";
            case FS_OFS_INTL:  return "OFS_INTL";
            case FS_FFS_INTL:  return "FFS_INTL";
            case FS_OFS_DC:    return "OFS_DC";
            case FS_FFS_DC:    return "FFS_DC";
            case FS_OFS_LNFS:  return "OFS_LNFS";
            case FS_FFS_LNFS:  return "FFS_LNFS";
        }
        return "???";
    }
};

struct FSBlockTypeEnum : Reflection<FSBlockTypeEnum, FSBlockType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <= FS_BLOCK_TYPE_COUNT;
    }
    
    static const char *prefix() { return "FS"; }
    static const char *key(FSBlockType value)
    {
        switch (value) {
                
            case FS_UNKNOWN_BLOCK:     return "UNKNOWN_BLOCK";
            case FS_EMPTY_BLOCK:       return "EMPTY_BLOCK";
            case FS_BOOT_BLOCK:        return "BOOT_BLOCK";
            case FS_ROOT_BLOCK:        return "ROOT_BLOCK";
            case FS_BITMAP_BLOCK:      return "BITMAP_BLOCK";
            case FS_BITMAP_EXT_BLOCK:  return "BITMAP_EXT_BLOCK";
            case FS_USERDIR_BLOCK:     return "USERDIR_BLOCK";
            case FS_FILEHEADER_BLOCK:  return "FILEHEADER_BLOCK";
            case FS_FILELIST_BLOCK:    return "FILELIST_BLOCK";
            case FS_DATA_BLOCK_OFS:    return "DATA_BLOCK_OFS";
            case FS_DATA_BLOCK_FFS:    return "DATA_BLOCK_FFS";
            case FS_BLOCK_TYPE_COUNT:  return "???";
        }
        return "???";
    }
};

struct FSItemTypeEnum : Reflection<FSItemTypeEnum, FSItemType> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <= FSI_COUNT;
    }
    
    static const char *prefix() { return "FS"; }
    static const char *key(FSItemType value)
    {
        switch (value) {
                
            case FSI_UNKNOWN:               return "UNKNOWN";
            case FSI_UNUSED:                return "UNUSED";
            case FSI_DOS_HEADER:            return "DOS_HEADER";
            case FSI_DOS_VERSION:           return "DOS_VERSION";
            case FSI_BOOTCODE:              return "BOOTCODE";
            case FSI_TYPE_ID:               return "TYPE_ID";
            case FSI_SUBTYPE_ID:            return "SUBTYPE_ID";
            case FSI_SELF_REF:              return "SELF_REF";
            case FSI_CHECKSUM:              return "CHECKSUM";
            case FSI_HASHTABLE_SIZE:        return "HASHTABLE_SIZE";
            case FSI_HASH_REF:              return "HASH_REF";
            case FSI_PROT_BITS:             return "PROT_BITS";
            case FSI_BCPL_STRING_LENGTH:    return "BCPL_STRING_LENGTH";
            case FSI_BCPL_DISK_NAME:        return "BCPL_DISK_NAME";
            case FSI_BCPL_DIR_NAME:         return "BCPL_DIR_NAME";
            case FSI_BCPL_FILE_NAME:        return "BCPL_FILE_NAME";
            case FSI_BCPL_COMMENT:          return "BCPL_COMMENT";
            case FSI_CREATED_DAY:           return "CREATED_DAY";
            case FSI_CREATED_MIN:           return "CREATED_MIN";
            case FSI_CREATED_TICKS:         return "CREATED_TICKS";
            case FSI_MODIFIED_DAY:          return "MODIFIED_DAY";
            case FSI_MODIFIED_MIN:          return "MODIFIED_MIN";
            case FSI_MODIFIED_TICKS:        return "MODIFIED_TICKS";
            case FSI_NEXT_HASH_REF:         return "NEXT_HASH_REF";
            case FSI_PARENT_DIR_REF:        return "PARENT_DIR_REF";
            case FSI_FILEHEADER_REF:        return "FILEHEADER_REF";
            case FSI_EXT_BLOCK_REF:         return "EXT_BLOCK_REF";
            case FSI_BITMAP_BLOCK_REF:      return "BITMAP_BLOCK_REF";
            case FSI_BITMAP_EXT_BLOCK_REF:  return "BITMAP_EXT_BLOCK_REF";
            case FSI_BITMAP_VALIDITY:       return "BITMAP_VALIDITY";
            case FSI_FILESIZE:              return "FILESIZE";
            case FSI_DATA_BLOCK_NUMBER:     return "DATA_BLOCK_NUMBER";
            case FSI_DATA_BLOCK_REF_COUNT:  return "DATA_BLOCK_REF_COUNT";
            case FSI_FIRST_DATA_BLOCK_REF:  return "FIRST_DATA_BLOCK_REF";
            case FSI_NEXT_DATA_BLOCK_REF:   return "NEXT_DATA_BLOCK_REF";
            case FSI_DATA_BLOCK_REF:        return "DATA_BLOCK_REF";
            case FSI_DATA_COUNT:            return "DATA_COUNT";
            case FSI_DATA:                  return "DATA";
            case FSI_BITMAP:                return "BITMAP";
            case FSI_COUNT:                 return "???";
        }
        return "???";
    }
};
