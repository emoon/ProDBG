// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"

class Mouse : public AmigaComponent {

    // Reference to the control port this device belongs to
    ControlPort &port;
    
    // Current configuration
    MouseConfig config;

    // The control port this device is connected to
    // PortNr nr;

public:
    
    // Mouse button states
    bool leftButton;
    bool rightButton;
    
private:
    
    // The current mouse position
    double mouseX;
    double mouseY;

    // Recorded mouse position in getDeltaX() and getDeltaY()
    double oldMouseX;
    double oldMouseY;

    /* The target mouse position. In order to achieve a smooth mouse movement,
     * a new mouse coordinate is not written directly into mouseX and mouseY.
     * Instead, these variables are set. In execute(), mouseX and mouseY are
     * shifted smoothly towards the target positions.
     */
    double targetX;
    double targetY;
    
    // Dividers applied to raw coordinates in setXY()
    const double dividerX = 128;
    const double dividerY = 128;
    
    // Mouse movement in pixels per execution step
    double shiftX = 31;
    double shiftY = 31;


    //
    // Initializing
    //
    
public:
    
    Mouse(Amiga& ref, ControlPort& pref);
    
    const char *getDescription() const override;
    
    void _reset(bool hard) override;
    
    
    //
    // Configuring
    //
    
private:
    
    void _dump() const override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker & config.pullUpResistors;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Accessing
    //

public:

    // Modifies the POTGOR bits according to the current button state
    void changePotgo(u16 &potgo) const;

    // Modifies the PRA bits of CIA A according to the current button state
    void changePra(u8 &pra) const;

    
    //
    // Using the mouse
    //
    
public:
    
    // Returns a horizontal or vertical position change
    i64 getDeltaX();
    i64 getDeltaY();

    // Returns the mouse coordinates as they appear in the JOYDAT register
    u16 getXY();
    
    // Emulates a mouse movement
    void setXY(double x, double y);
    void setDeltaXY(double dx, double dy);

    // Presses or releases a mouse button
    void setLeftButton(bool value);
    void setRightButton(bool value);

    // Triggers a gamepad event
    void trigger(GamePadAction event);

    // Performs periodic actions for this device
    void execute();
};
