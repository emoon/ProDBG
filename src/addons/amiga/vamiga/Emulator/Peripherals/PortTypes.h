// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// This file must conform to standard ANSI-C to be compatible with Swift.

#ifndef _PORT_TYPES_H
#define _PORT_TYPES_H

#include "Aliases.h"

//
// Enumerations
//

VAMIGA_ENUM(long, SerialPortDevice)
{
    SPD_NONE,
    SPD_LOOPBACK
};

inline bool isSerialPortDevice(long value) {
    return value >= 0 && value <= SPD_LOOPBACK;
}

VAMIGA_ENUM(long, PortNr)
{
    PORT_1 = 1,
    PORT_2 = 2
};

inline bool isPortNr(long value) {
    return value == PORT_1 || value == PORT_2;
}

VAMIGA_ENUM(long, ControlPortDevice)
{
    CPD_NONE,
    CPD_MOUSE,
    CPD_JOYSTICK
};

inline bool isControlPortDevice(long value) {
    return value >= 0 && value <= CPD_JOYSTICK;
}

VAMIGA_ENUM(long, GamePadAction)
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

inline bool isGamePadAction(long value) {
    return value >= 0 && value <= RELEASE_RIGHT;
}

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

#endif
