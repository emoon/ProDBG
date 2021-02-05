// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"

typedef union
{
    struct
    {
        u8 lo;
        u8 mid;
        u8 hi;
    };
    u32 value;
}
Counter24;

class TOD : public AmigaComponent {
    
    friend CIA;
    
    // Reference to the connected CIA
    CIA *cia;

    // Result of the latest inspection
    CounterInfo info;
            
    // The 24 bit counter
    Counter24 tod;
    
    // Counter value before the latest increment
    Counter24 preTod;
    
    // Time stamp of the last increment
    Cycle lastInc;
        
    // The counter latch
    Counter24 latch;
    
    // Alarm value
    Counter24 alarm;
    
    /* Indicates if the TOD registers are frozen. The CIA chip freezes the
     * registers when the counter's high byte (bits 16 - 23) is read and
     * reactivates them, when the low byte (bits 0 - 7) is read. Although the
     * values stay constant, the internal clock continues to run.
     */
    bool frozen;
    
    /* Indicates if the TOD clock is halted. The CIA chip stops the TOD clock
     * when the counter's high byte (bits 16 - 23) is written and restarts it,
     * when the low byte (bits 0 - 7) is written.
     */
    bool stopped;
    
    /* Indicates if tod time matches the alarm value. This value is read in
     * checkIrq() for edge detection.
     */
    bool matching;
    
    
    //
    // Initializing
    //

public:

    TOD(CIA *cia, Amiga& ref);

    const char *getDescription() const override;
    void _reset(bool hard) override;


    //
    // Analyzing
    //
    
    CounterInfo getInfo() { return HardwareComponent::getInfo(info); }

    void _inspect() override;
    void _dump() const override;

    
    //
    // Serializing
    //
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & tod.value
        & preTod.value
        & lastInc
        & latch.value
        & alarm.value
        & frozen
        & stopped
        & matching;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing
    //
    
    // Returns the counter's high byte (bits 16 - 23).
    u8 getCounterHi(Cycle timeStamp = INT64_MAX) const;

    // Returns the counter's intermediate byte (bits 8 - 15).
    u8 getCounterMid(Cycle timeStamp = INT64_MAX) const;
    
    // Returns the counter's low byte (bits 0 - 7).
    u8 getCounterLo(Cycle timeStamp = INT64_MAX) const;

    // Returns the alarm value's high byte (bits 16 - 23).
    u8 getAlarmHi() const;

    // Returns the alarm value's intermediate byte (bits 8 - 15).
    u8 getAlarmMid() const;

    // Returns the alarm value's low byte (bits 0 - 7).
    u8 getAlarmLo() const;

    // Sets the counter's high byte (bits 16 - 23).
    void setCounterHi(u8 value);

    // Sets the counter's intermediate byte (bits 8 - 15).
    void setCounterMid(u8 value);

    // Sets the counter's low byte (bits 0 - 7).
    void setCounterLo(u8 value);

    // Sets the alarm value's high byte (bits 16 - 23).
    void setAlarmHi(u8 value);

    // Sets the alarm value's intermediate byte (bits 8 - 15).
    void setAlarmMid(u8 value);

    // Sets the alarm value's low byte (bits 0 - 7).
    void setAlarmLo(u8 value);


    //
    // Executing
    //

public:
    
    // Increments the counter
    void increment();
    
private:

    bool incLoNibble(u8 &counter);
    bool incHiNibble(u8 &counter);

    // Updates variable 'matching'. A positive edge triggers an interrupt.
    void checkIrq();

    // Freezes the counter
    void freeze() { if (!frozen) { latch.value = tod.value; frozen = true; } }
    
    // Unfreezes the counter
    void defreeze() { frozen = false; }
    
    // Stops the counter
    void stop() { stopped = true; }
    
    // Starts the counter
    void cont() { stopped = false; }
};
