// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "PortPublicTypes.h"
#include "Reflection.h"

struct SerialPortDeviceEnum : Reflection<SerialPortDeviceEnum, SerialPortDevice> {
    
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

struct PortNrEnum : Reflection<PortNrEnum, PortNr> {
    
    static bool isValid(long value)
    {
        return value == PORT_1 || PORT_2;
    }
    
    static const char *prefix() { return ""; }
    static const char *key(PortNr value)
    {
        switch (value) {
                
            case PORT_1:  return "PORT_1";
            case PORT_2:  return "PORT_2";
        }
        return "???";
    }
};

struct ControlPortDeviceEnum : Reflection<ControlPortDeviceEnum, ControlPortDevice> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <  CPD_COUNT;
    }
    
    static const char *prefix() { return "CPD"; }
    static const char *key(ControlPortDevice value)
    {
        switch (value) {
                
            case CPD_NONE:      return "NONE";
            case CPD_MOUSE:     return "MOUSE";
            case CPD_JOYSTICK:  return "JOYSTICK";
            case CPD_COUNT:     return "???";
        }
        return "???";
    }
};

struct GamePadActionEnum : Reflection<GamePadActionEnum, GamePadAction> {
    
    static bool isValid(long value)
    {
        return (unsigned long)value <= RELEASE_RIGHT;
    }
    
    static const char *prefix() { return nullptr; }
    static const char *key(GamePadAction value)
    {
        switch (value) {
                
            case PULL_UP:        return "PULL_UP";
            case PULL_DOWN:      return "PULL_DOWN";
            case PULL_LEFT:      return "PULL_LEFT";
            case PULL_RIGHT:     return "PULL_RIGHT";
            case PRESS_FIRE:     return "PRESS_FIRE";
            case PRESS_LEFT:     return "PRESS_LEFT";
            case PRESS_RIGHT:    return "PRESS_RIGHT";
            case RELEASE_X:      return "RELEASE_X";
            case RELEASE_Y:      return "RELEASE_Y";
            case RELEASE_XY:     return "RELEASE_XY";
            case RELEASE_FIRE:   return "RELEASE_FIRE";
            case RELEASE_LEFT:   return "RELEASE_LEFT";
            case RELEASE_RIGHT:  return "RELEASE_RIGHT";
        }
        return "???";
    }
};
