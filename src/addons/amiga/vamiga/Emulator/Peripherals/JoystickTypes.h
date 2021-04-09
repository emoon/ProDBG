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

#ifdef __cplusplus
struct GamePadActionEnum : util::Reflection<GamePadActionEnum, GamePadAction> {
    
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
#endif
