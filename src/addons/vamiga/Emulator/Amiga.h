// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

// General
#include "AmigaComponent.h"
#include "Serialization.h"
#include "MsgQueue.h"

// Sub components
#include "Agnus.h"
#include "Blitter.h"
#include "ControlPort.h"
#include "CIA.h"
#include "Copper.h"
#include "CPU.h"
#include "Denise.h"
#include "Disk.h"
#include "Drive.h"
#include "Joystick.h"
#include "Keyboard.h"
#include "Memory.h"
#include "Moira.h"
#include "Mouse.h"
#include "Oscillator.h"
#include "Paula.h"
#include "RTC.h"
#include "SerialPort.h"
#include "ZorroManager.h"

// File types
#include "ADFFile.h"
#include "BootBlockImage.h"
#include "DMSFile.h"
#include "EXEFile.h"
#include "ExtendedRomFile.h"
#include "EXTFile.h"
#include "Folder.h"
#include "FSDevice.h"
#include "HDFFile.h"
#include "IMGFile.h"
#include "RomFile.h"
#include "Snapshot.h"

void threadTerminated(void *thisAmiga);
void *threadMain(void *thisAmiga);

/* A complete virtual Amiga. This class is the most prominent one of all. To
 * run the emulator, it is sufficient to create a single object of this type.
 * All subcomponents are created automatically. The public API gives you
 * control over the emulator's behaviour such as running and pausing the
 * emulation. Please note that most subcomponents have their own public API.
 * E.g., to query information from Paula, you need to invoke a public method on
 * amiga.paula.
 */
class Amiga : public HardwareComponent {

    /* The inspection target. In order to update the GUI periodically, the
     * emulator schedules this event in the inspector slot (INS_SLOT in the
     * secondary table) on a periodic basis. If the event is EVENT_NONE, no
     * action is taken. If an INS_xxx event is scheduled, inspect() is called
     * on a certain Amiga component.
     */
    EventID inspectionTarget = INS_NONE;

    // Result of the latest inspection
    AmigaInfo info;

     
    //
    // Sub components
    //
    
public:
    
    // Core components
    CPU cpu = CPU(*this);
    CIAA ciaA = CIAA(*this);
    CIAB ciaB = CIAB(*this);
    Memory mem = Memory(*this);
    Agnus agnus = Agnus(*this);
    Denise denise = Denise(*this);
    Paula paula = Paula(*this);
    
    // Logic board
    Oscillator oscillator = Oscillator(*this);
    RTC rtc = RTC(*this);
    ZorroManager zorro = ZorroManager(*this);
    
    // Ports
    ControlPort controlPort1 = ControlPort(*this, PORT_1);
    ControlPort controlPort2 = ControlPort(*this, PORT_2);
    SerialPort serialPort = SerialPort(*this);

    // Peripherals
    Keyboard keyboard = Keyboard(*this);

    // Floppy drives
    Drive df0 = Drive(*this, 0);
    Drive df1 = Drive(*this, 1);
    Drive df2 = Drive(*this, 2);
    Drive df3 = Drive(*this, 3);
    
    // Shortcuts to all four drives
    Drive *df[4] = { &df0, &df1, &df2, &df3 };
    
    //
    // Message queue
    //
    
    /* Communication channel to the GUI. The GUI registers a listener and a
     * callback function to retrieve messages.
     */
    MsgQueue queue;

    
    //
    // Emulator thread
    //
    
private:
    
    /* Run loop control. This variable is checked at the end of each runloop
     * iteration. Most of the time, the variable is 0 which causes the runloop
     * to repeat. A value greater than 0 means that one or more runloop control
     * flags are set. These flags are flags processed and the loop either
     * repeats or terminates depending on the provided flags.
     */
    u32 runLoopCtrl = 0;
    
    // The invocation counter for implementing suspend() / resume()
    isize suspendCounter = 0;
    
    // The emulator thread
    pthread_t p = (pthread_t)0;
    
    /* Mutex to coordinate the ownership of the emulator thread.
     *
     */
    pthread_mutex_t threadLock;
    
    /* Lock to synchronize the access to all state changing methods such as
     * run(), pause(), etc.
     */
    pthread_mutex_t stateChangeLock;
    
    
    //
    // Snapshot storage
    //
    
private:
    
    Snapshot *autoSnapshot = nullptr;
    Snapshot *userSnapshot = nullptr;

    
    //
    // Initializing
    //
    
public:
    
    Amiga();
    ~Amiga();

    const char *getDescription() const override { return "Amiga"; }

    void prefix() const override;

    void reset(bool hard);
    void hardReset() { reset(true); }
    void softReset() { reset(false); }

private:
    
    void _reset(bool hard) override;

    
    //
    // Configuring
    //
    
public:
        
    // Gets a single configuration item
    long getConfigItem(Option option) const;
    long getConfigItem(Option option, long id) const;
    
    // Sets a single configuration item
    bool configure(Option option, long value);
    bool configure(Option option, long id, long value);
    
    
    //
    // Analyzing
    //
    
public:
    
    AmigaInfo getInfo() { return HardwareComponent::getInfo(info); }
    
    EventID getInspectionTarget() const;
    void setInspectionTarget(EventID id);
    
private:
    
    void _inspect() override;
    void _dump() const override;
    
    
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
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Controlling
    //
    
public:
    
    void powerOn();
    void powerOff();
    void run();
    void pause();
    
    void setWarp(bool enable);
    bool inWarpMode() { return warpMode; }
    void enableWarpMode() { setWarp(true); }
    void disableWarpMode() { setWarp(false); }

    void setDebug(bool enable);
    bool inDebugMode() { return debugMode; }
    void enableDebugMode() { setDebug(true); }
    void disableDebugMode() { setDebug(false); }

private:
    
    void _powerOn() override;
    void _powerOff() override;
    void _run() override;
    void _pause() override;
    void _setWarp(bool enable) override;

    
    //
    // Working with the emulator thread
    //
    
public:
    
    /* Requests the emulator thread to stop and locks the threadLock. The
     * function is called in all state changing methods to obtain ownership
     * of the emulator thread. After returning, the emulator is either powered
     * off (if it was powered off before) or paused (if it was running before).
     */
    void acquireThreadLock();
    
    /* Returns true if a call to powerOn() will be successful. It returns false,
     * e.g., if no Kickstart Rom or Boot Rom is installed.
     */
    bool isReady(ErrorCode *error = nullptr);
    
    /* Pauses the emulation thread temporarily. Because the emulator is running
     * in a separate thread, the GUI has to pause the emulator before changing
     * it's internal state. This is done by embedding the code inside a
     * suspend / resume block:
     *
     *            suspend();
     *            do something with the internal state;
     *            resume();
     *
     *  It it safe to nest multiple suspend() / resume() blocks.
     */
    void suspend();
    void resume();
    
    /* Sets or clears a run loop control flag. The functions are thread-safe
     * and can be called from inside or outside the emulator thread.
     */
    void setControlFlags(u32 flags);
    void clearControlFlags(u32 flags);
    
    // Convenience wrappers for controlling the run loop
    void signalAutoSnapshot() { setControlFlags(RL_AUTO_SNAPSHOT); }
    void signalUserSnapshot() { setControlFlags(RL_USER_SNAPSHOT); }
    void signalInspect() { setControlFlags(RL_INSPECT); }
    void signalStop() { setControlFlags(RL_STOP); }


    //
    // Running the emulator
    //
    
public:
    
    // Runs or pauses the emulator
    void stopAndGo();
    
    /* Executes a single instruction. This function is used for single-stepping
     * through the code inside the debugger. It starts the execution thread and
     * terminates it after the next instruction has been executed.
     */
    void stepInto();
    
    /* Runs the emulator until the instruction following the current one is
     * reached. This function is used for single-stepping through the code
     * inside the debugger. It sets a soft breakpoint to PC+n where n is the
     * length bytes of the current instruction and starts the emulator thread.
     */
    void stepOver();
    
    /* The thread enter function. This (private) method is invoked when the
     * emulator thread launches. It has to be declared public to make it
     * accessible by the emulator thread.
     */
    void threadWillStart();
    
    /* The thread exit function. This (private) method is invoked when the
     * emulator thread terminates. It has to be declared public to make it
     * accessible by the emulator thread.
     */
    void threadDidTerminate();
    
    /* The Amiga run loop. This function is one of the most prominent ones. It
     * implements the outermost loop of the emulator and therefore the place
     * where emulation starts. If you want to understand how the emulator works,
     * this function should be your starting point.
     */
    void runLoop();

    
    //
    // Handling snapshots
    //
    
public:
    
    /* Requests a snapshot to be taken. Once the snapshot is ready, a message
     * is written into the message queue. The snapshot can then be picked up by
     * calling latestAutoSnapshot() or latestUserSnapshot(), depending on the
     * requested snapshot type.
     */
    void requestAutoSnapshot();
    void requestUserSnapshot();
     
    // Returns the most recent snapshot or nullptr if none was taken
    Snapshot *latestAutoSnapshot();
    Snapshot *latestUserSnapshot();

    /* Loads the current state from a snapshot file. There is an thread-unsafe
     * and thread-safe version of this function. The first one can be unsed
     * inside the emulator thread or from outside if the emulator is halted.
     * The second one can be called any time.
     */
    void loadFromSnapshotUnsafe(Snapshot *snapshot);
    void loadFromSnapshotSafe(Snapshot *snapshot);
};
