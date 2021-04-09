// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "AmigaComponent.h"
#include "Amiga.h"  

AmigaComponent::AmigaComponent(Amiga& ref) :
agnus(ref.agnus),
amiga(ref),
blitter(ref.agnus.blitter),
ciaa(ref.ciaA),
ciab(ref.ciaB),
controlPort1(ref.controlPort1),
controlPort2(ref.controlPort2),
copper(ref.agnus.copper),
cpu(ref.cpu),
denise(ref.denise),
diskController(ref.paula.diskController),
dmaDebugger(ref.agnus.dmaDebugger),
df0(ref.df0),
df1(ref.df1),
df2(ref.df2),
df3(ref.df3),
keyboard(ref.keyboard),
mem(ref.mem),
messageQueue(ref.queue),
oscillator(ref.oscillator),
paula(ref.paula),
pixelEngine(ref.denise.pixelEngine),
retroShell(ref.retroShell),
rtc(ref.rtc),
serialPort(ref.serialPort),
uart(ref.paula.uart),
zorro(ref.zorro)
{

};

bool
AmigaComponent::isPoweredOff() const
{
    return amiga.isPoweredOff();
}

bool
AmigaComponent::isPoweredOn() const
{
    return amiga.isPoweredOn();
}

bool
AmigaComponent::isPaused() const
{
    return amiga.isPaused();
}

bool
AmigaComponent::isRunning() const
{
    return amiga.isRunning();
}

void
AmigaComponent::suspend()
{
    amiga.suspend();
}

void
AmigaComponent::resume()
{
    amiga.resume();
}

void
AmigaComponent::prefix() const
{
    amiga.prefix();
}
