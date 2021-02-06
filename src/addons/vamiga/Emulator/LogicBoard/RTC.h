// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _RTC_H
#define _RTC_H

#include "AmigaComponent.h"

class RTC : public AmigaComponent {

    // Current configuration
    RTCConfig config;

    /* The currently stored time. The RTC stores the time as a difference to
     * the time provided by the host machine. I.e.:
     *
     *     Time of the real-time clock = Time of the host machine + timeDiff
     *
     * By default, this variable is 0 which means that the Amiga's real-time
     * clock is identical to the one in the host machine.
     */
    time_t timeDiff;
    
    // The RTC registers
    u8 reg[4][16];
    
    // The last call to function getTime()
    Cycle lastCall;

    // Remembers the most recent query of the host machine's real-time clock
    Cycle lastMeasure;

    // The result of the most recent query
    time_t lastMeasuredValue;
    
    
    //
    // Constructing and serializing
    //
    
public:
    
    RTC(Amiga& ref);

private:
    
    void _reset(bool hard) override;

    
    //
    // Configuring
    //
    
public:
    
    RTCConfig getConfig() { return config; }
    
    long getConfigItem(ConfigOption option);
    bool setConfigItem(ConfigOption option, long value) override;
    
    bool isPresent() { return config.model != RTC_NONE; }

private:
    
    void _dumpConfig() override;

    
    //
    // Analyzing
    //
    
private:
    
    void _dump() override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.model;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker

        & timeDiff
        & reg
        & lastCall
        & lastMeasure
        & lastMeasuredValue;
    }
    
    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing the stored time
    //
    
    // Returns the current value of the real-time clock
    time_t getTime();
    
    // Sets the current value of the real-time clock
    void setTime(time_t t);
    
    
    //
    // Accessing registers
    //
    
public:
    
    // Reads one of the 16 RTC registers
    u8 peek(unsigned nr);

    // Writes one of the 16 RTC registers
    void poke(unsigned nr, u8 value);

private:

    // Reads one of the three control registers
    u8 peekD() { return reg[0][0xD]; };
    u8 peekE() { return config.model == RTC_RICOH ? 0 : reg[0][0xE]; }
    u8 peekF() { return config.model == RTC_RICOH ? 0 : reg[0][0xF]; }

    // Writes one of the three control registers
    void pokeD(u8 value) { reg[0][0xD] = value; }
    void pokeE(u8 value) { reg[0][0xE] = value; }
    void pokeF(u8 value) { reg[0][0xF] = value; }

    /* Returns the currently selected register bank. The Ricoh clock comprises
     * four register banks. A bank is selected by by bits 0 and 1 in control
     * register D. The OKI clock has a single bank, only.
     */
    int bank() { return config.model == RTC_RICOH ? (reg[0][0xD] & 0b11) : 0; }
    
    /* Converts the register value to the internally stored time-stamp. This
     * function has to be called *before* a RTC register is *read*.
     */
    void time2registers();
    void time2registersOki(tm *t);
    void time2registersRicoh(tm *t);

    /* Converts the internally stored time-stamp to register values. This
     * function has to be called *after* a RTC register is *written*.
     */
    void registers2time();
    void registers2timeOki(tm *t);
    void registers2timeRicoh(tm *t);
};

#endif
