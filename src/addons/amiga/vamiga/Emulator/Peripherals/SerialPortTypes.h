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

//
// Enumerations
//

enum_long(SPD)
{
    SPD_NONE,
    SPD_LOOPBACK,
    
    SPD_COUNT
};
typedef SPD SerialPortDevice;

#ifdef __cplusplus
struct SerialPortDeviceEnum : util::Reflection<SerialPortDeviceEnum, SerialPortDevice> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value < SPD_COUNT;
    }
    
    static const char *prefix() { return "SPD"; }
    static const char *key(SerialPortDevice value)
    {
        switch (value) {
                
            case SPD_NONE:      return "NONE";
            case SPD_LOOPBACK:  return "LOOPBACK";
            case SPD_COUNT:     return "???";
        }
        return "???";
    }
};
#endif

//
// Structures
//

typedef struct
{
    SerialPortDevice device;
}
SerialPortConfig;

typedef struct
{
    u32 port;

    bool txd;
    bool rxd;
    bool rts;
    bool cts;
    bool dsr;
    bool cd;
    bool dtr;
}
SerialPortInfo;
