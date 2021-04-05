// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _KEYBOARD_PRIVATE_TYPES_H
#define _KEYBOARD_PRIVATE_TYPES_H

enum KeyboardState : long
{
    KB_SELFTEST,
    KB_SYNC,
    KB_STRM_ON,
    KB_STRM_OFF,
    KB_SEND
};

inline bool isKeyboardState(long value) {
    return value >= 0 && value <= KB_SEND;
}

#endif
