// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

u8
CIA::peek(u16 addr)
{
    u8 result;

    trace(CIAREG_DEBUG, "Peek(%d [%s])\n", addr, ciaRegName(addr));

    wakeUp();

    assert(addr <= 0x000F);
    switch(addr) {
            
        case 0x00: // CIA_DATA_PORT_A

            updatePA();
            result = PA;
            break;

        case 0x01: // CIA_DATA_PORT_B

            updatePB();
            result = PB;
            break;

        case 0x02: // CIA_DATA_DIRECTION_A

            result = DDRA;
            // debug(DSKREG_DEBUG, "read DDRA = %X\n", DDRA);
            break;

        case 0x03: // CIA_DATA_DIRECTION_B

            result = DDRB;
            // debug(DSKREG_DEBUG, "read DDRB = %X\n", DDRB);
            break;
            
        case 0x04: // CIA_TIMER_A_LOW
            
            result = LO_BYTE(counterA);
            break;
            
        case 0x05: // CIA_TIMER_A_HIGH
            result = HI_BYTE(counterA);
            break;
            
        case 0x06: // CIA_TIMER_B_LOW

            result = LO_BYTE(counterB);
            break;
            
        case 0x07: // CIA_TIMER_B_HIGH

            result = HI_BYTE(counterB);
            break;
            
        case 0x08: // EVENT_0_7

            result = tod.getCounterLo(clock - DMA_CYCLES(isCIAA() ? 95 : 210));
            tod.defreeze();
            break;
        
        case 0x09: // EVENT_8_15

            result = tod.getCounterMid(clock - DMA_CYCLES(isCIAA() ? 95 : 210));
            break;
            
        case 0x0A: // EVENT_16_23

            if (!(CRB & 0x80)) tod.freeze();
            result = tod.getCounterHi(clock - DMA_CYCLES(isCIAA() ? 95 : 210));
            break;
            
        case 0x0B: // UNUSED

            result = 0xFF;
            break;
            
        case 0x0C: // CIA_SERIAL_DATA_REGISTER
            
            result = sdr;
            break;
            
        case 0x0D: // CIA_INTERRUPT_CONTROL

            // Set upper bit if an IRQ is being triggered
            if ((delay & CIASetInt1) && (icr & 0x1F)) {
                icr |= 0x80;
            }
            
            // Remember result
            result = icr;
            
            // Release interrupt request
            if (INT == 0) {
                delay |= CIAClearInt0;
            }
            
            // Discard pending interrupts
            delay &= ~(CIASetInt0 | CIASetInt1);
        
            // Schedule the ICR bits to be cleared
            delay |= CIAClearIcr0; // Uppermost bit
            delay |= CIAAckIcr0;   // Other bits
            icrAck = result;

            // Remember the read access
            delay |= CIAReadIcr0;
            break;

        case 0x0E: // CIA_CONTROL_REG_A

            result = (u8)(CRA & ~0x90); // Bit 4 and 7 always read as 0
            break;
            
        case 0x0F: // CIA_CONTROL_REG_B
            
            result = (u8)(CRB & ~0x10); // Bit 4 always reads as 0
            break;
            
        default:
            assert(false);
            result = 0;
            break;
    }
    
    // debug("Peek %d (hex: %02X) = %d\n", addr, addr, result);
    
    return result;
}

u8
CIA::spypeek(u16 addr)
{
    return const_cast<const CIA*>(this)->spypeek(addr);
}

u8
CIA::spypeek(u16 addr) const
{
    bool running;

    assert(addr <= 0x000F);
    switch(addr) {
          
        case 0x00: // CIA_DATA_PORT_A
            return PA;
            
        case 0x01: // CIA_DATA_PORT_B
            return PB;
            
        case 0x02: // CIA_DATA_DIRECTION_A
            return DDRA;
            
        case 0x03: // CIA_DATA_DIRECTION_B
            return DDRB;
            
        case 0x04: // CIA_TIMER_A_LOW
            running = delay & CIACountA3;
            return LO_BYTE(counterA - (running ? (u16)idleSince() : 0));
            
        case 0x05: // CIA_TIMER_A_HIGH
            running = delay & CIACountA3;
            return HI_BYTE(counterA - (running ? (u16)idleSince() : 0));
            
        case 0x06: // CIA_TIMER_B_LOW
            running = delay & CIACountB3;
            return LO_BYTE(counterB - (running ? (u16)idleSince() : 0));
            
        case 0x07: // CIA_TIMER_B_HIGH
            running = delay & CIACountB3;
            return HI_BYTE(counterB - (running ? (u16)idleSince() : 0));
            
        case 0x08: // CIA_EVENT_0_7
            return tod.getCounterLo();
            
        case 0x09: // CIA_EVENT_8_15
            return tod.getCounterMid();
            
        case 0x0A: // CIA_EVENT_16_23
            return tod.getCounterHi();
            
        case 0x0B: // UNUSED
            return 0;
            
        case 0x0C: // CIA_SERIAL_DATA_REGISTER
            return sdr;
            
        case 0x0D: // CIA_INTERRUPT_CONTROL
            return icr;
            
        case 0x0E: // CIA_CONTROL_REG_A
            return CRA & ~0x10;
            
        case 0x0F: // CIA_CONTROL_REG_B
            return CRB & ~0x10;
            
        default:
            assert(false);
            return 0;
    }
}

void
CIA::poke(u16 addr, u8 value)
{
    trace(CIAREG_DEBUG, "Poke(%d [%s],$%X)\n", addr, ciaRegName(addr), value);
    
    wakeUp();
    
    switch(addr) {
        
        case 0x00: // CIA_DATA_PORT_A

            // debug("%s poke(0, %X)\n", getDescription(), value);
            pokePA(value);
            return;
            
        case 0x01: // CIA_DATA_PORT_B
            
            // if (isCIAB()) debug("poke(1, %X)\n", value);
            PRB = value;
            updatePB();
            return;
            
        case 0x02: // CIA_DATA_DIRECTION_A
        
            // debug("%s poke(DDRA, %X)\n", getDescription(), value);
            if ((isCIAA() && value != 0x03) || (isCIAB() && value != 0xC0)) {
                trace(XFILES, "XFILES (DDRA) Setting unusual value %x\n", value);
            }
            pokeDDRA(value);
            return;
            
        case 0x03: // CIA_DATA_DIRECTION_B
        
            // debug("%s poke(DDRB, %X)\n", getDescription(), value);
            if (isCIAB() && value != 0xFF) {
                trace(XFILES, "XFILES (DDRB) Setting unusual value %x\n", value);
            }
            DDRB = value;
            updatePB();
            return;
            
        case 0x04: // CIA_TIMER_A_LOW
            
            latchA = (latchA & 0xFF00) | value;
            if (delay & CIALoadA2) {
                counterA = (counterA & 0xFF00) | value;
            }
            return;
            
        case 0x05: // CIA_TIMER_A_HIGH
            
            latchA = (latchA & 0x00FF) | (value << 8);
            if (delay & CIALoadA2) {
                counterA = (counterA & 0x00FF) | (value << 8);
            }
            
            // Load counter if timer is stopped
            if (!(CRA & 0x01)) {
                delay |= CIALoadA0;
            }
            
            /* MOS 8520 only feature:
             * "In one-shot mode, a write to timer-high (register 5 for timer A,
             *  register 7 for Timer B) will transfer the timer latch to the
             *  counter and initiate counting regardless of the start bit." [HRM]
             */
            if (CRA & 0x08) {
                if (!(CRA & 0x01)) {
                    PB67Toggle |= 0x40;
                }
                if (!(CRA & 0x20)) {
                    delay |= CIACountA1 | CIALoadA0 | CIACountA0;
                    feed |= CIACountA0;
                }
                CRA |= 0x01;
            }
            
            return;
            
        case 0x06: // CIA_TIMER_B_LOW

            latchB = (latchB & 0xFF00) | value;
            if (delay & CIALoadB2) {
                counterB = (counterB & 0xFF00) | value;
            }
            return;
            
        case 0x07: // CIA_TIMER_B_HIGH
            
            // debug("CIA7: %x\n", value);
            latchB = (latchB & 0x00FF) | (value << 8);
            if (delay & CIALoadB2) {
                counterB = (counterB & 0x00FF) | (value << 8);
            }
            
            // Load counter if timer is stopped
            if ((CRB & 0x01) == 0) {
                delay |= CIALoadB0;
            }
            
            /* MOS 8520 only feature:
             * "In one-shot mode, a write to timer-high (register 5 for timer A,
             *  register 7 for Timer B) will transfer the timer latch to the
             *  counter and initiate counting regardless of the start bit." [HRM]
             */
            if (CRB & 0x08) {
                if (!(CRB & 0x01)) {
                    PB67Toggle |= 0x80;
                }
                if (!(CRB & 0x60)) {
                    delay |= CIACountB1 | CIALoadB0 | CIACountB0;
                    feed |= CIACountB0;
                }
                CRB |= 0x01;
            }
            
            return;
            
        case 0x08: // CIA_EVENT_0_7
            
            if (CRB & 0x80) {
                tod.setAlarmLo(value);
            } else {
                tod.setCounterLo(value);
                tod.cont();
            }
            return;
            
        case 0x09: // CIA_EVENT_8_15
            
            if (CRB & 0x80) {
                tod.setAlarmMid(value);
            } else {
                tod.setCounterMid(value);
            }
            return;
            
        case 0x0A: // CIA_EVENT_16_23
            
            if (CRB & 0x80) {
                tod.setAlarmHi(value);
            } else {
                tod.setCounterHi(value);
                tod.stop();
            }
            return;
            
        case 0x0B: // UNUSED
        
            return;
            
        case 0x0C: // CIA_DATA_REGISTER
            
            sdr = value;
            delay |= CIASdrToSsr0;
            feed |= CIASdrToSsr0;
            return;
            
        case 0x0D: // CIA_INTERRUPT_CONTROL
            
            // Bit 7 means set (1) or clear (0) the other bits
            if ((value & 0x80) != 0) {
                imr |= (value & 0x1F);
            } else {
                imr &= ~(value & 0x1F);
            }
            // debug("imr = %d (hex: %X) icr = %d (hex: %X) INT = %d\n", imr, imr, icr, icr, INT);
            
            // Raise an interrupt in the next cycle if conditions match
            if ((imr & icr & 0x1F) && INT && !(delay & CIAReadIcr1)) {
                delay |= (CIASetInt1 | CIASetIcr1);
            }
            return;
            
        case 0x0E: // CIA_CONTROL_REG_A
        
            // -------0 : Stop timer
            // -------1 : Start timer
            if (value & 0x01) {
                delay |= CIACountA1 | CIACountA0;
                feed |= CIACountA0;
                if (!(CRA & 0x01))
                    PB67Toggle |= 0x40; // Toggle is high on start
            } else {
                delay &= ~(CIACountA1 | CIACountA0);
                feed &= ~CIACountA0;
            }
            
            // ------0- : Don't indicate timer underflow on port B
            // ------1- : Indicate timer underflow on port B bit 6
            if (value & 0x02) {
                PB67TimerMode |= 0x40;
                if (!(value & 0x04)) {
                    if ((delay & CIAPB7Low1) == 0) {
                        PB67TimerOut &= ~0x40;
                    } else {
                        PB67TimerOut |= 0x40;
                    }
                } else {
                    PB67TimerOut = (PB67TimerOut & ~0x40) | (PB67Toggle & 0x40);
                }
            } else {
                PB67TimerMode &= ~0x40;
            }
            
            // -----0-- : Upon timer underflow, invert port B bit 6
            // -----1-- : Upon timer underflow, generate a positive edge
            //            on port B bit 6 for one cycle

            // ----0--- : Timer restarts upon underflow
            // ----1--- : Timer stops upon underflow (One shot mode)
            if (value & 0x08) {
                feed |= CIAOneShotA0;
            } else {
                feed &= ~CIAOneShotA0;
            }
            
            // ---0---- : Nothing to do
            // ---1---- : Load start value into timer
            if (value & 0x10) {
                delay |= CIALoadA0;
            }

            // --0----- : Timer counts system cycles
            // --1----- : Timer counts positive edges on CNT pin
            if (value & 0x20) {
                delay &= ~(CIACountA1 | CIACountA0);
                feed &= ~CIACountA0;
            }
    
            // -0------ : Serial shift register in input mode (read)
            // -1------ : Serial shift register in output mode (write)
            if ((value ^ CRA) & 0x40) {

                // Serial direction changing
                trace(CIASER_DEBUG, "Serial register: %s\n", (value & 0x40) ? "output" : "input");

                // Inform the keyboard if this CIA is connected to it
                if (isCIAA()) keyboard.setSPLine(!(value & 0x40), clock);
                                
                serCounter = 0;
                
                delay &= ~(CIASsrToSdr0 | CIASsrToSdr1 | CIASsrToSdr2 | CIASsrToSdr3);
                delay &= ~(CIASdrToSsr0 | CIASdrToSsr1);
                feed &= ~CIASdrToSsr0;
            
                delay &= ~(CIASerClk0 | CIASerClk1 | CIASerClk2);
                feed &= ~CIASerClk0;
            }
            
            updatePB(); // Because PB67timerMode and PB6TimerOut may have changed
            CRA = value;
            
            return;
            
        case 0x0F: // CIA_CONTROL_REG_B
        {
            // -------0 : Stop timer
            // -------1 : Start timer
            if (value & 0x01) {
                delay |= CIACountB1 | CIACountB0;
                feed |= CIACountB0;
                if (!(CRB & 0x01))
                    PB67Toggle |= 0x80; // Toggle is high on start
            } else {
                delay &= ~(CIACountB1 | CIACountB0);
                feed &= ~CIACountB0;
            }
            
            // ------0- : Don't indicate timer underflow on port B
            // ------1- : Indicate timer underflow on port B bit 7
            if (value & 0x02) {
                PB67TimerMode |= 0x80;
                if ((value & 0x04) == 0) {
                    if ((delay & CIAPB7Low1) == 0) {
                        PB67TimerOut &= ~0x80;
                    } else {
                        PB67TimerOut |= 0x80;
                    }
                } else {
                    PB67TimerOut = (PB67TimerOut & ~0x80) | (PB67Toggle & 0x80);
                }
            } else {
                PB67TimerMode &= ~0x80;
            }
            
            // -----0-- : Upon timer underflow, invert port B bit 7
            // -----1-- : Upon timer underflow, generate a positive edge
            //            on port B bit 7 for one cycle
            
            // ----0--- : Timer restarts upon underflow
            // ----1--- : Timer stops upon underflow (One shot mode)
            if (value & 0x08) {
                feed |= CIAOneShotB0;
            } else {
                feed &= ~CIAOneShotB0;
            }
            
            // ---0---- : Nothing to do
            // ---1---- : Load start value into timer
            if (value & 0x10) {
                delay |= CIALoadB0;
            }
            
            // -00----- : Timer counts system cycles
            // -01----- : Timer counts positive edges on CNT pin
            // -10----- : Timer counts underflows of timer A
            // -11----- : Timer counts underflows of timer A occurring along with a
            //            positive edge on CNT pin
            if (value & 0x60) {
                delay &= ~(CIACountB1 | CIACountB0);
                feed &= ~CIACountB0;
            }
            
            // 0------- : Writing into TOD registers sets TOD
            // 1------- : Writing into TOD registers sets alarm time
            
            updatePB(); // Because PB67timerMode and PB6TimerOut may have changed
            CRB = value;
            
            return;
        }
            
        default:
            assert(false);
    }
}
