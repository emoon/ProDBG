// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

#define CIA_DEBUG (nr == 0 ? CIAA_DEBUG : CIAB_DEBUG)

CIA::CIA(int n, Amiga& ref) : AmigaComponent(ref), nr(n)
{    
    subComponents = vector<HardwareComponent *> { &tod };
    
    config.revision = CIA_8520_DIP;
    config.todBug = true;
    config.eClockSyncing = true;
    
    PA = 0xFF;
    PB = 0xFF;
}

void
CIA::_reset(bool hard)
{
    if (!hard) wakeUp();

    RESET_SNAPSHOT_ITEMS(hard)
    
    CNT = true;
    INT = 1;
    
    counterA = 0xFFFF;
    counterB = 0xFFFF;
    latchA = 0xFFFF;
    latchB = 0xFFFF;
    
    // UAE initializes CRB with 4 (which I think is wrong)
    if (MIMIC_UAE) CRB = 0x4;

    updatePA();
    updatePB();
    
    // Update the memory layout because the OVL bit may have changed
    mem.updateMemSrcTables();
}

long
CIA::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_CIA_REVISION:   return config.revision;
        case OPT_TODBUG:         return config.todBug;
        case OPT_ECLOCK_SYNCING: return config.eClockSyncing;
        
        default:
            assert(false);
            return 0;
    }
}

bool
CIA::setConfigItem(Option option, long value)
{
    switch (option) {
            
        case OPT_CIA_REVISION:
            
            if (!CIARevisionEnum::verify(value)) return false;
            
            if (config.revision == value) {
                return false;
            }
            
            config.revision = (CIARevision)value;
            return true;

        case OPT_TODBUG:
            
            if (config.todBug == value) {
                return false;
            }
            
            config.todBug = value;
            return true;
            
        case OPT_ECLOCK_SYNCING:
            
            if (config.eClockSyncing == value) {
                return false;
            }

            config.eClockSyncing = value;
            return true;
            
        default:
            return false;
    }
}

void
CIA::_dumpConfig() const
{
    msg("      revision : %s\n", CIARevisionEnum::key(config.revision));
    msg("        todBug : %s\n", config.todBug ? "yes" : "no");
    msg(" eClockSyncing : %s\n", config.eClockSyncing ? "yes" : "no");
}

void
CIA::_inspect()
{
    synchronized {
        
        updatePA();
        info.portA.port = PA;
        info.portA.reg = PRA;
        info.portA.dir = DDRA;
        
        updatePB();
        info.portB.port = PB;
        info.portB.reg = PRB;
        info.portB.dir = DDRB;
        
        info.timerA.count = LO_HI(spypeek(0x04), spypeek(0x05));
        info.timerA.latch = latchA;
        info.timerA.running = (delay & CIACountA3);
        info.timerA.toggle = CRA & 0x04;
        info.timerA.pbout = CRA & 0x02;
        info.timerA.oneShot = CRA & 0x08;
        
        info.timerB.count = LO_HI(spypeek(0x06), spypeek(0x07));
        info.timerB.latch = latchB;
        info.timerB.running = (delay & CIACountB3);
        info.timerB.toggle = CRB & 0x04;
        info.timerB.pbout = CRB & 0x02;
        info.timerB.oneShot = CRB & 0x08;
        
        info.sdr = sdr;
        info.ssr = ssr;
        info.icr = icr;
        info.imr = imr;
        info.intLine = INT;
        
        info.cnt = tod.info;
        info.cntIntEnable = imr & 0x04;
        
        info.idleSince = idleSince();
        info.idleTotal = idleTotal();
        info.idlePercentage = clock ? (double)idleCycles / (double)clock : 100.0;
    }
}

void
CIA::_dump() const
{    
    msg("                   Clock : %lld\n", clock);
    msg("                Sleeping : %s\n", sleeping ? "yes" : "no");
    msg("               Tiredness : %d\n", tiredness);
    msg(" Most recent sleep cycle : %lld\n", sleepCycle);
    msg("Most recent wakeup cycle : %lld\n", wakeUpCycle);
    msg("\n");
    msg("               Counter A : %04X\n", counterA);
    msg("                 Latch A : %04X\n", latchA);
    msg("         Data register A : %02X\n", PRA);
    msg("   Data port direction A : %02X\n", DDRA);
    msg("             Data port A : %02X\n", PA);
    msg("      Control register A : %02X\n", CRA);
    msg("\n");
    msg("               Counter B : %04X\n", counterB);
    msg("                 Latch B : %04X\n", latchB);
    msg("         Data register B : %02X\n", PRB);
    msg("   Data port direction B : %02X\n", DDRB);
    msg("             Data port B : %02X\n", PB);
    msg("      Control register B : %02X\n", CRB);
    msg("\n");
    msg("   Interrupt control reg : %02X\n", icr);
    msg("      Interrupt mask reg : %02X\n", imr);
    msg("\n");
    msg("                 SDR/SSR : %02X/%02X\n", sdr, ssr);
    msg("              serCounter : %02X\n", serCounter);
    msg("\n");
    msg("                     CNT : %d\n", CNT);
    msg("                     INT : %d\n", INT);
    msg("\n");
}

void
CIA::emulateRisingEdgeOnFlagPin()
{
    wakeUp();
}

void
CIA::emulateFallingEdgeOnFlagPin()
{
    wakeUp();

    icr |= 0x10;
    
    if (imr & 0x10) {
        triggerFlagPinIrq();
    }
}

void
CIA::emulateRisingEdgeOnCntPin()
{
    trace(CIASER_DEBUG, "emulateRisingEdgeOnCntPin\n");
    
    wakeUp();
    CNT = 1;
    
    // Timer A
    if ((CRA & 0x21) == 0x21) delay |= CIACountA1;
    
    // Timer B
    if ((CRB & 0x61) == 0x21) delay |= CIACountB1;
    
    // Serial register
    if (!(CRA & 0x40) /* input mode */ ) {
        
        // debug("rising CNT: serCounter %d\n", serCounter);
        if (serCounter == 0) serCounter = 8;
        trace(CIASER_DEBUG, "Clocking in bit %d [%d]\n", SP, serCounter);
        
        // Shift in a bit from the SP line
        ssr = (u8)(ssr << 1) | (u8)SP;
        
        // Perform special action if a byte is complete
        if (--serCounter == 0) {
            
            // Load the data register (SDR) with the shift register (SSR)
            trace(CIASER_DEBUG, "Loading %x into sdr\n", sdr);
            delay |= CIASsrToSdr0; // sdr = ssr;
            
            // Trigger interrupt
            delay |= CIASerInt0;
            // debug(KBD_DEBUG, "Received serial byte: %02x\n", sdr);
        }
    }
}

void
CIA::emulateFallingEdgeOnCntPin()
{
    trace(CIASER_DEBUG, "emulateFallingEdgeOnCntPin\n");

    wakeUp();
    CNT = 0;
}

void
CIA::reloadTimerA()
{
    counterA = latchA;
    
    // Make sure the timer waits for one cycle before it continues to count
    delay &= ~CIACountA2;
}

void
CIA::reloadTimerB()
{
    counterB = latchB;
    
    // Make sure the timer waits for one cycle before it continues to count
    delay &= ~CIACountB2;
}

void
CIA::triggerTimerIrq()
{
    trace(CIA_DEBUG, "triggerTimerIrq()\n");
    delay |= (delay & CIAReadIcr0) ? CIASetInt0 : CIASetInt1;
    delay |= (delay & CIAReadIcr0) ? CIASetIcr0 : CIASetIcr1;
}

void
CIA::triggerTodIrq()
{
    trace(CIA_DEBUG, "triggerTodIrq()\n");
    delay |= CIASetInt0;
    delay |= CIASetIcr0;
}

void
CIA::triggerFlagPinIrq()
{
    trace(CIA_DEBUG, "triggerFlagPinIrq()\n");
    delay |= CIASetInt0;
    delay |= CIASetIcr0;
}

void
CIA::triggerSerialIrq()
{
    trace(CIA_DEBUG, "triggerSerialIrq()\n");
    delay |= CIASetInt0;
    delay |= CIASetIcr0;
}

void
CIA::todInterrupt()
{
    wakeUp();
    delay |= CIATODInt0;
}

void
CIA::executeOneCycle()
{
    clock += CIA_CYCLES(1);
    
    // debug("Executing CIA: new clock = %lld\n", clock);
    
    u64 oldDelay = delay;
    u64 oldFeed  = feed;
    
    //
	// Layout of timer (A and B)
	//

    // Source: "A Software Model of the CIA6526" by Wolfgang Lorenz
	//
    //                           Phi2            Phi2                  Phi2
	//                            |               |                     |
	// timerA      -----    ------v------   ------v------     ----------v---------
	// input  ---->| & |--->| dwDelay & |-X-| dwDelay & |---->| decrement counter|
	//         --->|   |    |  CountA2  | | |  CountA3  |     |        (1)       |
	//         |   -----    ------------- | -------------     |                  |
	// -----------------          ^ Clr   |                   |                  |
	// | bCRA & 0x01   | Clr (3)  |       | ------------------| new counter = 0? |
	// | timer A start |<----     |       | |                 |                  |
	// -----------------    |     |       v v                 |                  |
 	//                    -----   |      -----                |      timer A     |
	//                    | & |   |      | & |                |  16 bit counter  |
	//                    |   |   |      |   |                |     and latch    |
	//                    -----   |      -----                |                  |
    //                     ^ ^    |        |(2)               |                  |
    //                     | |    ---------|-------------     |                  |
    //                     | |             |            |     |                  |
	// timer A             | |             |    -----   |     |                  |
	// output  <-----------|-X-------------X--->|>=1|---X---->| load from latch  |
	//                     |                --->|   |         |        (4)       |
	//                    -----             |   -----         --------------------
	//                    |>=1|             |
	//                    |   |             |       Phi2
	//                    -----             |        |
	//                     ^ ^              |  ------v------    ----------------
	//                     | |              ---| dwDelay & |<---| bcRA & 0x10  |
	//                     | ----------------  |  LoadA1   |    | force load   |
	//                     |       Phi2     |  -------------    ----------------
    //                     |        |       |                            ^ Clr
	// -----------------   |  ------v------ |                            |
	// | bCRA & 0x08   |   |  | dwDelay & | |                           Phi2
	// | one shot      |---X->| oneShotA0 |--
	// -----------------      -------------

				
	// Timer A

	// Decrement counter

	if (delay & CIACountA3)
		counterA--; // (1)
	
	// Check underflow condition
	bool timerAOutput = (counterA == 0 && (delay & CIACountA2)); // (2)
	
	if (timerAOutput) {
        
        trace(CIA_DEBUG, "Timer A underflow\n");
        
        icrAck &= ~0x01;
        
		// Stop timer in one shot mode
		if ((delay | feed) & CIAOneShotA0) { // (3)
			CRA &= ~0x01;
			delay &= ~(CIACountA2 | CIACountA1 | CIACountA0);
			feed &= ~CIACountA0;
		}
		
		// Timer A output to timer B in cascade mode
		if ((CRB & 0x61) == 0x41 || ((CRB & 0x61) == 0x61 && CNT)) {
			delay |= CIACountB1;
		}
        
        // Reload counter immediately
		delay |= CIALoadA1;
	}
    
	// Load counter
	if (delay & CIALoadA1) // (4)
		reloadTimerA(); 
	
	// Timer B
	
	// Decrement counter
	if (delay & CIACountB3) {
		counterB--; // (1)
    } 

	// Check underflow condition
	bool timerBOutput = (counterB == 0 && (delay & CIACountB2)); // (2)
	
	if (timerBOutput) {
				
        // debug("Timer B underflow\n");

        icrAck &= ~0x02;
        
		// Stop timer in one shot mode
		if ((delay | feed) & CIAOneShotB0) { // (3)
			CRB &= ~0x01;
			delay &= ~(CIACountB2 | CIACountB1 | CIACountB0);
			feed &= ~CIACountB0;
		}
		delay |= CIALoadB1;
	}
	
	// Load counter
	if (delay & CIALoadB1) // (4)
		reloadTimerB();
		
    //
    // Serial register
    //
    
    if (delay & CIASsrToSdr3) {
        sdr = ssr;
    }
    
    // Generate clock signal
    if (timerAOutput && (CRA & 0x40) /* output mode */ ) {
        
        if (serCounter) {
            
            // Toggle serial clock signal
            feed ^= CIASerClk0;
            
        } else if (delay & CIASdrToSsr1) {
            
            // Load the shift register (SSR) with the data register (SDR)
            ssr = sdr;
            delay &= ~(CIASdrToSsr1 | CIASdrToSsr0);
            feed &= ~CIASdrToSsr0;
            serCounter = 8;
            
            // Toggle serial clock signal
            feed ^= CIASerClk0;
        }
    }
    
    // Run shift register with generated clock signal
    if (serCounter && (CRA & 0x40) /* output mode */) {
        if ((delay & (CIASerClk2 | CIASerClk1)) == CIASerClk1) {      // Positive edge
            if (serCounter == 1) {
                delay |= CIASerInt0; // Trigger interrupt
            }
        }
        else if ((delay & (CIASerClk2 | CIASerClk1)) == CIASerClk2) { // Negative edge
            serCounter--;
        }
    }
	
	//
	// Timer output to PB6 (timer A) and PB7 (timer B)
    // 
	
	// Source: "A Software Model of the CIA6526" by Wolfgang Lorenz
	//
	//                     (7)            -----------------
    //         -------------------------->| bCRA & 0x04   |
    //         |                          | timer mode    |  ----------------
	//         |                          | 0x00: pulse   |->| 0x02 (timer) |
	// timerA  | Flip ---------------     |       (7)     |  |              |
    // output -X----->| bPB67Toggle |---->| 0x04: toggle  |  | bCRA & 0x02  |
	//            (5) |  ^ 0x40     |     |       (8)     |  | output mode  |-> PB6 out
	//                ---------------     -----------------  |              |
	//                       ^ Set        -----------------  | 0x00 (port)  |
	//                       |            | port B bit 6  |->|              |
	// ----------------- 0->1|            |    output     |  ----------------
	// | bCRA & 0x01   |------            -----------------
	// | timer A start |
	// -----------------

	// Timer A output to PB6
	
	if (timerAOutput) {
		
		PB67Toggle ^= 0x40; // (5) toggle underflow counter bit
		
		if (CRA & 0x02) { // (6)

			if ((CRA & 0x04) == 0) { 
				// (7) set PB6 high for one clock cycle
				PB67TimerOut |= 0x40;
				delay |= CIAPB6Low0;
				delay &= ~CIAPB6Low1;
			} else { 
				// (8) toggle PB6 (copy bit 6 from PB67Toggle)
				// PB67TimerOut = (PB67TimerOut & 0xBF) | (PB67Toggle & 0x40);
                PB67TimerOut ^= 0x40;
			}
		}
	}

	// Timer B output to PB7
	
	if (timerBOutput) {
		
		PB67Toggle ^= 0x80; // (5) toggle underflow counter bit
	
		if (CRB & 0x02) { // (6)
		
			if ((CRB & 0x04) == 0) {
				// (7) set PB7 high for one clock cycle
				PB67TimerOut |= 0x80;
				delay |= CIAPB7Low0;
				delay &= ~CIAPB7Low1;
			} else {
				// (8) toggle PB7 (copy bit 7 from PB67Toggle)
				// PB67TimerOut = (PB67TimerOut & 0x7F) | (PB67Toggle & 0x80);
                PB67TimerOut ^= 0x80;
			}
		}
	}
	
	// Set PB67 back to low
	if (delay & CIAPB6Low1)
		PB67TimerOut &= ~0x40;

	if (delay & CIAPB7Low1)
		PB67TimerOut &= ~0x80;

	
	//
	// Interrupt logic
    //
	
	// Source: "A Software Model of the CIA6526" by Wolfgang Lorenz
	//
	//                      ----------
	//                      | bIMR & |----
	//                      |  0x01  |   |    -----
	//                      ----------   ---->| & |----
	// timerA       (9) Set ----------   ---->|   |   |
	// output  ------------>| bICR & |   |    -----   |
	//           ---------->|  0x01  |----            |  -----
	//           |      Clr ----------                -->|>=1|---
	//           |          ----------                -->|   |  |
	//           |          | bIMR & |----            |  -----  |
	//           |          |  0x02  |   |    -----   |         |
	//           |          ----------   ---->| & |----         |
	// timerB    | (10) Set ----------   ---->|   |             |
	// output  --|--------->| bICR & |   |    -----             |
	//           X--------->|  0x01  |----                      |
	//           |      Clr ----------       	                |
	// read      |                                              |
	// ICR ------X---------------X-------------------           |
	//                           | (12)             |           |
	//                           v Clr              v Clr       |
	//           ------      ----------      ----------------   | (11)
	// Int    <--| -1 |<-----| bICR & |<-----|   dwDelay &  |<---
	// ouptput   |    |      |  0x80  | Set  |  Interrupt1  |
	// (14)      ------      ---------- (13) -------^--------
	//                                              |
	//                                             Phi2
    
	if (timerAOutput) { // (9)
		icr |= 0x01;
	}

    if (timerBOutput) { // (10)
        icr |= 0x02;
    }
    
    // Check for timer interrupt
    if ((timerAOutput && (imr & 0x01)) || (timerBOutput && (imr & 0x02))) { // (11)
        triggerTimerIrq();
    }

    // Check for TOD interrupt
    if (delay & CIATODInt0) {
        icr |= 0x04;
        if (imr & 0x04) {
            triggerTodIrq();
        }
    }
    
    // Check for Serial interrupt
    if (delay & CIASerInt2) {
        icr |= 0x08;
        if (imr & 0x08) {
            triggerSerialIrq();
        }
    }
    
    if (delay & (CIAClearIcr1 | CIAAckIcr1 | CIASetIcr1 | CIASetInt1 | CIAClearInt0)) {
        
        if (delay & CIAClearIcr1) { // (12)
            icr &= 0x7F;
        }
        if (delay & CIAAckIcr1) {
            icr &= ~icrAck;
        }
        if (delay & CIASetIcr1) { // (13)
            icr |= 0x80;
        }
        if (delay & CIASetInt1) { // (14)
            INT = 0;
            pullDownInterruptLine();
        }
        if (delay & CIAClearInt0) { // (14)
            INT = 1;
            releaseInterruptLine();
        }
    }

	// Move delay flags left and feed in new bits
	delay = ((delay << 1) & CIADelayMask) | feed;
    
    // Go into idle state if possible
    if (oldDelay == delay && oldFeed == feed) tiredness++; else tiredness = 0;
  
    // Sleep if threshold is reached
    if (tiredness > 8 && !CIA_ON_STEROIDS) {
        sleep();
        scheduleWakeUp();
        return;
    }
    
    scheduleNextExecution();
}

void
CIA::sleep()
{
    // Don't call this method on a sleeping CIA
    assert(!sleeping);
    
    // Determine maximum possible sleep cycle based on timer counts
    assert(IS_CIA_CYCLE(clock));
    Cycle sleepA = clock + CIA_CYCLES((counterA > 2) ? (counterA - 1) : 0);
    Cycle sleepB = clock + CIA_CYCLES((counterB > 2) ? (counterB - 1) : 0);
    
    // CIAs with stopped timers can sleep forever
    if (!(feed & CIACountA0)) sleepA = INT64_MAX;
    if (!(feed & CIACountB0)) sleepB = INT64_MAX;
    Cycle sleep = MIN(sleepA, sleepB);
    
    // ZZzzz
    // debug("ZZzzzz: clock = %lld A = %d B = %d sleepA = %lld sleepB = %lld\n", clock, counterA, counterB, sleepA, sleepB);
    sleepCycle = clock;
    wakeUpCycle = sleep;
    sleeping = true;
    tiredness = 0;
}

void
CIA::wakeUp()
{
    if (!sleeping) return;
    sleeping = false;
    
    Cycle targetCycle = CIA_CYCLES(AS_CIA_CYCLES(agnus.clock));
    wakeUp(targetCycle);
}

void
CIA::wakeUp(Cycle targetCycle)
{
    assert(clock == sleepCycle);

    // Calculate the number of missed cycles
    Cycle missedCycles = targetCycle - sleepCycle;
    assert(missedCycles % CIA_CYCLES(1) == 0);
    
    // Make up for missed cycles
    if (missedCycles > 0) {
        
        if (feed & CIACountA0) {
            assert(counterA >= AS_CIA_CYCLES(missedCycles));
            counterA -= AS_CIA_CYCLES(missedCycles);
            // debug("Making up %d timer A cycles\n", AS_CIA_CYCLES(missedCycles));
        }
        if (feed & CIACountB0) {
            assert(counterB >= AS_CIA_CYCLES(missedCycles));
            counterB -= AS_CIA_CYCLES(missedCycles);
            // debug("Making up %d timer B cycles\n", AS_CIA_CYCLES(missedCycles));
        }
        
        idleCycles += missedCycles;
        clock = targetCycle;
    }
    
    // Schedule the next execution event
    scheduleNextExecution();
}

CIACycle
CIA::idleSince() const
{
    return isAwake() ? 0 : AS_CIA_CYCLES(agnus.clock - sleepCycle);
}


//
// CIA A
//

CIAA::CIAA(Amiga& ref) : CIA(0, ref)
{
}

void
CIAA::_powerOn()
{
    CIA::_powerOn();
    messageQueue.put(MSG_POWER_LED_DIM);
}

void
CIAA::_powerOff()
{
    messageQueue.put(MSG_POWER_LED_OFF);
}

void 
CIAA::pullDownInterruptLine()
{
    trace(CIA_DEBUG, "Pulling down IRQ line\n");
    paula.raiseIrq(INT_PORTS);
}

void 
CIAA::releaseInterruptLine()
{
    trace(CIA_DEBUG, "Releasing IRQ line\n");
}

//              -------
//     OVL <--- | PA0 |  Overlay Rom
//    /LED <--- | PA1 |  Power LED
//   /CHNG ---> | PA2 |  Floppy drive disk change signal
//   /WPRO ---> | PA3 |  Floppy drive write protection enabled
//    /TK0 ---> | PA4 |  Floppy drive track 0 indicator
//    /RDY ---> | PA5 |  Floppy drive ready
//   /FIR0 ---> | PA6 |  Port 0 fire button
//   /FIR1 ---> | PA7 |  Port 1 fire button
//              -------

void
CIAA::updatePA()
{
    u8 internal = portAinternal();
    u8 external = portAexternal();

    u8 oldPA = PA;
    PA = (internal & DDRA) | (external & ~DDRA);

    // A connected device may force the output level to a specific value
    controlPort1.changePra(PA);
    controlPort2.changePra(PA);

    // PLCC CIAs always return the PRA contents for output bits
    // We ignore PLCC emulation until the A600 is supported
    // if (config.type == CIA_8520_PLCC) PA = (PA & ~DDRA) | (PRA & DDRA);

    // Check the LED bit
    if ((oldPA ^ PA) & 0b00000010) {
        messageQueue.put((PA & 0b00000010) ? MSG_POWER_LED_DIM : MSG_POWER_LED_ON);
    }

    // Check the OVL bit which controls the Kickstart ROM overlay
    if ((oldPA ^ PA) & 0b00000001) {
        mem.updateMemSrcTables();
    }
    
    /*
    if (oldPA ^ PA) {
        debug("## PA changed: /FIR1: %d /FIR0: %d /RDY: %d /TK0: %d /WPRO: %d /CHNG: %d /LED: %d OVL: %d\n",
              !!(PA & 0x80), !!(PA & 0x40), !!(PA & 0x20), !!(PA & 0x10),
              !!(PA & 0x08), !!(PA & 0x04), !!(PA & 0x02), !!(PA & 0x01));
    }
    */
}

u8
CIAA::portAinternal() const
{
    return PRA;
}

u8
CIAA::portAexternal() const
{
    u8 result;
    
    // Set drive status bits
    result = diskController.driveStatusFlags();

    // The OVL bit must be 1
    assert(result & 1);
    
    return result;
}

//                    -------
//  Centronics 0 <--> | PB0 |
//  Centronics 1 <--> | PB1 |
//  Centronics 2 <--> | PB2 |
//  Centronics 3 <--> | PB3 |
//  Centronics 4 <--> | PB4 |
//  Centronics 5 <--> | PB5 |
//  Centronics 6 <--> | PB6 |
//  Centronics 7 <--> | PB7 |
//                    -------

void
CIAA::updatePB()
{
    u8 internal = portBinternal();
    u8 external = portBexternal();

    PB = (internal & DDRB) | (external & ~DDRB);

    // Check if timer A underflows show up on PB6
    if (GET_BIT(PB67TimerMode, 6))
        REPLACE_BIT(PB, 6, PB67TimerOut & (1 << 6));
    
    // Check if timer B underflows show up on PB7
    if (GET_BIT(PB67TimerMode, 7))
        REPLACE_BIT(PB, 7, PB67TimerOut & (1 << 7));

    // PLCC CIAs always return the PRB contents for output bits
    // We ignore PLCC emulation until the A600 is supported
    // if (config.type == CIA_8520_PLCC) PB = (PB & ~DDRB) | (PRB & DDRB);
}

u8
CIAA::portBinternal() const
{
    return PRB;
}

u8
CIAA::portBexternal() const
{
    return 0xFF;
}

void
CIAA::setKeyCode(u8 keyCode)
{
    trace(KBD_DEBUG, "setKeyCode: %x\n", keyCode);
    
    // Put the key code into the serial data register
    sdr = keyCode;
    
    // Trigger a serial data interrupt
    delay |= CIASerInt0;

    // Wake up the CIA
    wakeUp();
}

//
// CIA B
// 

CIAB::CIAB(Amiga& ref) : CIA(1, ref)
{
}

void 
CIAB::pullDownInterruptLine()
{
    trace(CIA_DEBUG, "Pulling down IRQ line\n");
    paula.raiseIrq(INT_EXTER);
}

void 
CIAB::releaseInterruptLine()
{
    trace(CIA_DEBUG, "Releasing IRQ line\n");
}

//                                 -------
//      Parallel port: BUSY   ---> | PA0 |
//      Parallel Port: POUT   ---> | PA1 |
//  Parallel / Serial: SEL/RI ---> | PA2 |
//        Serial port: /DSR   ---> | PA3 |
//        Serial port: /CTS   ---> | PA4 |
//        Serial port: /CD    ---> | PA5 |
//        Serial port: /RTS   <--- | PA6 |
//        Serial port: /DTR   <--- | PA7 |
//                                 -------

u8
CIAB::portAinternal() const
{
    return PRA;
}

u8
CIAB::portAexternal() const
{
    u8 result = 0xFF;

    // Parallel port
    // NOT IMPLEMENTED

    // Shared between parallel and serial port
    if (serialPort.getRI()) CLR_BIT(result, 2);

    // Serial port
    if (serialPort.getDSR()) CLR_BIT(result, 3);
    if (serialPort.getCTS()) CLR_BIT(result, 4);
    if (serialPort.getCD())  CLR_BIT(result, 5);
    if (serialPort.getRTS()) CLR_BIT(result, 6);
    if (serialPort.getDTR()) CLR_BIT(result, 7);

    return result;
}

void
CIAB::updatePA()
{
    // debug(CIA_DEBUG, "updatePA()\n");

    u8 internal = portAinternal();
    u8 external = portAexternal();

    u8 oldPA = PA;
    PA = (internal & DDRA) | (external & ~DDRA);

    // Drive serial pins if they are configured as output
    if (GET_BIT(DDRA, 6)) serialPort.setRTS(!GET_BIT(internal, 6));
    if (GET_BIT(DDRA, 7)) serialPort.setDTR(!GET_BIT(internal, 7));
    
    // PLCC CIAs always return the PRA contents for output bits
    // We ignore PLCC emulation until the A600 is supported
    // if (config.type == CIA_8520_PLCC) PA = (PA & ~DDRA) | (PRA & DDRA);

    /* Inside the Amiga, PA0 and PA1 of CIAB are wired to the SP pin and the
     * CNT pin, respectively. If the shift register is run in input mode,
     * a positive edge on the CNT pin will transfer the value on the SP pin
     * into the shift register. To shift in the correct value, we need to set
     * the SP pin first and emulate the edge on the CNT pin afterwards.
     */
    if (DDRA & 1) setSP(PA & 1); else setSP(1);

    if (!(oldPA & 2) &&  (PA & 2)) emulateRisingEdgeOnCntPin();
    if ( (oldPA & 2) && !(PA & 2)) emulateFallingEdgeOnCntPin();
    
}

//            -------
//  /STEP <-- | PB0 |   (Floppy drive step heads)
//    DIR <-- | PB1 |   (Floppy drive head direction)
//  /SIDE <-- | PB2 |   (Floppy drive side select)
//  /SEL0 <-- | PB3 |   (Floppy drive select df0)
//  /SEL1 <-- | PB4 |   (Floppy drive select df1)
//  /SEL2 <-- | PB5 |   (Floppy drive select df2)
//  /SEL3 <-- | PB6 |   (Floppy drive select df3)
//   _MTR <-- | PB7 |   (Floppy drive motor on)
//            -------

u8
CIAB::portBinternal() const
{
    u8 result = PRB;
    
    // Check if timer A underflow shows up on PB6
    if (GET_BIT(PB67TimerMode, 6))
        REPLACE_BIT(result, 6, PB67TimerOut & (1 << 6));

    // Check if timer B underflows show up on PB7
    if (GET_BIT(PB67TimerMode, 7))
        REPLACE_BIT(result, 7, PB67TimerOut & (1 << 7));

    return result;
}

u8
CIAB::portBexternal() const
{
    return 0xFF;
}

void
CIAB::updatePB()
{
    u8 internal = portBinternal();
    u8 external = portBexternal();

    u8 oldPB = PB;
    PB = (internal & DDRB) | (external & ~DDRB);

    // PLCC CIAs always return the PRB contents for output bits
    // We ignore PLCC emulation until the A600 is supported
    // if (config.type == CIA_8520_PLCC) PB = (PB & ~DDRB) | (PRB & DDRB);

    // Notify the disk controller about the changed bits
    if (oldPB ^ PB) {
        /*
        debug("PB changed: MTR: %d SEL3: %d SEL2: %d SEL1: %d SEL0: %d SIDE: %d DIR: %d STEP: %d\n",
              !!(PB & 0x80), !!(PB & 0x40), !!(PB & 0x20), !!(PB & 0x10),
              !!(PB & 0x08), !!(PB & 0x04), !!(PB & 0x02), !!(PB & 0x01));
        */
        diskController.PRBdidChange(oldPB, PB);
    }
}
