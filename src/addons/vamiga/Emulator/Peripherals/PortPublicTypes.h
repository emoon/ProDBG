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

enum_long(SPD)
{
    SPD_NONE,
    SPD_LOOPBACK,
    
    SPD_COUNT
};
typedef SPD SerialPortDevice;

inline bool isSerialPortDevice(long value) {
    return value >= 0 && value <= SPD_LOOPBACK;
}

enum_long(PortNr)
{
    PORT_1 = 1,
    PORT_2 = 2    
};

enum_long(CPD)
{
    CPD_NONE,
    CPD_MOUSE,
    CPD_JOYSTICK,
    
    CPD_COUNT
};
typedef CPD ControlPortDevice;

enum_long(GAME_PAD_ACTION)
{
    PULL_UP = 0,   // Pull the joystick up
    PULL_DOWN,     // Pull the joystick down
    PULL_LEFT,     // Pull the joystick left
    PULL_RIGHT,    // Pull the joystick right
    PRESS_FIRE,    // Press the joystick button
    PRESS_LEFT,    // Press the left mouse button
    PRESS_RIGHT,   // Press the right mouse button
    RELEASE_X,     // Move back to neutral horizontally
    RELEASE_Y,     // Move back to neutral vertically
    RELEASE_XY,    // Move back to neutral
    RELEASE_FIRE,  // Release the joystick button
    RELEASE_LEFT,  // Release the left mouse button
    RELEASE_RIGHT  // Release the right mouse button
};
typedef GAME_PAD_ACTION GamePadAction;


//
// Structures
//

typedef struct
{
    bool m0v;
    bool m0h;
    bool m1v;
    bool m1h;
    u16 joydat;
    u16 potgo;
    u16 potgor;
    u16 potdat;
}
ControlPortInfo;

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

typedef struct
{
    bool pullUpResistors;
}
MouseConfig;
