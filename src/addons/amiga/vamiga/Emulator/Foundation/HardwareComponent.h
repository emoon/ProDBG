// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _HARDWARE_COMPONENT_H
#define _HARDWARE_COMPONENT_H

#include "AmigaObject.h"
#include "Serialization.h"

/* This class defines the base functionality of all hardware components. It
 * comprises functions for initializing, configuring, and serializing the
 * emulator, as well as functions for powering up and down, running and pausing.
 */
class HardwareComponent : public AmigaObject {
    
public:
    
    // Sub components
    vector<HardwareComponent *> subComponents;
    
protected:
    
    /* State model. The virtual hardware components can be in three different
     * states called 'Off', 'Paused', and 'Running':
     *
     *        Off: The Amiga is turned off
     *     Paused: The Amiga is turned on, but there is no emulator thread
     *    Running: The Amiga is turned on and the emulator thread running
     */
    EmulatorState state = STATE_OFF;
    
    /* Indicates if the emulator should be executed in warp mode. To speed up
     * emulation (e.g., during disk accesses), the virtual hardware may be put
     * into warp mode. In this mode, the emulation thread is no longer paused
     * to match the target frequency and runs as fast as possible.
     */
    bool warpMode = false;
    
    /* Indicates if the emulator should be executed in debug mode. Debug mode
     * is enabled when the GUI debugger is opend and disabled when the GUI
     * debugger is closed. In debug mode, several time-consuming tasks are
     * performed that are usually left out. E.g., the CPU checks for
     * breakpoints and records the executed instruction in it's trace buffer.
     */
    bool debugMode = false;
        
    
    //
    // Initializing
    //
    
public:
    
    virtual ~HardwareComponent();
    
    /* Initializes the component and it's subcomponents. The initialization
     * procedure is initiated exactly once, in the constructor of the Amiga
     * class. Some subcomponents implement the delegation method _initialize()
     * to finalize their initialization, e.g., by setting up referecens that
     * did not exist when they were constructed.
     */
    void initialize();
    virtual void _initialize() { };
    
    /* Resets the component and it's subcomponents. Two reset modes are
     * distinguished:
     *
     *     hard: A hard reset restores the initial state. It resets the Amiga
     *           from an emulator point of view.
     *
     *     soft: A soft reset emulates a reset inside the virtual Amiga. It is
     *           used to emulate the RESET instruction of the CPU.
     */
    void reset(bool hard);
    virtual void _reset(bool hard) = 0;
    
    
    //
    // Configuring
    //
    
    /* Configures the component and it's subcomponents. This function
     * distributes a configuration request to all subcomponents by calling
     * setConfigItem(). The function returns true iff the current configuration
     * has changed.
     */
    bool configure(ConfigOption option, long value);
    bool configure(unsigned dfn, ConfigOption option, long value);
    
    /* Requests the change of a single configuration item. Each sub-component
     * checks if it is responsible for the requested configuration item. If
     * yes, it changes the internal state. If no, it ignores the request.
     * The function returns true iff the current configuration has changed.
     */
    virtual bool setConfigItem(ConfigOption option, long value) { return false; }
    virtual bool setConfigItem(unsigned dfn, ConfigOption option, long value) { return false; }
    
    // Dumps debug information about the current configuration to the console
    void dumpConfig();
    virtual void _dumpConfig() { }
    
    
    //
    // Analyzing
    //
    
    /* Collects information about the component and it's subcomponents. Many
     * components contain an info variable of a class specific type (e.g.,
     * CPUInfo, MemoryInfo, ...). These variables contain the information shown
     * in the GUI's inspector window and are updated by calling this function.
     * Note: Because this function accesses the internal emulator state with
     * many non-atomic operations, it must not be called on a running emulator.
     * To carry out inspections while the emulator is running, set up an
     * inspection target via Amiga::setInspectionTarget().
     */
    void inspect();
    virtual void _inspect() { }
    
    /* Base method for building the class specific getInfo() methods. When the
     * emulator is running, the result of the most recent inspection is
     * returned. If the emulator isn't running, the function first updates the
     * cached values in order to return up-to-date results.
     */
    template<class T> T getInfo(T &cachedValues) {
        
        if (!isRunning()) inspect();
        
        T result;
        synchronized { result = cachedValues; }
        return result;
    }
    
    // Dumps debug information about the internal state to the console
    void dump();
    virtual void _dump() { }
    
    
    //
    // Serializing
    //
    
    // Returns the size of the internal state in bytes
    size_t size();
    virtual size_t _size() = 0;
    
    // Loads the internal state from a memory buffer
    size_t load(u8 *buffer);
    virtual size_t _load(u8 *buffer) = 0;
    
    // Saves the internal state to a memory buffer
    size_t save(u8 *buffer);
    virtual size_t _save(u8 *buffer) = 0;
    
    /* Delegation methods called inside load() or save(). Some components
     * override these methods to add custom behavior if not all elements can be
     * processed by the default implementation.
     */
    virtual size_t willLoadFromBuffer(u8 *buffer) { return 0; }
    virtual size_t didLoadFromBuffer(u8 *buffer) { return 0; }
    virtual size_t willSaveToBuffer(u8 *buffer) {return 0; }
    virtual size_t didSaveToBuffer(u8 *buffer) { return 0; }
    
    
    //
    // Controlling
    //
    
public:
    
    /* State model. At any time, a component is in one of three states:
     *
     *          -----------------------------------------------
     *         |                     run()                     |
     *         |                                               V
     *     ---------   powerOn()   ---------     run()     ---------
     *    |   Off   |------------>| Paused  |------------>| Running |
     *    |         |<------------|         |<------------|         |
     *     ---------   powerOff()  ---------    pause()    ---------
     *         ^                                               |
     *         |                   powerOff()                  |
     *          -----------------------------------------------
     *
     *     isPoweredOff()         isPaused()          isRunning()
     * |-------------------||-------------------||-------------------|
     *                      |----------------------------------------|
     *                                     isPoweredOn()
     *
     * Additional component flags: warp (on / off), debug (on / off)
     */
    
    bool isPoweredOff() { return state == STATE_OFF; }
    bool isPoweredOn() { return state != STATE_OFF; }
    bool isPaused() { return state == STATE_PAUSED; }
    bool isRunning() { return state == STATE_RUNNING; }
    
protected:
    
    /* powerOn() powers the component on.
     *
     * current   | next      | action
     * ------------------------------------------------------------------------
     * off       | paused    | _powerOn() on each subcomponent
     * paused    | paused    | none
     * running   | running   | none
     */
    void powerOn();
    virtual void _powerOn() { }
    
    /* powerOff() powers the component off.
     *
     * current   | next      | action
     * ------------------------------------------------------------------------
     * off       | off       | none
     * paused    | off       | _powerOff() on each subcomponent
     * running   | off       | pause(), _powerOff() on each subcomponent
     */
    void powerOff();
    virtual void _powerOff() { }
    
    /* run() puts the component in 'running' state.
     *
     * current   | next      | action
     * ------------------------------------------------------------------------
     * off       | running   | powerOn(), _run() on each subcomponent
     * paused    | running   | _run() on each subcomponent
     * running   | running   | none
     */
    void run();
    virtual void _run() { }
    
    /* pause() puts the component in 'paused' state.
     *
     * current   | next      | action
     * ------------------------------------------------------------------------
     * off       | off       | none
     * paused    | paused    | none
     * running   | paused    | _pause() on each subcomponent
     */
    void pause();
    virtual void _pause() { };
        
    // Switches warp mode on or off
    void setWarp(bool enable);
    virtual void _setWarp(bool enable) { };
    
    // Switches debug mode on or off
    void setDebug(bool enable);
    virtual void _setDebug(bool enable) { };
};

//
// Standard implementations of _reset, _load, and _save
//

#define COMPUTE_SNAPSHOT_SIZE \
SerCounter counter; \
applyToPersistentItems(counter); \
applyToHardResetItems(counter); \
applyToResetItems(counter); \
return counter.count;

#define RESET_SNAPSHOT_ITEMS(hard) \
{ \
SerResetter resetter; \
if (hard) applyToHardResetItems(resetter); \
applyToResetItems(resetter); \
debug(SNP_DEBUG, "Resetted (%s)\n", hard ? "hard" : "soft"); \
}

#define LOAD_SNAPSHOT_ITEMS \
{ \
SerReader reader(buffer); \
applyToPersistentItems(reader); \
applyToHardResetItems(reader); \
applyToResetItems(reader); \
debug(SNP_DEBUG, "Recreated from %d bytes\n", reader.ptr - buffer); \
return reader.ptr - buffer; \
}

#define SAVE_SNAPSHOT_ITEMS \
{ \
SerWriter writer(buffer); \
applyToPersistentItems(writer); \
applyToHardResetItems(writer); \
applyToResetItems(writer); \
debug(SNP_DEBUG, "Serialized to %d bytes\n", writer.ptr - buffer); \
return writer.ptr - buffer; \
}

#endif
