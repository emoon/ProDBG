// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------
// THIS FILE MUST CONFORM TO ANSI-C TO BE COMPATIBLE WITH SWIFT
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

//
// Enumerations
//

enum_long(RTC_REVISION)
{
    RTC_NONE,
    RTC_OKI,
    RTC_RICOH,
    
    RTC_COUNT
};
typedef RTC_REVISION RTCRevision;


//
// Structures
//

typedef struct
{
    RTCRevision model;
}
RTCConfig;
