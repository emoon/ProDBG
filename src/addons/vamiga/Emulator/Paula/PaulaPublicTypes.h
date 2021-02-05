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

#include "AudioPublicTypes.h"
#include "DiskControllerPublicTypes.h"

//
// Structures
//

typedef struct
{
    u16 intreq;
    u16 intena;
    u16 adkcon;
}
PaulaInfo;

typedef struct
{
    u16 receiveBuffer;
    u16 receiveShiftReg;
    u16 transmitBuffer;
    u16 transmitShiftReg;
}
UARTInfo;
