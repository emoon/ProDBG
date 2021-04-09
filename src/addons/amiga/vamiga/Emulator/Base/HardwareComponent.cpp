// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "HardwareComponent.h"

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
HardwareComponent::inspect()
{
    // Inspect all subcomponents
    for (HardwareComponent *c : subComponents) {
        c->inspect();
    }
    
    // Inspect this component
    _inspect();
}

void HardwareComponent::dump(Dump::Category category, std::ostream& ss) const
{
    _dump(category, ss);
}

void
HardwareComponent::dump(Dump::Category category) const
{
    dump(category, std::cout);
}

void
HardwareComponent::dump(std::ostream& ss) const
{
    dump((Dump::Category)(-1), ss);
}

void
HardwareComponent::dump() const
{
    dump((Dump::Category)(-1));
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
    isize result;
    
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
    result = (isize)(ptr - buffer);
    
    // Verify that the number of written bytes matches the snapshot size
    trace(SNP_DEBUG, "Loaded %zd bytes (expected %zd)\n", result, size());
    assert(result == size());

    return result;
}

isize
HardwareComponent::save(u8 *buffer)
{
    u8 *ptr = buffer;
    isize result;
    
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
    result = (isize)(ptr - buffer);
    
    // Verify that the number of written bytes matches the snapshot size
    trace(SNP_DEBUG, "Saved %zd bytes (expected %zd)\n", result, size());
    assert(result == size());

    return result;
}

void
HardwareComponent::powerOn()
{
    for (auto c : subComponents) { c->powerOn(); }
    _powerOn();
}
    
void
HardwareComponent::powerOff()
{
    _powerOff();
    for (auto c : subComponents) { c->powerOff(); }
}

void
HardwareComponent::run()
{
    for (auto c : subComponents) { c->run(); }
    _run();
}

void
HardwareComponent::pause()
{
    _pause();
    for (auto c : subComponents) { c->pause(); }
}

void
HardwareComponent::warpOn()
{
    for (auto c : subComponents) { c->warpOn(); }
    warpMode = true;
    _warpOn();
}

void
HardwareComponent::warpOff()
{
    for (auto c : subComponents) { c->warpOff(); }
    warpMode = false;
    _warpOff();
}

void
HardwareComponent::debugOn()
{    
    for (auto c : subComponents) { c->debugOn(); }
    debugMode = true;
    _debugOn();
}

void
HardwareComponent::debugOff()
{
    for (auto c : subComponents) { c->debugOff(); }
    debugMode = false;
    _debugOff();
}
