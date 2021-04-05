// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Mouse::Mouse(Amiga& ref, ControlPort& pref) : AmigaComponent(ref), port(pref)
{
    setDescription(port.nr == PORT_1 ? "Mouse1" : "Mouse2");

    config.pullUpResistors = true;
}

void Mouse::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    leftButton = false;
    rightButton = false;
    mouseX = 0;
    mouseY = 0;
    oldMouseX = 0;
    oldMouseY = 0;
    targetX = 0;
    targetY = 0;
}

void
Mouse::_dump()
{
    msg(" leftButton = %d\n", leftButton);
    msg("rightButton = %d\n", rightButton);
    msg("     mouseX = %f\n", mouseX);
    msg("     mouseY = %f\n", mouseY);
    msg("  oldMouseX = %f\n", oldMouseX);
    msg("  oldMouseY = %f\n", oldMouseY);
    msg("    targetX = %f\n", targetX);
    msg("    targetY = %f\n", targetY);
    msg("   dividerX = %f\n", dividerX);
    msg("   dividerY = %f\n", dividerY);
    msg("     shiftX = %f\n", shiftX);
    msg("     shiftY = %f\n", shiftY);
}

void
Mouse::changePotgo(u16 &potgo)
{
    u16 mask = port.nr == 1 ? 0x0400 : 0x4000;

    if (rightButton || HOLD_MOUSE_R) {
        potgo &= ~mask;
    } else if (config.pullUpResistors) {
        potgo |= mask;
    }
}

void
Mouse::changePra(u8 &pra)
{
    u16 mask = port.nr == 1 ? 0x0040 : 0x0080;

    if (leftButton || HOLD_MOUSE_L) {
        pra &= ~mask;
    } else if (config.pullUpResistors) {
        pra |= mask;
    }
}

i64
Mouse::getDeltaX()
{
    execute();

    i64 result = (i16)(mouseX - oldMouseX);
    oldMouseX = mouseX;

    return result;
}

i64
Mouse::getDeltaY()
{
    execute();

    i64 result = (i16)(mouseY - oldMouseY);
    oldMouseY = mouseY;

    return result;
}

u16
Mouse::getXY()
{
    // Update mouseX and mouseY
    execute();
    
    // Assemble the result
    return HI_LO((u16)mouseY & 0xFF, (u16)mouseX & 0xFF);
}

void
Mouse::setXY(double x, double y)
{
    targetX = x / dividerX;
    targetY = y / dividerY;
    port.device = CPD_MOUSE;
}

void
Mouse::setDeltaXY(double dx, double dy)
{
    targetX += dx / dividerX;
    targetY += dy / dividerY;
    port.device = CPD_MOUSE;
}

void
Mouse::setLeftButton(bool value)
{
    trace(PORT_DEBUG, "setLeftButton(%d)\n", value);
    
    leftButton = value;
    port.device = CPD_MOUSE;
}

void
Mouse::setRightButton(bool value)
{
    trace(PORT_DEBUG, "setRightButton(%d)\n", value);
    
    rightButton = value;
    port.device = CPD_MOUSE;
}

void
Mouse::trigger(GamePadAction event)
{
    assert(isGamePadAction(event));

    trace(PORT_DEBUG, "trigger(%d)\n", event);

    switch (event) {

        case PRESS_LEFT: setLeftButton(true); break;
        case RELEASE_LEFT: setLeftButton(false); break;
        case PRESS_RIGHT: setRightButton(true); break;
        case RELEASE_RIGHT: setRightButton(false); break;
        default: break;
    }
}

void
Mouse::execute()
{
    mouseX = targetX;
    mouseY = targetY;
}
