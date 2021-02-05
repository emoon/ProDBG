// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

//
// Emulator thread
//

void
threadTerminated(void *thisAmiga)
{
    assert(thisAmiga != nullptr);
    
    // Inform the Amiga that the thread has been canceled
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadDidTerminate();
}

void
*threadMain(void *thisAmiga) {
    
    assert(thisAmiga != nullptr);
    
    // Inform the Amiga that the thread is about to start
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadWillStart();
    
    // Configure the thread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, nullptr);
    pthread_cleanup_push(threadTerminated, thisAmiga);
    
    // Enter the run loop
    amiga->runLoop();
    
    // Clean up and exit
    pthread_cleanup_pop(1);
    pthread_exit(nullptr);
}


//
// Amiga Class
//

Amiga::Amiga()
{
    /* The order of subcomponents is important here, because some components
     * are dependend on others during initialization. I.e.,
     *
     * - The control ports, the serial Controller, the disk controller, and the
     *   disk drives must preceed the CIAs, because the CIA port values depend
     *   on these devices.
     *
     * - The CIAs must preceed memory, because they determine if the lower
     *   memory banks are overlayed by Rom.
     *
     * - Memory mus preceed the CPU, because it contains the CPU reset vector.
     */

    subComponents = vector<HardwareComponent *> {

        &oscillator,
        &agnus,
        &rtc,
        &denise,
        &paula,
        &zorro,
        &controlPort1,
        &controlPort2,
        &serialPort,
        &keyboard,
        &df0,
        &df1,
        &df2,
        &df3,
        &ciaA,
        &ciaB,
        &mem,
        &cpu,
        &queue
    };

    // Set up the initial state
    initialize();
    hardReset();

    // Initialize the mach timer info
    // mach_timebase_info(&tb);
    
    // Initialize mutex
    pthread_mutex_init(&threadLock, nullptr);
    pthread_mutex_init(&stateChangeLock, nullptr);
    
    // Print some debug information
    if (SNP_DEBUG) {
        
        msg("             Agnus : %lu bytes\n", sizeof(Agnus));
        msg("       AudioFilter : %lu bytes\n", sizeof(AudioFilter));
        msg("       AudioStream : %lu bytes\n", sizeof(AudioStream));
        msg("               CIA : %lu bytes\n", sizeof(CIA));
        msg("       ControlPort : %lu bytes\n", sizeof(ControlPort));
        msg("               CPU : %lu bytes\n", sizeof(CPU));
        msg("            Denise : %lu bytes\n", sizeof(Denise));
        msg("             Drive : %lu bytes\n", sizeof(Drive));
        msg("          Keyboard : %lu bytes\n", sizeof(Keyboard));
        msg("            Memory : %lu bytes\n", sizeof(Memory));
        msg("moira::Breakpoints : %lu bytes\n", sizeof(moira::Breakpoints));
        msg("moira::Watchpoints : %lu bytes\n", sizeof(moira::Watchpoints));
        msg("   moira::Debugger : %lu bytes\n", sizeof(moira::Debugger));
        msg("      moira::Moira : %lu bytes\n", sizeof(moira::Moira));
        msg("             Muxer : %lu bytes\n", sizeof(Muxer));
        msg("        Oscillator : %lu bytes\n", sizeof(Oscillator));
        msg("             Paula : %lu bytes\n", sizeof(Paula));
        msg("       PixelEngine : %lu bytes\n", sizeof(PixelEngine));
        msg("               RTC : %lu bytes\n", sizeof(RTC));
        msg("           Sampler : %lu bytes\n", sizeof(Sampler));
        msg("    ScreenRecorder : %lu bytes\n", sizeof(ScreenRecorder));
        msg("        SerialPort : %lu bytes\n", sizeof(SerialPort));
        msg("            Volume : %lu bytes\n", sizeof(Volume));
        msg("             Zorro : %lu bytes\n", sizeof(ZorroManager));
        msg("\n");
    }
}

Amiga::~Amiga()
{
    debug(RUN_DEBUG, "Destroying Amiga[%p]\n", this);
    powerOff();
    
    pthread_mutex_destroy(&threadLock);
    pthread_mutex_destroy(&stateChangeLock);
}

void
Amiga::prefix() const
{
    fprintf(stderr, "[%lld] (%3d,%3d) ",
            agnus.frame.nr, agnus.pos.v, agnus.pos.h);

    fprintf(stderr, "%06X ", cpu.getPC0());
    fprintf(stderr, "%2X ", cpu.getIPL());

    u16 dmacon = agnus.dmacon;
    bool dmaen = dmacon & DMAEN;
    fprintf(stderr, "%c%c%c%c%c%c ",
            (dmacon & BPLEN) ? (dmaen ? 'B' : 'B') : '-',
            (dmacon & COPEN) ? (dmaen ? 'C' : 'c') : '-',
            (dmacon & BLTEN) ? (dmaen ? 'B' : 'b') : '-',
            (dmacon & SPREN) ? (dmaen ? 'S' : 's') : '-',
            (dmacon & DSKEN) ? (dmaen ? 'D' : 'd') : '-',
            (dmacon & AUDEN) ? (dmaen ? 'A' : 'a') : '-');

    fprintf(stderr, "%04X %04X ", paula.intena, paula.intreq);

    if (agnus.copper.servicing) {
        fprintf(stderr, "[%06X] ", agnus.copper.getCopPC());
    }
}

void
Amiga::reset(bool hard)
{
    if (hard) suspend();
    
    // If a disk change is in progress, finish it
    paula.diskController.serviceDiskChangeEvent();
    
    // Execute the standard reset routine
    HardwareComponent::reset(hard);
    
    if (hard) resume();

    // Inform the GUI
    if (hard) queue.put(MSG_RESET);
}

void
Amiga::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    // Clear all runloop flags
    runLoopCtrl = 0;
}

long
Amiga::getConfigItem(Option option) const
{
    switch (option) {

        case OPT_AGNUS_REVISION:
        case OPT_SLOW_RAM_MIRROR:
            return agnus.getConfigItem(option);
            
        case OPT_DENISE_REVISION:
        case OPT_BRDRBLNK:
        case OPT_HIDDEN_SPRITES:
        case OPT_HIDDEN_LAYERS:
        case OPT_HIDDEN_LAYER_ALPHA:
        case OPT_CLX_SPR_SPR:
        case OPT_CLX_SPR_PLF:
        case OPT_CLX_PLF_PLF:
            return denise.getConfigItem(option);
            
        case OPT_RTC_MODEL:
            return rtc.getConfigItem(option);

        case OPT_CHIP_RAM:
        case OPT_SLOW_RAM:
        case OPT_FAST_RAM:
        case OPT_EXT_START:
        case OPT_SLOW_RAM_DELAY:
        case OPT_BANKMAP:
        case OPT_UNMAPPING_TYPE:
        case OPT_RAM_INIT_PATTERN:
            return mem.getConfigItem(option);
            
        case OPT_SAMPLING_METHOD:
        case OPT_FILTER_TYPE:
        case OPT_FILTER_ALWAYS_ON:
        case OPT_AUDVOLL:
        case OPT_AUDVOLR:
            return paula.muxer.getConfigItem(option);

        case OPT_BLITTER_ACCURACY:
            return agnus.blitter.getConfigItem(option);

        case OPT_DRIVE_SPEED:
        case OPT_LOCK_DSKSYNC:
        case OPT_AUTO_DSKSYNC:
            return paula.diskController.getConfigItem(option);
            
        case OPT_SERIAL_DEVICE:
            return serialPort.getConfigItem(option);

        case OPT_CIA_REVISION: 
        case OPT_TODBUG:
        case OPT_ECLOCK_SYNCING:
            return ciaA.getConfigItem(option);

        case OPT_ACCURATE_KEYBOARD:
            return keyboard.getConfigItem(option);

        default: assert(false); return 0;
    }
}

long
Amiga::getConfigItem(Option option, long id) const
{
    switch (option) {
            
        case OPT_AUDPAN:
        case OPT_AUDVOL:
            return paula.muxer.getConfigItem(option, id);

        case OPT_DRIVE_CONNECT:
            return paula.diskController.getConfigItem(option, id);
            
        case OPT_DRIVE_TYPE:
        case OPT_EMULATE_MECHANICS:
            return df[id]->getConfigItem(option);
            
        default: assert(false);
    }
    
    return 0;
}

bool
Amiga::configure(Option option, long value)
{
    // Propagate configuration request to all components
    bool changed = HardwareComponent::configure(option, value);
    
    // Inform the GUI if the configuration has changed
    if (changed) queue.put(MSG_CONFIG);
    
    // Dump the current configuration in debugging mode
    if (changed && CNF_DEBUG) dumpConfig();

    return changed;
}

bool
Amiga::configure(Option option, long id, long value)
{
    // Propagate configuration request to all components
    bool changed = HardwareComponent::configure(option, id, value);
    
    // Inform the GUI if the configuration has changed
    if (changed) queue.put(MSG_CONFIG);
    
    // Dump the current configuration in debugging mode
    if (changed && CNF_DEBUG) dumpConfig();
        
    return changed;
}

EventID
Amiga::getInspectionTarget() const
{
    return agnus.slot[SLOT_INS].id;
}

void
Amiga::setInspectionTarget(EventID id)
{
    suspend();
    inspectionTarget = id;
    agnus.scheduleRel<SLOT_INS>(0, inspectionTarget);
    agnus.serviceINSEvent();
    resume();
}

void
Amiga::_inspect()
{
    synchronized {
        
        info.cpuClock = cpu.getMasterClock();
        info.dmaClock = agnus.clock;
        info.ciaAClock = ciaA.clock;
        info.ciaBClock = ciaB.clock;
        info.frame = agnus.frame.nr;
        info.vpos = agnus.pos.v;
        info.hpos = agnus.pos.h;
    }
}

void
Amiga::_dump() const
{
    msg("    poweredOn: %s\n", isPoweredOn() ? "yes" : "no");
    msg("   poweredOff: %s\n", isPoweredOff() ? "yes" : "no");
    msg("       paused: %s\n", isPaused() ? "yes" : "no");
    msg("      running: %s\n", isRunning() ? "yes" : "no");
    msg("         warp: %s\n", warpMode ? "on" : "off");
    msg("\n");
}

void
Amiga::powerOn()
{
    debug(RUN_DEBUG, "powerOn()\n");
    
    #ifdef DF0_DISK
    DiskFile *df0file = AmigaFile::make <ADFFile> (DF0_DISK);
    if (df0file) {
        Disk *disk = Disk::makeWithFile(df0file);
        df0.ejectDisk();
        df0.insertDisk(disk);
        df0.setWriteProtection(false);
    }
    #endif

    #ifdef DF1_DISK
        DiskFile *df1file = DiskFile::makeWithFile(DF1_DISK);
        if (df1file) {
            Disk *disk = Disk::makeWithFile(df1file);
            df1.ejectDisk();
            df1.insertDisk(disk);
            df1.setWriteProtection(false);
        }
    #endif
    
    #ifdef INITIAL_BREAKPOINT
        debugMode = true;
        cpu.debugger.breakpoints.addAt(INITIAL_BREAKPOINT);
    #endif
    
    pthread_mutex_lock(&stateChangeLock);
        
    if (!isPoweredOn() && isReady()) {
        
        acquireThreadLock();
        HardwareComponent::powerOn();
    }
    
    pthread_mutex_unlock(&stateChangeLock);
}

void
Amiga::_powerOn()
{
    debug(RUN_DEBUG, "_powerOn()\n");

    // Clear all runloop flags
    // runLoopCtrl = 0;

    // Update the recorded debug information
    inspect();

    queue.put(MSG_POWER_ON);
}

void
Amiga::powerOff()
{
    debug(RUN_DEBUG, "powerOff()\n");
    
    pthread_mutex_lock(&stateChangeLock);
    
    if (!isPoweredOff()) {
        
        acquireThreadLock();
        HardwareComponent::powerOff();
    }
    
    pthread_mutex_unlock(&stateChangeLock);
}

void
Amiga::_powerOff()
{
    debug(RUN_DEBUG, "_powerOff()\n");
    
    // Update the recorded debug information
    inspect();
    
    queue.put(MSG_POWER_OFF);
}

void
Amiga::run()
{
    debug(RUN_DEBUG, "run()\n");
        
    pthread_mutex_lock(&stateChangeLock);
    
    if (!isRunning() && isReady()) {
        
        acquireThreadLock();
        HardwareComponent::run();
    }
    
    pthread_mutex_unlock(&stateChangeLock);
}

void
Amiga::_run()
{
    debug(RUN_DEBUG, "_run()\n");
    
    // Start the emulator thread
    pthread_create(&p, nullptr, threadMain, (void *)this);
    
    // Inform the GUI
    queue.put(MSG_RUN);
}

void
Amiga::pause()
{
    debug(RUN_DEBUG, "pause()\n");

    pthread_mutex_lock(&stateChangeLock);
    
    if (!isPaused()) {
        
        acquireThreadLock();
        HardwareComponent::pause();
    }
    
    pthread_mutex_unlock(&stateChangeLock);
}

void
Amiga::_pause()
{
    debug(RUN_DEBUG, "_pause()\n");
    
    // When we reach this line, the emulator thread is already gone
    assert(p == (pthread_t)0);
    
    // Update the recorded debug information
    inspect();

    // Inform the GUI
    queue.put(MSG_PAUSE);
}

void
Amiga::setWarp(bool enable)
{
    suspend();
    HardwareComponent::setWarp(enable);
    resume();
}

void
Amiga::_setWarp(bool enable)
{
    if (enable) {
        
        queue.put(MSG_WARP_ON);
        
    } else {
        
        oscillator.restart();
        queue.put(MSG_WARP_OFF);
    }
}

void
Amiga::setDebug(bool enable)
{
    HardwareComponent::setDebug(enable);
}

void
Amiga::acquireThreadLock()
{
    if (state == EMULATOR_STATE_RUNNING) {
        
        // Assure the emulator thread exists
        assert(p != (pthread_t)0);
        
        // Free the thread lock by terminating the thread
        signalStop();
        
    } else {
        
        // There must be no emulator thread
        assert(p == (pthread_t)0);
        
        // It's save to free the lock immediately
        pthread_mutex_unlock(&threadLock);
    }
    
    // Acquire the lock
    pthread_mutex_lock(&threadLock);
}

bool
Amiga::isReady(ErrorCode *error)
{
    if (!mem.hasRom()) {
        msg("isReady: No Boot Rom or Kickstart Rom found\n");
        if (error) *error = ERROR_ROM_MISSING;
        return false;
    }

    if (!mem.hasChipRam()) {
        msg("isReady: No Chip Ram found\n");
        if (error) *error = ERROR_ROM_MISSING;
        return false;
    }
    
    if (mem.hasArosRom()) {

        if (!mem.hasExt()) {
            msg("isReady: Aros requires an extension Rom\n");
            if (error) *error = ERROR_AROS_NO_EXTROM;
            return false;
        }

        if (mem.ramSize() < MB(1)) {
            msg("isReady: Aros requires at least 1 MB of memory\n");
            if (error) *error = ERROR_AROS_RAM_LIMIT;
            return false;
        }
    }

    if (mem.chipRamSize() > KB(agnus.chipRamLimit())) {
        msg("isReady: Chip Ram exceeds Agnus limit\n");
        if (error) *error = ERROR_CHIP_RAM_LIMIT;
        return false;
    }

    return true;
}

void
Amiga::suspend()
{
    pthread_mutex_lock(&stateChangeLock);
    
    debug(RUN_DEBUG, "Suspending (%zu)...\n", suspendCounter);
    
    if (suspendCounter || isRunning()) {
        
        acquireThreadLock();
        assert(!isRunning()); // At this point, the emulator is already paused
                
        suspendCounter++;
    }
    
    pthread_mutex_unlock(&stateChangeLock);
}

void
Amiga::resume()
{
    pthread_mutex_lock(&stateChangeLock);
    
    debug(RUN_DEBUG, "Resuming (%zu)...\n", suspendCounter);
    
    if (suspendCounter && --suspendCounter == 0) {
        
        acquireThreadLock();
        HardwareComponent::run();
    }
    
    pthread_mutex_unlock(&stateChangeLock);
}

void
Amiga::setControlFlags(u32 flags)
{
    synchronized { runLoopCtrl |= flags; }
}

void
Amiga::clearControlFlags(u32 flags)
{
    synchronized { runLoopCtrl &= ~flags; }
}

void
Amiga::stopAndGo()
{
    isRunning() ? pause() : run();
}

void
Amiga::stepInto()
{
    if (isRunning()) return;

    cpu.debugger.stepInto();
    run();
}

void
Amiga::stepOver()
{
    if (isRunning()) return;
    
    cpu.debugger.stepOver();
    run();
}

void
Amiga::threadWillStart()
{
    debug(RUN_DEBUG, "Emulator thread started\n");
}

void
Amiga::threadDidTerminate()
{
    debug(RUN_DEBUG, "Emulator thread terminated\n");

    // Trash the thread pointer
    p = (pthread_t)0;
    
    // Pause all components
    HardwareComponent::pause();
        
    // Release the thread lock
    pthread_mutex_unlock(&threadLock);
}

void
Amiga::runLoop()
{
    debug(RUN_DEBUG, "runLoop()\n");

    // Prepare to run
    oscillator.restart();
    
    // Enable or disable debugging features
    if (debugMode) {
        cpu.debugger.enableLogging();
    } else {
        cpu.debugger.disableLogging();
    }
    agnus.scheduleRel<SLOT_INS>(0, inspectionTarget);
    
    // Enter the loop
    while(1) {
        
        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (runLoopCtrl) {
            
            // Are we requested to take a snapshot?
            if (runLoopCtrl & RL_AUTO_SNAPSHOT) {
                debug(RUN_DEBUG, "RL_AUTO_SNAPSHOT\n");
                autoSnapshot = Snapshot::makeWithAmiga(this);
                queue.put(MSG_AUTO_SNAPSHOT_TAKEN);
                clearControlFlags(RL_AUTO_SNAPSHOT);
            }
            if (runLoopCtrl & RL_USER_SNAPSHOT) {
                debug(RUN_DEBUG, "RL_USER_SNAPSHOT\n");
                userSnapshot = Snapshot::makeWithAmiga(this);
                queue.put(MSG_USER_SNAPSHOT_TAKEN);
                clearControlFlags(RL_USER_SNAPSHOT);
            }

            // Are we requested to update the debugger info structs?
            if (runLoopCtrl & RL_INSPECT) {
                debug(RUN_DEBUG, "RL_INSPECT\n");
                inspect();
                clearControlFlags(RL_INSPECT);
            }

            // Did we reach a breakpoint?
            if (runLoopCtrl & RL_BREAKPOINT_REACHED) {
                inspect();
                queue.put(MSG_BREAKPOINT_REACHED);
                debug(RUN_DEBUG, "BREAKPOINT_REACHED pc: %x\n", cpu.getPC());
                clearControlFlags(RL_BREAKPOINT_REACHED);
                break;
            }

            // Did we reach a watchpoint?
            if (runLoopCtrl & RL_WATCHPOINT_REACHED) {
                inspect();
                queue.put(MSG_WATCHPOINT_REACHED);
                debug(RUN_DEBUG, "WATCHPOINT_REACHED pc: %x\n", cpu.getPC());
                clearControlFlags(RL_WATCHPOINT_REACHED);
                break;
            }

            // Are we requested to terminate the run loop?
            if (runLoopCtrl & RL_STOP) {
                clearControlFlags(RL_STOP);
                debug(RUN_DEBUG, "RL_STOP\n");
                break;
            }
        }
    }
}

/*
void
Amiga::restartTimer()
{
    timeBase = time_in_nanos();
    clockBase = agnus.clock;
}
*/
/*
void
Amiga::synchronizeTiming()
{
    u64 now          = time_in_nanos();
    Cycle clockDelta = agnus.clock - clockBase;
    u64 elapsedTime  = (u64)(clockDelta * 1000 / masterClockFrequency);
    u64 targetTime   = timeBase + elapsedTime;
        
    // Check if we're running too slow ...
    if (now > targetTime) {
        
        // Check if we're completely out of sync ...
        if (now - targetTime > 200000000) {
            
            // warn("The emulator is way too slow (%lld).\n", now - targetTime);
            restartTimer();
            return;
        }
    }
    
    // Check if we're running too fast ...
    if (now < targetTime) {
        
        // Check if we're completely out of sync ...
        if (targetTime - now > 200000000) {
            
            warn("The emulator is way too fast (%lld).\n", targetTime - now);
            restartTimer();
            return;
        }
        
        // See you soon...
        mach_wait_until(targetTime);
    }
}
*/

void
Amiga::requestAutoSnapshot()
{
    if (!isRunning()) {

        // Take snapshot immediately
        autoSnapshot = Snapshot::makeWithAmiga(this);
        queue.put(MSG_AUTO_SNAPSHOT_TAKEN);
        
    } else {

        // Schedule the snapshot to be taken
        signalAutoSnapshot();
    }
}

void
Amiga::requestUserSnapshot()
{
    if (!isRunning()) {
        
        // Take snapshot immediately
        userSnapshot = Snapshot::makeWithAmiga(this);
        queue.put(MSG_USER_SNAPSHOT_TAKEN);
        
    } else {
        
        // Schedule the snapshot to be taken
        signalUserSnapshot();
    }
}

Snapshot *
Amiga::latestAutoSnapshot()
{
    Snapshot *result = autoSnapshot;
    autoSnapshot = nullptr;
    return result;
}

Snapshot *
Amiga::latestUserSnapshot()
{
    Snapshot *result = userSnapshot;
    userSnapshot = nullptr;
    return result;
}

void
Amiga::loadFromSnapshotUnsafe(Snapshot *snapshot)
{
    u8 *ptr;
    
    if (snapshot && (ptr = snapshot->getData())) {
        load(ptr);
        queue.put(MSG_SNAPSHOT_RESTORED);
    }
}

void
Amiga::loadFromSnapshotSafe(Snapshot *snapshot)
{
    trace(SNP_DEBUG, "loadFromSnapshotSafe\n");
    
    suspend();
    loadFromSnapshotUnsafe(snapshot);
    resume();
}
