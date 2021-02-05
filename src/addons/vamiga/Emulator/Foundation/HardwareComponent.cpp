// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

HardwareComponent::~HardwareComponent()
{
    debug(RUN_DEBUG, "Terminated\n");
}

void
HardwareComponent::initialize()
{
    // Initialize all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->initialize();
    }

    // Initialize this component
    _initialize();
}

void
HardwareComponent::reset(bool hard)
{
    // Reset all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->reset(hard);
    }
    
    // Reset this component
    debug(RUN_DEBUG, "Reset [%p]\n", this);
    _reset(hard);
}

bool
HardwareComponent::configure(Option option, long value)
{
    bool result = false;
    
    // Configure all subcomponents
    for (HardwareComponent *c : subComponents) {
        result |= c->configure(option, value);
    }
    
    // Configure this component
    result |= setConfigItem(option, value);

    return result;
}

bool
HardwareComponent::configure(Option option, long id, long value)
{
    bool result = false;
    
    // Configure all subcomponents
    for (HardwareComponent *c : subComponents) {
        result |= c->configure(option, id, value);
    }
    
    // Configure this component
    result |= setConfigItem(option, id, value);

    return result;
}

void
HardwareComponent::dumpConfig() const
{
    // Dump the configuration of all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->dumpConfig();
    }

    // Dump the configuration of this component
    msg("%s (%p):\n", getDescription(), this);
    _dumpConfig();
    msg("\n");
}

void
HardwareComponent::inspect()
{
    // Inspect all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->inspect();
    }
    
    // Inspect this component
    _inspect();
}

void
HardwareComponent::dump()
{
    msg("%s (memory location: %p)\n\n", getDescription(), this);
    _dump();
}

isize
HardwareComponent::size()
{
    isize result = _size();

    for (HardwareComponent *c : subComponents) {
        result += c->size();
    }

    return result;
}

isize
HardwareComponent::load(const u8 *buffer)
{
    const u8 *ptr = buffer;

    // Call delegation method
    ptr += willLoadFromBuffer(ptr);

    // Load internal state of all subcomponents
    for (HardwareComponent *c : subComponents) {
        ptr += c->load(ptr);
    }

    // Load internal state of this component
    ptr += _load(ptr);

    // Call delegation method
    ptr += didLoadFromBuffer(ptr);

    // Verify that the number of written bytes matches the snapshot size
    trace(SNP_DEBUG, "Loaded %ld bytes (expected %zu)\n", ptr - buffer, size());
    assert(ptr - buffer == size());

    return ptr - buffer;
}

isize
HardwareComponent::save(u8 *buffer)
{
    u8 *ptr = buffer;

    // Call delegation method
    ptr += willSaveToBuffer(ptr);

    // Save internal state of all subcomponents
    for (HardwareComponent *c : subComponents) {
        ptr += c->save(ptr);
    }

    // Save internal state of this component
    ptr += _save(ptr);

    // Call delegation method
    ptr += didSaveToBuffer(ptr);

    // Verify that the number of written bytes matches the snapshot size
    trace(SNP_DEBUG, "Saved %ld bytes (expected %zu)\n", ptr - buffer, size());
    assert(ptr - buffer == size());

    return ptr - buffer;
}

void
HardwareComponent::powerOn()
{
    if (isPoweredOff()) {

        assert(!isRunning());
        
        // Power all subcomponents on
        for (HardwareComponent *c : subComponents) {
            c->powerOn();
        }
        
        // Reset all non-persistant snapshot items
        _reset(true);

        // Power this component on
        debug(RUN_DEBUG, "Powering on\n");
        state = EMULATOR_STATE_PAUSED;
        _powerOn();
    }
}

void
HardwareComponent::powerOff()
{
    if (isPoweredOn()) {
        
        // Pause if needed
        pause();
        
        // Power off this component
        debug(RUN_DEBUG, "Powering off\n");
        state = EMULATOR_STATE_OFF;
        _powerOff();

        // Power all subcomponents off
        for (HardwareComponent *c : subComponents) {
            c->powerOff();
        }
    }
}

void
HardwareComponent::run()
{
    if (!isRunning()) {
        
        // Power on if needed
        powerOn();
            
        // Start all subcomponents
        for (HardwareComponent *c : subComponents) {
            c->run();
        }
        
        // Start this component
        debug(RUN_DEBUG, "Run\n");
        state = EMULATOR_STATE_RUNNING;
        _run();
    }
}

void
HardwareComponent::pause()
{
    if (isRunning()) {
        
        // Pause this component
        debug(RUN_DEBUG, "Pause\n");
        state = EMULATOR_STATE_PAUSED;
        _pause();

        // Pause all subcomponents
        for (HardwareComponent *c : subComponents) {
            c->pause();
        }
    }
}

void
HardwareComponent::setWarp(bool enable)
{
    if (warpMode == enable) return;
    
    warpMode = enable;

     // Enable or disable warp mode for all subcomponents
     for (HardwareComponent *c : subComponents) {
         c->setWarp(enable);
     }

     // Enable warp mode for this component
     _setWarp(enable);
}

void
HardwareComponent::setDebug(bool enable)
{
    if (debugMode == enable) return;
    
    debugMode = enable;

     // Enable or disable debug mode for all subcomponents
     for (HardwareComponent *c : subComponents) {
         c->setDebug(enable);
     }

     // Enable debug mode for this component
     _setDebug(enable);
}
