// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Copper.h"
#include "Agnus.h"

void
Copper::serviceEvent(EventID id)
{
    u16 reg;
    Beam beam;

    servicing = true;

    switch (id) {
            
        case COP_REQ_DMA:

            trace(COP_DEBUG && verbose, "COP_REQ_DMA\n");
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Don't wake up in an odd cycle
            if (agnus.pos.h % 2) { reschedule(); break; }

            // Continue with fetching the first instruction word
            schedule(COP_FETCH);
            break;
            
        case COP_WAKEUP:
            
            trace(COP_DEBUG && verbose, "COP_WAKEUP\n");
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }
            
            // Don't wake up in an odd cycle
            if (agnus.pos.h % 2) { reschedule(); break; }
            
            // Continue with fetching the first instruction word
            schedule(COP_FETCH);
            break;
            
        case COP_WAKEUP_BLIT:
            
            trace(COP_DEBUG && verbose, "COP_WAKEUP_BLIT\n");
            
            // Check if the Blitter is busy, keep on waiting
            if (agnus.blitter.isActive()) {
                agnus.scheduleAbs<SLOT_COP>(NEVER, COP_WAIT_BLIT);
                break;
            }
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }
            
            // Don't wake up in an odd cycle
            if (agnus.pos.h % 2) { reschedule(); break; }
            
            // Continue with fetching the first instruction word
            schedule(COP_FETCH);
            break;
            
        case COP_FETCH:

            trace(COP_DEBUG && verbose, "COP_FETCH\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Load the first instruction word
            cop1ins = agnus.doCopperDMA(coppc);
            advancePC();

            if (COP_CHECKSUM) {
                checkcnt++;
                checksum = util::fnv_1a_it32(checksum, cop1ins);
            }

            // Dynamically determine the end of the Copper list
            if (copList == 1) {
                if (coppc > cop1end) cop1end = coppc;
            } else {
                if (coppc > cop2end) cop2end = coppc;
            }

            // Fork execution depending on the instruction type
            schedule(isMoveCmd() ? COP_MOVE : COP_WAIT_OR_SKIP);
            break;
            
        case COP_MOVE:

            trace(COP_DEBUG && verbose, "COP_MOVE\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus.doCopperDMA(coppc);
            advancePC();

            if (COP_CHECKSUM) checksum = util::fnv_1a_it32(checksum, cop2ins);

            // Extract register number from the first instruction word
            reg = (cop1ins & 0x1FE);

            // Stop the Copper if address is illegal
            if (isIllegalAddress(reg)) { agnus.cancel<SLOT_COP>(); break; }

            // Continue with fetching the new command
            schedule(COP_FETCH);

            // Only proceed if the skip flag is not set
            if (skip) { skip = false; break; }

            // Write value into custom register
            switch (reg) {
                case 0x88:
                    schedule(COP_JMP1);
                    agnus.slot[SLOT_COP].data = 1;
                    break;
                case 0x8A:
                    schedule(COP_JMP1);
                    agnus.slot[SLOT_COP].data = 2;
                    break;
                default:
                    move(reg, cop2ins);
            }
            break;
            
        case COP_WAIT_OR_SKIP:

            // debug(COP_DEBUG, "COP_WAIT_OR_SKIP: %X wait %x (%d)\n", coppc, cop1ins, cop1ins);

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Load the second instruction word
            cop2ins = agnus.doCopperDMA(coppc);
            advancePC();

            if (COP_CHECKSUM) checksum = util::fnv_1a_it32(checksum, cop2ins);

            // Fork execution depending on the instruction type
            schedule(isWaitCmd() ? COP_WAIT1 : COP_SKIP1);
            break;

        case COP_WAIT1:
            
            trace(COP_DEBUG, "COP_WAIT1: %X wait %x (%d)\n", coppc, cop1ins, cop1ins);

            trace(COP_DEBUG && verbose, "COP_WAIT1\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Schedule next state
            schedule(COP_WAIT2);
            break;

        case COP_WAIT2:

            // debug(COP_DEBUG, "COP_WAIT2: %X wait %x (%d)\n", coppc, cop1ins, cop1ins);

            trace(COP_DEBUG && verbose, "COP_WAIT2\n");

            // Clear the skip flag
            skip = false;
            
            // Check if we need to wait for the Blitter
            if (!getBFD() && agnus.blitter.isActive()) {
                agnus.scheduleAbs<SLOT_COP>(NEVER, COP_WAIT_BLIT);
                break;
            }
            
            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Test 'coptim3' suggests that cycle $E1 is blocked in this state
            if (agnus.pos.h == 0xE1) { reschedule(); break; }

            // Schedule a wakeup event at the target position
            scheduleWaitWakeup(getBFD());
            break;

        case COP_WAIT_BLIT:
            
            trace(COP_DEBUG, "COP_WAIT_BLIT: %X wait %x (%d)\n", coppc, cop1ins, cop1ins);

            trace(COP_DEBUG && verbose, "COP_WAIT_BLIT\n");
            
            // Wait for the next free cycle
            if (agnus.busOwner[agnus.pos.h] != BUS_NONE &&
                agnus.busOwner[agnus.pos.h] != BUS_BLITTER) {
                // debug("COP_WAIT_BLIT delay\n");
                reschedule(); break;
            }
            
            // Schedule a wakeup event at the target position
            scheduleWaitWakeup(false /* BFD */);
            break;

        case COP_SKIP1:

            trace(COP_DEBUG && verbose, "COP_SKIP1\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Schedule next state
            schedule(COP_SKIP2);
            break;

        case COP_SKIP2:

            trace(COP_DEBUG && verbose, "COP_SKIP2\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            // Test 'coptim3' suggests that cycle $E1 is blocked in this state
            if (agnus.pos.h == 0xE1) { reschedule(); break; }

            // Compute the beam position that needs to be compared
            beam = agnus.addToBeam(agnus.pos, 2);

            // Run the comparator to see if the next command is skipped
            trace(COP_DEBUG && verbose, "Calling comparator(%d,%d)\n", beam.v, beam.h);
            skip = comparator(beam);

            // If the BFD flag is cleared, we also need to check the Blitter
            if (!getBFD()) skip &= !agnus.blitter.isActive();
            
            // Continue with the next command
            schedule(COP_FETCH);
            break;

        case COP_JMP1:

            // debug("COP_JMP1\n");

            // The bus is not needed in this cycle, but still allocated
            (void)agnus.allocateBus<BUS_COPPER>();

            // In cycle $E0, Copper continues with the next state in $E1 (?!)
            if (agnus.pos.h == 0xE0) {
                schedule(COP_JMP2, 1);
                break;
            }

            schedule(COP_JMP2);
            break;

        case COP_JMP2:

            // debug("COP_JMP2\n");

            // Wait for the next possible DMA cycle
            if (!agnus.busIsFree<BUS_COPPER>()) { reschedule(); break; }

            switchToCopperList((isize)agnus.slot[SLOT_COP].data);
            schedule(COP_FETCH);
            break;

        case COP_VBLANK:

            // debug("COP_VBLANK\n");
            // debug("agnus.copdma = %d\n", agnus.copdma());
            
            // Allocate the bus
            // TODO: FIND OUT IF THE BUS IS REALLY ALLOCATED IN THIS STATE
            if (agnus.copdma() && !agnus.allocateBus<BUS_COPPER>()) { reschedule(); break; }

            switchToCopperList(1);
            activeInThisFrame = agnus.copdma();
            schedule(COP_FETCH);

            break;

        default:
            
            assert(false);
            break;
    }

    servicing = false;
}

void
Copper::schedule(EventID next, int delay)
{
    agnus.scheduleRel<SLOT_COP>(DMA_CYCLES(delay), next);
}

void
Copper::reschedule(int delay)
{
    agnus.rescheduleRel<SLOT_COP>(DMA_CYCLES(delay));
}
