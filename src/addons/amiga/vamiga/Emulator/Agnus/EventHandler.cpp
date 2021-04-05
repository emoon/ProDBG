// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
Agnus::inspectEvents()
{
    synchronized {
        
        eventInfo.cpuClock = cpu.getMasterClock();
        eventInfo.cpuCycles = cpu.getCpuClock();
        eventInfo.dmaClock = clock;
        eventInfo.ciaAClock = ciaa.clock;
        eventInfo.ciaBClock  = ciab.clock;
        eventInfo.frame = frame.nr;
        eventInfo.vpos = pos.v;
        eventInfo.hpos = pos.h;
        
        // Inspect all slots
        for (int i = 0; i < SLOT_COUNT; i++) inspectEventSlot((EventSlot)i);
    }
}

void
Agnus::inspectEventSlot(EventSlot nr)
{
    assert(isEventSlot(nr));
    
    EventSlotInfo *i = &eventInfo.slotInfo[nr];
    Cycle trigger = slot[nr].triggerCycle;

    i->slotName = slotName((EventSlot)nr);
    i->eventId = slot[nr].id;
    i->trigger = trigger;
    i->triggerRel = trigger - clock;

    if (belongsToCurrentFrame(trigger)) {
        Beam beam = cycleToBeam(trigger);
        i->vpos = beam.v;
        i->hpos = beam.h;
        i->frameRel = 0;
    } else if (belongsToNextFrame(trigger)) {
        i->vpos = 0;
        i->hpos = 0;
        i->frameRel = 1;
    } else {
        assert(belongsToPreviousFrame(trigger));
        i->vpos = 0;
        i->hpos = 0;
        i->frameRel = -1;
    }

    switch ((EventSlot)nr) {

        case REG_SLOT:
            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case REG_CHANGE:    i->eventName = "REG_CHANGE"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case RAS_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case RAS_HSYNC:     i->eventName = "RAS_HSYNC"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case CIAA_SLOT:
        case CIAB_SLOT:

            switch (slot[nr].id) {
                case 0:             i->eventName = "none"; break;
                case CIA_EXECUTE:   i->eventName = "CIA_EXECUTE"; break;
                case CIA_WAKEUP:    i->eventName = "CIA_WAKEUP"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case BPL_SLOT:

            switch ((int)slot[nr].id) {
                case 0:                              i->eventName = "none"; break;
                case DRAW_ODD:                       i->eventName = "BPL [O]"; break;
                case DRAW_EVEN:                      i->eventName = "BPL [E]"; break;
                case DRAW_ODD | DRAW_EVEN:           i->eventName = "BPL [OE]"; break;
                case BPL_L1:                         i->eventName = "BPL_L1"; break;
                case BPL_L1 | DRAW_ODD:              i->eventName = "BPL_L1 [O]"; break;
                case BPL_L1 | DRAW_EVEN:             i->eventName = "BPL_L1 [E]"; break;
                case BPL_L1 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_L1 [OE]"; break;
                case BPL_L2:                         i->eventName = "BPL_L2"; break;
                case BPL_L2 | DRAW_ODD:              i->eventName = "BPL_L2 [O]"; break;
                case BPL_L2 | DRAW_EVEN:             i->eventName = "BPL_L2 [E]"; break;
                case BPL_L2 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_L2 [OE]"; break;
                case BPL_L3:                         i->eventName = "BPL_L3"; break;
                case BPL_L3 | DRAW_ODD:              i->eventName = "BPL_L3 [O]"; break;
                case BPL_L3 | DRAW_EVEN:             i->eventName = "BPL_L3 [E]"; break;
                case BPL_L3 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_L3 [OE]"; break;
                case BPL_L4:                         i->eventName = "BPL_L4"; break;
                case BPL_L4 | DRAW_ODD:              i->eventName = "BPL_L4 [O]"; break;
                case BPL_L4 | DRAW_EVEN:             i->eventName = "BPL_L4 [E]"; break;
                case BPL_L4 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_L4 [OE]"; break;
                case BPL_L5:                         i->eventName = "BPL_L5"; break;
                case BPL_L5 | DRAW_ODD:              i->eventName = "BPL_L5 [O]"; break;
                case BPL_L5 | DRAW_EVEN:             i->eventName = "BPL_L5 [E]"; break;
                case BPL_L5 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_L5 [OE]"; break;
                case BPL_L6:                         i->eventName = "BPL_L6"; break;
                case BPL_L6 | DRAW_ODD:              i->eventName = "BPL_L6 [O]"; break;
                case BPL_L6 | DRAW_EVEN:             i->eventName = "BPL_L6 [E]"; break;
                case BPL_L6 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_L6 [OE]"; break;
                case BPL_H1:                         i->eventName = "BPL_H1"; break;
                case BPL_H1 | DRAW_ODD:              i->eventName = "BPL_H1 [O]"; break;
                case BPL_H1 | DRAW_EVEN:             i->eventName = "BPL_H1 [E]"; break;
                case BPL_H1 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_H1 [OE]"; break;
                case BPL_H2:                         i->eventName = "BPL_H2"; break;
                case BPL_H2 | DRAW_ODD:              i->eventName = "BPL_H2 [O]"; break;
                case BPL_H2 | DRAW_EVEN:             i->eventName = "BPL_H2 [E]"; break;
                case BPL_H2 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_H2 [OE]"; break;
                case BPL_H3:                         i->eventName = "BPL_H3"; break;
                case BPL_H3 | DRAW_ODD:              i->eventName = "BPL_H3 [O]"; break;
                case BPL_H3 | DRAW_EVEN:             i->eventName = "BPL_H3 [E]"; break;
                case BPL_H3 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_H3 [OE]"; break;
                case BPL_H4:                         i->eventName = "BPL_H4"; break;
                case BPL_H4 | DRAW_ODD:              i->eventName = "BPL_H4 [O]"; break;
                case BPL_H4 | DRAW_EVEN:             i->eventName = "BPL_H4 [E]"; break;
                case BPL_H4 | DRAW_ODD | DRAW_EVEN:  i->eventName = "BPL_H4 [OE]"; break;
                case BPL_EOL:                        i->eventName = "BPL_EOL"; break;
                case BPL_EOL | DRAW_ODD:             i->eventName = "BPL_EOL [O]"; break;
                case BPL_EOL | DRAW_EVEN:            i->eventName = "BPL_EOL [E]"; break;
                case BPL_EOL | DRAW_ODD | DRAW_EVEN: i->eventName = "BPL_EOL [OE]"; break;
                default:                i->eventName = "*** INVALID ***"; break;
            }
            break;

        case DAS_SLOT:

            switch (slot[nr].id) {
                case 0:             i->eventName = "none"; break;
                case DAS_REFRESH:   i->eventName = "DAS_REFRESH"; break;
                case DAS_D0:        i->eventName = "DAS_D0"; break;
                case DAS_D1:        i->eventName = "DAS_D1"; break;
                case DAS_D2:        i->eventName = "DAS_D2"; break;
                case DAS_A0:        i->eventName = "DAS_A0"; break;
                case DAS_A1:        i->eventName = "DAS_A1"; break;
                case DAS_A2:        i->eventName = "DAS_A2"; break;
                case DAS_A3:        i->eventName = "DAS_A3"; break;
                case DAS_S0_1:      i->eventName = "DAS_S0_1"; break;
                case DAS_S0_2:      i->eventName = "DAS_S0_2"; break;
                case DAS_S1_1:      i->eventName = "DAS_S1_1"; break;
                case DAS_S1_2:      i->eventName = "DAS_S1_2"; break;
                case DAS_S2_1:      i->eventName = "DAS_S2_2"; break;
                case DAS_S3_1:      i->eventName = "DAS_S3_1"; break;
                case DAS_S3_2:      i->eventName = "DAS_S3_2"; break;
                case DAS_S4_1:      i->eventName = "DAS_S4_1"; break;
                case DAS_S4_2:      i->eventName = "DAS_S4_2"; break;
                case DAS_S5_1:      i->eventName = "DAS_S5_1"; break;
                case DAS_S5_2:      i->eventName = "DAS_S5_2"; break;
                case DAS_S6_1:      i->eventName = "DAS_S6_1"; break;
                case DAS_S6_2:      i->eventName = "DAS_S6_2"; break;
                case DAS_S7_1:      i->eventName = "DAS_S7_1"; break;
                case DAS_S7_2:      i->eventName = "DAS_S7_2"; break;
                case DAS_SDMA:      i->eventName = "DAS_SDMA"; break;
                case DAS_TICK:      i->eventName = "DAS_TICK"; break;
                case DAS_TICK2:     i->eventName = "DAS_TICK2"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case COP_SLOT:

            switch (slot[nr].id) {

                case 0:                i->eventName = "none"; break;
                case COP_REQ_DMA:      i->eventName = "COP_REQ_DMA"; break;
                case COP_WAKEUP:       i->eventName = "COP_WAKEUP"; break;
                case COP_WAKEUP_BLIT:  i->eventName = "COP_WAKEUP_BLIT"; break;
                case COP_FETCH:        i->eventName = "COP_FETCH"; break;
                case COP_MOVE:         i->eventName = "COP_MOVE"; break;
                case COP_WAIT_OR_SKIP: i->eventName = "WAIT_OR_SKIP"; break;
                case COP_WAIT1:        i->eventName = "COP_WAIT1"; break;
                case COP_WAIT2:        i->eventName = "COP_WAIT2"; break;
                case COP_WAIT_BLIT:    i->eventName = "COP_WAIT_BLIT"; break;
                case COP_SKIP1:        i->eventName = "COP_SKIP1"; break;
                case COP_SKIP2:        i->eventName = "COP_SKIP1"; break;
                case COP_JMP1:         i->eventName = "COP_JMP1"; break;
                case COP_JMP2:         i->eventName = "COP_JMP2"; break;
                case COP_VBLANK:       i->eventName = "COP_VBLANK"; break;
                default:               i->eventName = "*** INVALID ***"; break;
            }
            break;

        case BLT_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case BLT_STRT1:     i->eventName = "BLT_STRT1"; break;
                case BLT_STRT2:     i->eventName = "BLT_STRT2"; break;
                case BLT_COPY_SLOW: i->eventName = "BLT_COPY_SLOW"; break;
                case BLT_COPY_FAKE: i->eventName = "BLT_COPY_FAKE"; break;
                case BLT_LINE_FAKE: i->eventName = "BLT_LINE_FAKE"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case SEC_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case SEC_TRIGGER:   i->eventName = "SEC_TRIGGER"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case CH0_SLOT:
        case CH1_SLOT:
        case CH2_SLOT:
        case CH3_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case CHX_PERFIN:    i->eventName = "CHX_PERFIN"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case DSK_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case DSK_ROTATE:    i->eventName = "DSK_ROTATE"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case DCH_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case DCH_INSERT:    i->eventName = "DCH_INSERT"; break;
                case DCH_EJECT:     i->eventName = "DCH_EJECT"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case VBL_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case VBL_STROBE0:   i->eventName = "VBL_STROBE0"; break;
                case VBL_STROBE1:   i->eventName = "VBL_STROBE1"; break;
                case VBL_END:       i->eventName = "VBL_END"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case IRQ_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case IRQ_CHECK:     i->eventName = "IRQ_CHECK"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case IPL_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case IPL_CHANGE:    i->eventName = "IPL_CHANGE"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case KBD_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case KBD_TIMEOUT:   i->eventName = "KBD_TIMEOUT"; break;
                case KBD_DAT:       i->eventName = "KBD_DAT"; break;
                case KBD_CLK0:      i->eventName = "KBD_CLK0"; break;
                case KBD_CLK1:      i->eventName = "KBD_CLK1"; break;
                case KBD_SYNC_DAT0: i->eventName = "KBD_SYNC_DAT0"; break;
                case KBD_SYNC_CLK0: i->eventName = "KBD_SYNC_CLK0"; break;
                case KBD_SYNC_DAT1: i->eventName = "KBD_SYNC_DAT1"; break;
                case KBD_SYNC_CLK1: i->eventName = "KBD_SYNC_CLK1"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case TXD_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case TXD_BIT:       i->eventName = "TXD_BIT"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case RXD_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case RXD_BIT:       i->eventName = "RXD_BIT"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        case POT_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case POT_DISCHARGE: i->eventName = "POT_DISCHARGE"; break;
                case POT_CHARGE:    i->eventName = "POT_CHARGE"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;
            
        case INS_SLOT:

            switch (slot[nr].id) {

                case 0:             i->eventName = "none"; break;
                case INS_NONE:      i->eventName = "INS_NONE"; break;
                case INS_AMIGA:     i->eventName = "INS_AMIGA"; break;
                case INS_CPU:       i->eventName = "INS_CPU"; break;
                case INS_MEM:       i->eventName = "INS_MEM"; break;
                case INS_CIA:       i->eventName = "INS_CIA"; break;
                case INS_AGNUS:     i->eventName = "INS_AGNUS"; break;
                case INS_PAULA:     i->eventName = "INS_PAULA"; break;
                case INS_DENISE:    i->eventName = "INS_DENISE"; break;
                case INS_PORTS:     i->eventName = "INS_PORTS"; break;
                case INS_EVENTS:    i->eventName = "INS_EVENTS"; break;
                default:            i->eventName = "*** INVALID ***"; break;
            }
            break;

        default: assert(false);
    }
}

void
Agnus::dumpEvents()
{
    inspectEvents();
        
    msg("Events:\n");
    for (unsigned i = 0; i < SLOT_COUNT; i++) {
        
        msg("Slot: %-17s ", eventInfo.slotInfo[i].slotName);
        msg("Event: %-15s ", eventInfo.slotInfo[i].eventName);
        msg("Trigger: ");
        
        Cycle trigger = eventInfo.slotInfo[i].trigger;
        if (trigger == NEVER) {
            msg("never\n");
        } else {
            msg("%lld ", trigger);
            msg("(%lld DMA cycles away)\n", AS_DMA_CYCLES(trigger - eventInfo.dmaClock));
        }
    }
}

EventInfo
Agnus::getEventInfo()
{
    EventInfo result;
    synchronized { result = eventInfo; }
    return result;
}

EventSlotInfo
Agnus::getEventSlotInfo(int nr)
{
    assert(isEventSlot(nr));

    EventSlotInfo result;
    synchronized { result = eventInfo.slotInfo[nr]; }
    return result;
}

void
Agnus::scheduleNextBplEvent(i16 hpos)
{
    assert(isHPos(hpos));

    if (u8 next = nextBplEvent[hpos]) {
        scheduleRel<BPL_SLOT>(DMA_CYCLES(next - pos.h), bplEvent[next]);
    }
    assert(hasEvent<BPL_SLOT>());
}

void
Agnus::scheduleBplEventForCycle(i16 hpos)
{
    assert(isHPos(hpos));
    assert(hpos >= pos.h);

    if (bplEvent[hpos] != EVENT_NONE) {
        scheduleRel<BPL_SLOT>(DMA_CYCLES(hpos - pos.h), bplEvent[hpos]);
    } else {
        scheduleNextBplEvent(hpos);
    }

    assert(hasEvent<BPL_SLOT>());
}

void
Agnus::scheduleNextDasEvent(i16 hpos)
{
    assert(isHPos(hpos));

    if (u8 next = nextDasEvent[hpos]) {
        scheduleRel<DAS_SLOT>(DMA_CYCLES(next - pos.h), dasEvent[next]);
        assert(hasEvent<DAS_SLOT>());
    } else {
        cancel<DAS_SLOT>();
    }
}

void
Agnus::scheduleDasEventForCycle(i16 hpos)
{
    assert(isHPos(hpos));
    assert(hpos >= pos.h);

    if (dasEvent[hpos] != EVENT_NONE) {
        scheduleRel<DAS_SLOT>(DMA_CYCLES(hpos - pos.h), dasEvent[hpos]);
    } else {
        scheduleNextDasEvent(hpos);
    }
}

void
Agnus::scheduleNextREGEvent()
{
    // Determine when the next register change happens
    Cycle nextTrigger = changeRecorder.trigger();

    // Schedule a register change event for that cycle
    scheduleAbs<REG_SLOT>(nextTrigger, REG_CHANGE);
}

void
Agnus::executeEventsUntil(Cycle cycle) {

    //
    // Check primary slots
    //

    if (isDue<RAS_SLOT>(cycle)) {
        serviceRASEvent();
    }
    if (isDue<REG_SLOT>(cycle)) {
        serviceREGEvent(cycle);
    }
    if (isDue<CIAA_SLOT>(cycle)) {
        serviceCIAEvent<0>();
    }
    if (isDue<CIAB_SLOT>(cycle)) {
        serviceCIAEvent<1>();
    }
    if (isDue<BPL_SLOT>(cycle)) {
        serviceBPLEvent();
    }
    if (isDue<DAS_SLOT>(cycle)) {
        serviceDASEvent();
    }
    if (isDue<COP_SLOT>(cycle)) {
        copper.serviceEvent(slot[COP_SLOT].id);
    }
    if (isDue<BLT_SLOT>(cycle)) {
        blitter.serviceEvent(slot[BLT_SLOT].id);
    }

    if (isDue<SEC_SLOT>(cycle)) {

        //
        // Check secondary slots
        //

        if (isDue<CH0_SLOT>(cycle)) {
            paula.channel0.serviceEvent();
        }
        if (isDue<CH1_SLOT>(cycle)) {
            paula.channel1.serviceEvent();
        }
        if (isDue<CH2_SLOT>(cycle)) {
            paula.channel2.serviceEvent();
        }
        if (isDue<CH3_SLOT>(cycle)) {
            paula.channel3.serviceEvent();
        }
        if (isDue<DSK_SLOT>(cycle)) {
            paula.diskController.serviceDiskEvent();
        }
        if (isDue<DCH_SLOT>(cycle)) {
            paula.diskController.serviceDiskChangeEvent();
        }
        if (isDue<VBL_SLOT>(cycle)) {
            serviceVblEvent();
        }
        if (isDue<IRQ_SLOT>(cycle)) {
            paula.serviceIrqEvent();
        }
        if (isDue<IPL_SLOT>(cycle)) {
            paula.serviceIplEvent();
        }
        if (isDue<KBD_SLOT>(cycle)) {
            amiga.keyboard.serviceKeyboardEvent(slot[KBD_SLOT].id);
        }
        if (isDue<TXD_SLOT>(cycle)) {
            uart.serviceTxdEvent(slot[TXD_SLOT].id);
        }
        if (isDue<RXD_SLOT>(cycle)) {
            uart.serviceRxdEvent(slot[RXD_SLOT].id);
        }
        if (isDue<POT_SLOT>(cycle)) {
            paula.servicePotEvent(slot[POT_SLOT].id);
        }
        if (isDue<INS_SLOT>(cycle)) {
            serviceINSEvent();
        }

        // Determine the next trigger cycle for all secondary slots
        Cycle nextSecTrigger = slot[SEC_SLOT + 1].triggerCycle;
        for (unsigned i = SEC_SLOT + 2; i < SLOT_COUNT; i++)
            if (slot[i].triggerCycle < nextSecTrigger)
                nextSecTrigger = slot[i].triggerCycle;

        // Update the secondary table trigger in the primary table
        rescheduleAbs<SEC_SLOT>(nextSecTrigger);
    }

    // Determine the next trigger cycle for all primary slots
    nextTrigger = slot[0].triggerCycle;
    for (unsigned i = 1; i <= SEC_SLOT; i++)
        if (slot[i].triggerCycle < nextTrigger)
            nextTrigger = slot[i].triggerCycle;
}
