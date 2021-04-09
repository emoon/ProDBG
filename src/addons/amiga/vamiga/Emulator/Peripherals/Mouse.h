// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "MouseTypes.h"
#include "Joystick.h"
#include "AmigaComponent.h"
#include "Chrono.h"

class ShakeDetector {
    
    // Horizontal position
    double x = 0.0;
    
    // Moved distance
    double dxsum = 0.0;

    // Direction (1 or -1)
    double dxsign = 1.0;
    
    // Number of turns
    isize dxturns = 0;
    
    // Time stamps
    u64 lastTurn = 0;
    util::Time lastShake;
    
public:
    
    // Feed in new coordinates and checks for a shake
    bool isShakingAbs(double x);
    bool isShakingRel(double dx);
};

class Mouse : public AmigaComponent {

    // Reference to the control port this device belongs to
    ControlPort &port;
    
    // Current configuration
    MouseConfig config;

    // Shake detector
    class ShakeDetector shakeDetector;
    
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
    
    // Scaling factors applied to the raw mouse coordinates in setXY()
    double scaleX = 1.0;
    double scaleY = 1.0;
    
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
    
public:
    
    const MouseConfig &getConfig() const { return config; }

    long getConfigItem(Option option) const;
    bool setConfigItem(Option option, long id, long value) override;
    
private:
    
    void updateScalingFactors();
    
    
    //
    // Analyzing
    //
    
private:
    
    void _dump(Dump::Category category, std::ostream& os) const override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker << config.pullUpResistors;
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
