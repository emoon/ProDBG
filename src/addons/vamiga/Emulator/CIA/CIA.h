// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CIA_H
#define _CIA_H

#include "TOD.h"

#define CIACountA0     (1ULL << 0) // Decrements timer A
#define CIACountA1     (1ULL << 1)
#define CIACountA2     (1ULL << 2)
#define CIACountA3     (1ULL << 3)
#define CIACountB0     (1ULL << 4) // Decrements timer B
#define CIACountB1     (1ULL << 5)
#define CIACountB2     (1ULL << 6)
#define CIACountB3     (1ULL << 7)
#define CIALoadA0      (1ULL << 8) // Loads timer A
#define CIALoadA1      (1ULL << 9)
#define CIALoadA2      (1ULL << 10)
#define CIALoadB0      (1ULL << 11) // Loads timer B
#define CIALoadB1      (1ULL << 12)
#define CIALoadB2      (1ULL << 13)
#define CIAPB6Low0     (1ULL << 14) // Sets pin PB6 low
#define CIAPB6Low1     (1ULL << 15)
#define CIAPB7Low0     (1ULL << 16) // Sets pin PB7 low
#define CIAPB7Low1     (1ULL << 17)
#define CIASetInt0     (1ULL << 18) // Triggers an interrupt
#define CIASetInt1     (1ULL << 19)
#define CIAClearInt0   (1ULL << 20) // Releases the interrupt line
#define CIAOneShotA0   (1ULL << 21)
#define CIAOneShotB0   (1ULL << 22)
#define CIAReadIcr0    (1ULL << 23) // Indicates that ICR was read recently
#define CIAReadIcr1    (1ULL << 24)
#define CIAClearIcr0   (1ULL << 25) // Clears bit 8 in ICR register
#define CIAClearIcr1   (1ULL << 26)
#define CIAClearIcr2   (1ULL << 27)
#define CIAAckIcr0     (1ULL << 28) // Clears bit 0 - 7 in ICR register
#define CIAAckIcr1     (1ULL << 29)
#define CIASetIcr0     (1ULL << 30) // Sets bit 8 in ICR register
#define CIASetIcr1     (1ULL << 31)
#define CIATODInt0     (1ULL << 32) // Triggers an IRQ with TOD as source
#define CIASerInt0     (1ULL << 33) // Triggers an IRQ with serial reg as source
#define CIASerInt1     (1ULL << 34)
#define CIASerInt2     (1ULL << 35)
#define CIASdrToSsr0   (1ULL << 36) // Move serial data reg to serial shift reg
#define CIASdrToSsr1   (1ULL << 37)
#define CIASsrToSdr0   (1ULL << 38) // Move serial shift reg to serial data reg
#define CIASsrToSdr1   (1ULL << 39)
#define CIASsrToSdr2   (1ULL << 40)
#define CIASsrToSdr3   (1ULL << 41)
#define CIASerClk0     (1ULL << 42) // Clock signal driving the serial register
#define CIASerClk1     (1ULL << 43)
#define CIASerClk2     (1ULL << 44)
#define CIASerClk3     (1ULL << 45)

#define CIADelayMask ~((1ULL << 46) \
| CIACountA0 | CIACountB0 \
| CIALoadA0 | CIALoadB0 \
| CIAPB6Low0 | CIAPB7Low0 \
| CIASetInt0 | CIAClearInt0 \
| CIAOneShotA0 | CIAOneShotB0 \
| CIAReadIcr0 | CIAClearIcr0 \
| CIAAckIcr0 | CIASetIcr0 \
| CIATODInt0 | CIASerInt0 \
| CIASdrToSsr0 | CIASsrToSdr0 \
| CIASerClk0)

class CIA : public AmigaComponent {
    
    friend class TOD;
    
protected:

    // Identification number (0 = CIA A, 1 = CIA B)
    int nr;

    // Current configuration
    CIAConfig config;

    // Result of the latest inspection
    CIAInfo info;


    //
    // Sub components
    //

public:
    
    TOD tod = TOD(this, amiga);


    //
    // Internals
    //
    
    // The CIA has been executed up to this master clock cycle
    Cycle clock;

protected:

    // Total number of skipped cycles (used by the debugger, only)
    Cycle idleCycles;
    
    // Timer A counter
    u16 counterA;
    
    // Timer B counter
    u16 counterB;
        
    // Timer A latch
    u16 latchA;
    
    // Timer B latch
    u16 latchB;


    //
    // Control
    //
    
    // Action flags
    u64 delay;
    u64 feed;
    
    // Control registers
    u8 CRA;
    u8 CRB;
    
    // Interrupt control register
    u8 icr;
    
    // ICR bits that need to deleted when CIAAckIcr1 hits
    u8 icrAck;
    
    // Interrupt mask register
    u8 imr;
    
protected:
    
    // Bit mask for PB outputs (0 = port register, 1 = timer)
    u8 PB67TimerMode;
    
    // PB output bits 6 and 7 in timer mode
    u8 PB67TimerOut;
    
    // PB output bits 6 and 7 in toggle mode
    u8 PB67Toggle;
    
    
    //
    // Port registers
    //
    
protected:
    
    // Peripheral data registers
    u8 PRA;
    u8 PRB;
    
    // Data directon registers
    u8 DDRA;
    u8 DDRB;
    
    // Peripheral ports
    u8 PA;
    u8 PB;
    
    
    //
    // Shift register logic
    //
    
protected:
    
    /* Serial data register
     * http://unusedino.de/ec64/technical/misc/cia6526/serial.html
     * "The serial port is a buffered, 8-bit synchronous shift register system.
     *  A control bit selects input or output mode. In input mode, data on the
     *  SP pin is shifted into the shift register on the rising edge of the
     *  signal applied to the CNT pin. After 8 CNT pulses, the data in the shift
     *  register is dumped into the Serial Data Register and an interrupt is
     *  generated. In the output mode, TIMER A is used for the baud rate
     *  generator. Data is shifted out on the SP pin at 1/2 the underflow rate
     *  of TIMER A. [...] Transmission will start following a write to the
     *  Serial Data Register (provided TIMER A is running and in continuous
     *  mode). The clock signal derived from TIMER A appears as an output on the
     *  CNT pin. The data in the Serial Data Register will be loaded into the
     *  shift register then shift out to the SP pin when a CNT pulse occurs.
     *  Data shifted out becomes valid on the falling edge of CNT and remains
     *  valid until the next falling edge. After 8 CNT pulses, an interrupt is
     *  generated to indicate more data can be sent. If the Serial Data Register
     *  was loaded with new information prior to this interrupt, the new data
     *  will automatically be loaded into the shift register and transmission
     *  will continue. If the microprocessor stays one byte ahead of the shift
     *  register, transmission will be continuous. If no further data is to be
     *  transmitted, after the 8th CNT pulse, CNT will return high and SP will
     *  remain at the level of the last data bit transmitted. SDR data is
     *  shifted out MSB first and serial input data should also appear in this
     *  format.
     */
    u8 sdr;
        
    // Serial shift register
    u8 ssr;
    
    /* Shift register counter
     * The counter is set to 8 when the shift register is loaded and decremented
     * when a bit is shifted out.
     */
    i8 serCounter;
    
    //
    // Port pins
    //
    
    bool SP;
    bool CNT;
    bool INT;
    
    
    //
    // Speeding up emulation (sleep logic)
    //
    
    /* Idle counter. When the CIA's state does not change during execution,
     * this variable is increased by one. If it exceeds a certain threshhold,
     * the chip is put into idle state via sleep().
     */
    u8 tiredness;
    
public:
    
    // Indicates if the CIA is currently idle
    bool sleeping;
    
    /* The last executed cycle before the chip went idle.
     * The variable is set in sleep()
     */
    Cycle sleepCycle;
    
    /* The first cycle to be executed after the chip went idle.
     * The variable is set in sleep()
     */
    Cycle wakeUpCycle;


    //
    // Initializing
    //

public:
    
    CIA(int n, Amiga& ref);

    bool isCIAA() { return nr == 0; }
    bool isCIAB() { return nr == 1; }

    void _reset(bool hard) override;
    
    
    //
    // Configuring
    //
    
public:
    
    CIAConfig getConfig() { return config; }
    
    long getConfigItem(ConfigOption option);
    bool setConfigItem(ConfigOption option, long value) override;
    
    bool getEClockSyncing() { return config.eClockSyncing; }

private:
    
    void _dumpConfig() override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker
        
        & config.revision
        & config.todBug
        & config.eClockSyncing;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker

        & clock
        & idleCycles
        & tiredness
        & sleeping
        & sleepCycle
        & wakeUpCycle;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker
        
        & counterA
        & counterB
        & latchA
        & latchB
        & delay
        & feed
        & CRA
        & CRB
        & icr
        & icrAck
        & imr
        & PB67TimerMode
        & PB67TimerOut
        & PB67Toggle
        & PRA
        & PRB
        & DDRA
        & DDRB
        & PA
        & PB
        & sdr
        & ssr
        & serCounter
        & SP
        & CNT
        & INT;
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Analyzing
    //
    
public:
    
    CIAInfo getInfo() { return HardwareComponent::getInfo(info); }

protected:
    
    void _inspect() override;
    void _dump() override;

    
    //
    // Accessing registers
    //
    
public:
    
    // Reads a value from a CIA register
    u8 peek(u16 addr);
    
    // Reads a value from a CIA register without causing side effects
    u8 spypeek(u16 addr);
    
    // Writes a value into a CIA register
    void poke(u16 addr, u8 value);
    
    
    //
    // Accessing the data ports
    //
    
public:
    
    // Returns the data registers (call updatePA() or updatePB() first)
    u8 getPA() { return PA; }
    u8 getPB() { return PB; }

private:

    // Returns the data direction register
    u8 getDDRA() { return DDRA; }
    u8 getDDRB() { return DDRB; }
    
    // Computes the values we currently see at port A
    virtual void updatePA() = 0;
    
    // Returns the value driving port A from inside the chip
    virtual u8 portAinternal() = 0;
    
    // Returns the value driving port A from outside the chip
    virtual u8 portAexternal() = 0;
    
    // Computes the value we currently see at port B
    virtual void updatePB() = 0;
    
    // Values driving port B from inside the chip
    virtual u8 portBinternal() = 0;
    
    // Values driving port B from outside the chip
    virtual u8 portBexternal() = 0;
    
protected:
    
    // Action method for poking the PA register
    virtual void pokePA(u8 value) { PRA = value; updatePA(); }
    
    // Action method for poking the DDRA register
    virtual void pokeDDRA(u8 value) { DDRA = value; updatePA(); }
    
    
    //
    // Accessing port pins
    //
    
public:
    
    // Getter for the interrupt line
    bool irqPin() { return INT; }

    // Simulates an edge edge on the flag pin
    void emulateRisingEdgeOnFlagPin();
    void emulateFallingEdgeOnFlagPin();

    // Simulates an edge on the CNT pin
    void emulateRisingEdgeOnCntPin();
    void emulateFallingEdgeOnCntPin();

    // Sets the serial port pin
    void setSP(bool value) { SP = value; }
    
    
    //
    // Handling interrupts
    //

private:

    // Requests the CPU to interrupt
    virtual void pullDownInterruptLine() = 0;
    
    // Removes the interrupt requests
    virtual void releaseInterruptLine() = 0;
    
    // Loads a latched value into timer
    void reloadTimerA();
    void reloadTimerB();
    
    // Triggers an interrupt (invoked inside executeOneCycle())
    void triggerTimerIrq();
    void triggerTodIrq();
    void triggerFlagPinIrq();
    void triggerSerialIrq();
    
public:
    
    // Handles an interrupt request from TOD
    void todInterrupt();

    
    //
    // Handling events
    //
    
    // Schedules the next execution event
    void scheduleNextExecution();
    
    // Schedules the next wakeup event
    void scheduleWakeUp();

    
    //
    // Executing
    //
    
public:
    
    // Advances the 24-bit counter by one tick
    // void incrementTOD();
    
    // Executes the CIA for one CIA cycle
    void executeOneCycle();
    
    
    //
    // Speeding up (sleep logic)
    //
    
private:
    
    // Puts the CIA into idle state
    void sleep();
    
public:
    
    // Emulates all previously skipped cycles
    void wakeUp();
    void wakeUp(Cycle targetCycle);
    
    // Returns true if the CIA is in idle state
    bool isSleeping() { return sleeping; }
    
    // Returns true if the CIA is awake
    bool isAwake() { return !sleeping; }
        
    // The CIA is idle since this number of cycles
    CIACycle idleSince();
    
    // Total number of cycles the CIA was idle
    CIACycle idleTotal() { return idleCycles; }
};


//
// CIAA
//

class CIAA : public CIA {
    
public:
    
    CIAA(Amiga& ref);

private:
    
    void _powerOn() override;
    void _powerOff() override;
    
    void pullDownInterruptLine() override;
    void releaseInterruptLine() override;
    
    u8 portAinternal() override;
    u8 portAexternal() override;
    void updatePA() override;
    u8 portBinternal() override;
    u8 portBexternal() override;
    void updatePB() override;
    
public:

    // Indicates if the power LED is currently on or off
    bool powerLED() { return (PA & 0x2) == 0; }

    // Emulates the reception of a keycode from the keyboard
    void setKeyCode(u8 keyCode);
};


//
// CIAB
//

class CIAB : public CIA {
    
public:
    
    CIAB(Amiga& ref);
    
private:
        
    void pullDownInterruptLine() override;
    void releaseInterruptLine() override;
    
    u8 portAinternal() override;
    u8 portAexternal() override;
    void updatePA() override;
    u8 portBinternal() override;
    u8 portBexternal() override;
    void updatePB() override;
};

#endif
