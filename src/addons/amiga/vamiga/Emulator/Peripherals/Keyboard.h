// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "KeyboardTypes.h"
#include "AmigaComponent.h"
#include "Event.h"

class Keyboard : public AmigaComponent {

    // Current configuration
    KeyboardConfig config;

    // The current keyboard state
    KeyboardState state;
    
    // Shift register storing the transmission bits
    u8 shiftReg;
    
    /* Time stamps recording an Amiga triggered change of the SP line. The SP
     * line is driven by the Amiga to transmit a handshake.
     */
    Cycle spLow;
    Cycle spHigh;

    // The keycode type-ahead buffer. The Amiga can hold up to 10 keycodes.
    static const isize bufferSize = 10;
    u8 typeAheadBuffer[bufferSize];
    
    // Next free position in the type ahead buffer
    u8 bufferIndex;
    
    // Remebers the keys that are currently held down
    bool keyDown[128];

    
    //
    // Initialization
    //
    
public:
    
    Keyboard(Amiga& ref);
    
    const char *getDescription() const override { return "Keyboard"; }

private:
    
    void _reset(bool hard) override;

    
    //
    // Configuring
    //
    
public:
    
    const KeyboardConfig &getConfig() const { return config; }

    long getConfigItem(Option option) const;
    bool setConfigItem(Option option, long value) override;

        
    //
    // Analyzing
    //
    
private:
    
    void _dump(Dump::Category category, std::ostream& os) const override;

    
    //
    // Serialization
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker << config.accurate;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        << state
        << shiftReg
        << spLow
        << spHigh
        << typeAheadBuffer
        << bufferIndex;
    }

    
    //
    // Controlling
    //
    
private:
    
    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Pressing and releasing keys
    //
    
public:

    bool keyIsPressed(long keycode) const;
    void pressKey(long keycode);
    void releaseKey(long keycode);
    void releaseAllKeys();
    
    
    //
    // Managing the type-ahead buffer
    //

private:

    bool bufferIsEmpty() const { return bufferIndex == 0; }
    bool bufferIsFull() const { return bufferIndex == bufferSize; }

    // Reads a keycode from the type-ahead buffer
    u8 readFromBuffer();

    // Writes a keycode into the type-ahead buffer
    void writeToBuffer(u8 keycode);

    
    //
    // Talking to the Amiga
    //
    
public:

    /* Emulates a change on the SP line. This function is called whenever the
     * CIA switches the serial register from input mode to output mode or vice
     * versa. The SP line is controlled by the Amiga to signal a handshake.
     */
    void setSPLine(bool value, Cycle cycle);
    
    // Services a keyboard event
    void serviceKeyboardEvent(EventID id);

    
    //
    // Running the device
    //

private:

    // Processes a detected handshake
    void processHandshake();

    // Performs all actions according to the current state
    void execute();
    
    // Sends a keycode to the Amiga
    void sendKeyCode(u8 keyCode);

    // Sends a sync pulse to the Amiga
    void sendSyncPulse();
};
