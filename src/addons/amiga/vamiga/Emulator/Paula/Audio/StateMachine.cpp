// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "StateMachine.h"
#include "Paula.h"

template <isize nr>
StateMachine<nr>::StateMachine(Amiga& ref) : AmigaComponent(ref)
{
}

template <isize nr> const char *
StateMachine<nr>::getDescription() const
{
    switch (nr) {
        case 0: return "StateMachine 0";
        case 1: return "StateMachine 1";
        case 2: return "StateMachine 2";
        case 3: return "StateMachine 3";
        default: assert(false);
    }
}

template <isize nr> void
StateMachine<nr>::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

template <isize nr> void
StateMachine<nr>::_inspect()
{
    synchronized {
        
        info.state = state;
        info.dma = AUDxON();
        info.audlenLatch = audlenLatch;
        info.audlen = audlen;
        info.audperLatch = audperLatch;
        info.audper = audper;
        info.audvolLatch = audvolLatch;
        info.audvol = audvol;
        info.auddat = auddat;
    }
}

template <isize nr> void
StateMachine<nr>::_dump(Dump::Category category, std::ostream& os) const
{
    os << "   State : " << (int)state;
    os << "  AUDxIP : " << (int)AUDxIP();
    os << "  AUDxON : " << (int)AUDxON();
}

template <isize nr> void
StateMachine<nr>::enableDMA()
{
    trace(AUD_DEBUG, "Enable DMA\n");

    switch (state) {

         case 0b000:

             move_000_001();
             break;
     }
}

template <isize nr> void
StateMachine<nr>::disableDMA()
{
    trace(AUD_DEBUG, "Disable DMA\n");

    switch (state) {

        case 0b001:

            move_001_000();
            break;

        case 0b101:

            move_101_000();
            break;
    }
}

template <isize nr> bool
StateMachine<nr>::AUDxON() const
{
    return agnus.auddma<nr>();
}

template <isize nr> bool
StateMachine<nr>::AUDxIP() const 
{
    return GET_BIT(paula.intreq, 7 + nr);
}

template <isize nr> void
StateMachine<nr>::AUDxIR()
{
    if (DISABLE_AUDIRQ) return;
    
    IrqSource source =
    nr == 0 ? INT_AUD0 : nr == 1 ? INT_AUD1 : nr == 2 ? INT_AUD2 : INT_AUD3;
    
    paula.scheduleIrqRel(source, DMA_CYCLES(1));
}

template <isize nr> void
StateMachine<nr>::percntrld()
{
    const EventSlot slot = (EventSlot)(SLOT_CH0+nr);

    
    u64 delay = (audperLatch == 0) ? 0x10000 : audperLatch;

    agnus.scheduleRel<slot>(DMA_CYCLES(delay), CHX_PERFIN);
}

template <isize nr> void
StateMachine<nr>::pbufld1()
{
    if (!AUDxAV()) { buffer = auddat; return; }
    
    // trace("Volume modulation %d (%d)\n", auddat & 0x7F, (i16)auddat);
    switch (nr) {
        case 0: paula.channel1.pokeAUDxVOL(auddat); break;
        case 1: paula.channel2.pokeAUDxVOL(auddat); break;
        case 2: paula.channel3.pokeAUDxVOL(auddat); break;
        case 3: break;
    }
}

template <isize nr> void
StateMachine<nr>::pbufld2()
{
    assert(AUDxAP());
    
    switch (nr) {
        case 0: paula.channel1.pokeAUDxPER(auddat); break;
        case 1: paula.channel2.pokeAUDxPER(auddat); break;
        case 2: paula.channel3.pokeAUDxPER(auddat); break;
        case 3: break;
    }
}

template <isize nr> bool
StateMachine<nr>::AUDxAV() const
{
    return (paula.adkcon >> nr) & 0x01;
}

template <isize nr> bool
StateMachine<nr>::AUDxAP() const
{
    return (paula.adkcon >> nr) & 0x10;
}

template <isize nr> void
StateMachine<nr>::penhi()
{
    if (!enablePenhi) return;
 
    Sampler *sampler = paula.muxer.sampler[nr];

    i8 sample = (i8)HI_BYTE(buffer);
    i16 scaled = sample * audvol;
    
    trace(AUD_DEBUG, "penhi: %d %d\n", sample, scaled);
                
    if (!sampler->isFull()) {
        sampler->write( TaggedSample { agnus.clock, scaled } );
    } else {
        warn("penhi: Sample buffer is full\n");
    }
    
    enablePenhi = false;
}

template <isize nr> void
StateMachine<nr>::penlo()
{
    if (!enablePenlo) return;

    Sampler *sampler = paula.muxer.sampler[nr];
    
    i8 sample = (i8)LO_BYTE(buffer);
    i16 scaled = sample * audvol;

    trace(AUD_DEBUG, "penlo: %d %d\n", sample, scaled);

    if (!sampler->isFull()) {
        sampler->write( TaggedSample { agnus.clock, scaled } );
    } else {
        warn("penlo: Sample buffer is full\n");
    }
    
    enablePenlo = false;
}

template <isize nr> void
StateMachine<nr>::move_000_010() {

    trace(AUD_DEBUG, "move_000_010\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());
    assert(!AUDxIP());

    volcntrld();
    percntrld();
    pbufld1();
    AUDxIR();

    state = 0b010;
    penhi();
}

template <isize nr> void
StateMachine<nr>::move_000_001() {

    trace(AUD_DEBUG, "move_000_001\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    lencntrld();
    AUDxDR();

    state = 0b001;
}

template <isize nr> void
StateMachine<nr>::move_001_000() {

    trace(AUD_DEBUG, "move_001_000\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());

    state = 0b000;
}

template <isize nr> void
StateMachine<nr>::move_001_101() {

    trace(AUD_DEBUG, "move_001_101\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    AUDxIR();
    AUDxDR();
    AUDxDSR();
    if (!lenfin()) lencount();

    state = 0b101;
}

template <isize nr> void
StateMachine<nr>::move_101_000() {

    trace(AUD_DEBUG, "move_101_000\n");

    // This transition is taken in IRQ mode only
    assert(!AUDxON());

    state = 0b000;
}

template <isize nr> void
StateMachine<nr>::move_101_010() {

    trace(AUD_DEBUG, "move_101_010\n");

    // This transition is taken in DMA mode only
    assert(AUDxON());

    percntrld();
    volcntrld();
    pbufld1();
    if (napnav()) AUDxDR();

    state = 0b010;
    penhi();
}

template <isize nr> void
StateMachine<nr>::move_010_011() {

    trace(AUD_DEBUG, "move_010_011\n");
    
    percntrld();
    
    // Check for attach period mode
    if (AUDxAP()) {
        
        pbufld2();
        
        if (AUDxON()) {
            
            // Additional DMA mode action
            AUDxDR();
            if (intreq2) { AUDxIR(); intreq2 = false; }
            
        } else {
            
            // Additional IRQ mode action
            AUDxIR();
        }
    }

    state = 0b011;
    penlo();
}

template <isize nr> void
StateMachine<nr>::move_011_000() {

    trace(AUD_DEBUG, "move_011_000\n");

    const EventSlot slot = (EventSlot)(SLOT_CH0+nr);
    agnus.cancel<slot>();

    intreq2 = false;
    state = 0b000;
}

template <isize nr> void
StateMachine<nr>::move_011_010()
{
    trace(AUD_DEBUG, "move_011_010\n");

    percntrld();
    pbufld1();
    volcntrld();
    
    if (napnav()) {

        if (AUDxON()) {

            // Additional DMA mode action
            AUDxDR();
            if (intreq2) { AUDxIR(); intreq2 = false; }

        } else {

            // Additional IRQ mode action
            AUDxIR();
        }
        
        // intreq2 = false;
    }

    state = 0b010;
    penhi();
}

template StateMachine<0>::StateMachine(Amiga &ref);
template StateMachine<1>::StateMachine(Amiga &ref);
template StateMachine<2>::StateMachine(Amiga &ref);
template StateMachine<3>::StateMachine(Amiga &ref);

template AudioChannelInfo StateMachine<0>::getInfo();
template AudioChannelInfo StateMachine<1>::getInfo();
template AudioChannelInfo StateMachine<2>::getInfo();
template AudioChannelInfo StateMachine<3>::getInfo();

template void StateMachine<0>::enableDMA();
template void StateMachine<1>::enableDMA();
template void StateMachine<2>::enableDMA();
template void StateMachine<3>::enableDMA();

template void StateMachine<0>::disableDMA();
template void StateMachine<1>::disableDMA();
template void StateMachine<2>::disableDMA();
template void StateMachine<3>::disableDMA();

template bool StateMachine<0>::AUDxIP() const;
template bool StateMachine<1>::AUDxIP() const;
template bool StateMachine<2>::AUDxIP() const;
template bool StateMachine<3>::AUDxIP() const;

template bool StateMachine<0>::AUDxON() const;
template bool StateMachine<1>::AUDxON() const;
template bool StateMachine<2>::AUDxON() const;
template bool StateMachine<3>::AUDxON() const;

template void StateMachine<0>::move_000_010();
template void StateMachine<1>::move_000_010();
template void StateMachine<2>::move_000_010();
template void StateMachine<3>::move_000_010();

template void StateMachine<0>::move_000_001();
template void StateMachine<1>::move_000_001();
template void StateMachine<2>::move_000_001();
template void StateMachine<3>::move_000_001();

template void StateMachine<0>::move_001_101();
template void StateMachine<1>::move_001_101();
template void StateMachine<2>::move_001_101();
template void StateMachine<3>::move_001_101();

template void StateMachine<0>::move_010_011();
template void StateMachine<1>::move_010_011();
template void StateMachine<2>::move_010_011();
template void StateMachine<3>::move_010_011();

template void StateMachine<0>::move_101_010();
template void StateMachine<1>::move_101_010();
template void StateMachine<2>::move_101_010();
template void StateMachine<3>::move_101_010();

template void StateMachine<0>::move_011_000();
template void StateMachine<1>::move_011_000();
template void StateMachine<2>::move_011_000();
template void StateMachine<3>::move_011_000();

template void StateMachine<0>::move_011_010();
template void StateMachine<1>::move_011_010();
template void StateMachine<2>::move_011_010();
template void StateMachine<3>::move_011_010();
