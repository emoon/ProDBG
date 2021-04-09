// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "StateMachineTypes.h"
#include "AmigaComponent.h"
#include "Sampler.h"
#include "Agnus.h"

template <isize nr>
class StateMachine : public AmigaComponent {

    // Result of the latest inspection
    AudioChannelInfo info;

public:

    // The state machine has been executed up to this clock cycle
    Cycle clock;

    // The current state of this machine
    i8 state;

    // The 16 bit output buffer
    u16 buffer;

    // Audio length (AUDxLEN)
    u16 audlenLatch;
    u16 audlen;

    // Audio period (AUDxPER)
    u16 audperLatch;
    i32 audper;

    // Audio volume (AUDxVOL)
    u16 audvolLatch;
    u16 audvol;

    // Audio data (AUDxDAT)
    u16 auddat;

    // Audio location (AUDxLC)
    u32 audlcLatch;

    // Audio DMA request to Agnus for one word of data
    bool audDR;
        
    // Set to true if the next 011->010 transition should trigger an interrupt
    bool intreq2;

    /* Two locks regulate the access to the sample buffer.
     *
     * "The minimum period is 124 color clocks. This means that the smallest
     *  number that should be placed in this register [AUDxPER] is 124 decimal.
     *  This corresponds to a maximum sample frequency of 28.86 khz." [HRM]
     *
     * Many games initialize the period  programs write a value of 1 into
     * AUDxPER (e.g., James Pond 2 and Ghosts'n Goblins). As a result, the
     * sample buffer is flooded with identical samples. To prevent this,
     * these two variables prevent the sample buffer from being written to in
     * penlo() and penhi(). The locks are released whenever a new sample is
     * written into the AUDxDAT register.
     *
     * This feature is experimental (and might well be disabled).
     */
    bool enablePenlo = false;
    bool enablePenhi = false;

    
    //
    // Initializing
    //

public:

    StateMachine(Amiga& ref);

    const char *getDescription() const override;

private:
    
    void _reset(bool hard) override;

    
    //
    // Analyzing
    //
    
public:
    
    AudioChannelInfo getInfo() { return HardwareComponent::getInfo(info); }
    
private:
    
    void _inspect() override;
    void _dump(Dump::Category category, std::ostream& os) const override;
    
    
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
        worker

        << clock;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker
        
        << state
        << buffer
        << audlenLatch
        << audlen
        << audperLatch
        << audper
        << audvolLatch
        << audvol
        << auddat
        << audlcLatch
        << audDR
        << intreq2
        << enablePenlo
        << enablePenhi;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing registers
    //

public:

    void pokeAUDxLEN(u16 value);
    void pokeAUDxPER(u16 value);
    void pokeAUDxVOL(u16 value);
    void pokeAUDxDAT(u16 value);

    // Called when the DMA mode changes
    void enableDMA();
    void disableDMA();
    
    
    //
    // Performing state machine actions
    //

public:

    // Returns true if the state machine is running in DMA mode
    bool AUDxON() const;

    // Returns true if the audio interrupt is pending
    bool AUDxIP() const;

    // Asks Paula to trigger the audio interrupt
    void AUDxIR();

    // Asks Agnus for one word of data
    void AUDxDR() { audDR = true; }

    // Tells Agnus to reset the DMA pointer to the block start
    void AUDxDSR() { agnus.reloadAUDxPT<nr>(); }

    // Reloads the period counter from its backup latch
    void percntrld();

    // Reloads the length counter from its backup latch
    void lencntrld() { audlen = audlenLatch; }

    // Counts length counter down one notch
    void lencount() { audlen--; }

    // Checks if the length counter has finished
    bool lenfin() { return audlen == 1; }

    // Reloads the volume register from its backup latch
    void volcntrld() { audvol = audvolLatch; }

    // Loads the output buffer from holding latch written to by AUDxDAT
    void pbufld1();

    // Like pbufld1, but only during 010->011 transition with attach period
    void pbufld2();

    // Returns true in attach volume mode
    bool AUDxAV() const;

    // Returns true in attach period mode
    bool AUDxAP() const;

    // Condition for normal DMA and interrupt requests
    bool napnav() { return !AUDxAP() || AUDxAV(); }

    // Enables the high byte of data to go to the digital-analog converter
    void penhi();

    // Enables the high byte of data to go to the digital-analog converter
    void penlo();

    // Transfers DMA requests to Agnus (done in the first refresh cycle)
    void requestDMA() { if (audDR) { agnus.setAudxDR<nr>(); audDR = 0; } }
    
    
    //
    // Performing state machine transitions
    //

private:

    void move_000_010();
    void move_000_001();
    void move_001_000();
    void move_001_101();
    void move_101_000();
    void move_101_010();
    void move_010_011();
    void move_011_000();
    void move_011_010();

    
    //
    // Servicing events
    //

public:

    void serviceEvent();
};
