// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CONTROL_PORT_H
#define _CONTROL_PORT_H

#include "AmigaComponent.h"
#include "Mouse.h"
#include "Joystick.h"

class ControlPort : public AmigaComponent {

    friend class Mouse;
    friend class Joystick;
    
    // Represented control port
    PortNr nr;

    // Result of the latest inspection
    ControlPortInfo info;
    
    // Connected device
    ControlPortDevice device = CPD_NONE;
    
    // The two mouse position counters
    i64 mouseCounterX = 0;
    i64 mouseCounterY = 0;

    // Resistances on the potentiometer lines (specified as a delta charge)
    double chargeDX;
    double chargeDY;

    //
    // Input sources
    //
    
public:
    
    Mouse mouse = Mouse(amiga, *this);
    Joystick joystick = Joystick(amiga, *this);


    //
    // Initializing
    //
    
public:
    
    ControlPort(Amiga& ref, PortNr nr);

    void _reset(bool hard) override { RESET_SNAPSHOT_ITEMS(hard) }

    
    //
    // Configuring
    //

public:
    
    ControlPortInfo getInfo() { return HardwareComponent::getInfo(info); }

private:
    
    void _inspect() override;
    void _dump() override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & mouseCounterX
        & mouseCounterY
        & chargeDX
        & chargeDY;
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing
    //

public:

    // Getter for the delta charges
    i16 getChargeDX() { return (i16)chargeDX; }
    i16 getChargeDY() { return (i16)chargeDY; }
    
    // Returns the control port bits showing up in the JOYxDAT register
    u16 joydat();

    // Emulates a write access to JOYTEST
    void pokeJOYTEST(u16 value);

    // Modifies the POTGOR bits according to the connected device
    void changePotgo(u16 &potgo);

    // Modifies the PRA bits of CIA A according to the connected device
    void changePra(u8 &pra);    
};

#endif

