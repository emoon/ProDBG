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

//
// Enumerations
//

enum_long(FILETYPE)
{
    FILETYPE_UKNOWN,
    FILETYPE_SNAPSHOT,
    FILETYPE_ADF,
    FILETYPE_HDF,
    FILETYPE_EXT,
    FILETYPE_IMG,
    FILETYPE_DMS,
    FILETYPE_EXE,
    FILETYPE_DIR,
    FILETYPE_ROM,
    FILETYPE_EXTENDED_ROM,
    
    FILETYPE_COUNT
};
typedef FILETYPE FileType;
