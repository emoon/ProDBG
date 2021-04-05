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
    assert(thisAmiga != NULL);
    
    // Inform the Amiga that the thread has been canceled
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadDidTerminate();
}

void
*threadMain(void *thisAmiga) {
    
    assert(thisAmiga != NULL);
    
    // Inform the Amiga that the thread is about to start
    Amiga *amiga = (Amiga *)thisAmiga;
    amiga->threadWillStart();
    
    // Configure the thread
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    pthread_cleanup_push(threadTerminated, thisAmiga);
    
    // Enter the run loop
    amiga->runLoop();
    
    // Clean up and exit
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}


//
// Amiga Class
//

Amiga::Amiga()
{
    setDescription("Amiga");

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
    };

    // Set up the initial state
    initialize();
    hardReset();

    // Initialize the mach timer info
    // mach_timebase_info(&tb);
    
    // Initialize mutex
    pthread_mutex_init(&threadLock, NULL);
    pthread_mutex_init(&stateChangeLock, NULL);
}

Amiga::~Amiga()
{
    trace("Destroying Amiga[%p]\n", this);
    powerOff();
    
    pthread_mutex_destroy(&threadLock);
    pthread_mutex_destroy(&stateChangeLock);
}

void
Amiga::prefix()
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
    if (hard) messageQueue.put(MSG_RESET);
}

AmigaConfiguration
Amiga::getConfig()
{
    AmigaConfiguration config;

    config.ciaA = ciaA.getConfig();
    config.ciaB = ciaB.getConfig();
    config.rtc = rtc.getConfig();
    config.audio = paula.muxer.getConfig();
    config.mem = mem.getConfig();
    config.agnus = agnus.getConfig();
    config.denise = denise.getConfig();
    config.serialPort = serialPort.getConfig();
    config.keyboard = keyboard.getConfig();
    config.blitter = agnus.blitter.getConfig();
    config.diskController = paula.diskController.getConfig();
    config.df0 = df0.getConfig();
    config.df1 = df1.getConfig();
    config.df2 = df2.getConfig();
    config.df3 = df3.getConfig();

    // Assure both CIAs are configured equally
    assert(config.ciaA.revision == config.ciaB.revision);
    assert(config.ciaA.todBug == config.ciaB.todBug);
    assert(config.ciaA.eClockSyncing == config.ciaB.eClockSyncing);

    return config;
}

long
Amiga::getConfigItem(ConfigOption option)
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
        case OPT_AUDVOL0:
        case OPT_AUDVOL1:
        case OPT_AUDVOL2:
        case OPT_AUDVOL3:
        case OPT_AUDPAN0:
        case OPT_AUDPAN1:
        case OPT_AUDPAN2:
        case OPT_AUDPAN3:
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
Amiga::getConfigItem(unsigned dfn, ConfigOption option)
{
    assert(dfn < 4);
            
    switch (option) {
            
        case OPT_DRIVE_CONNECT:
            return paula.diskController.getConfigItem(dfn, option);
            
        case OPT_DRIVE_TYPE:
        case OPT_EMULATE_MECHANICS:
            return df[dfn]->getConfigItem(option);
            
        default: assert(false);
    }
    
    return 0;
}

bool
Amiga::configure(ConfigOption option, long value)
{
    // Propagate configuration request to all components
    bool changed = HardwareComponent::configure(option, value);
    
    // Inform the GUI if the configuration has changed
    if (changed) messageQueue.put(MSG_CONFIG);
    
    // Dump the current configuration in debugging mode
    if (changed && OPT_DEBUG) dumpConfig();

    return changed;
}

bool
Amiga::configure(unsigned drive, ConfigOption option, long value)
{
    assert(drive <= 3);
    
    // Propagate configuration request to all components
    bool changed = HardwareComponent::configure(drive, option, value);
    
    // Inform the GUI if the configuration has changed
    if (changed) messageQueue.put(MSG_CONFIG);
    
    // Dump the current configuration in debugging mode
    if (changed && OPT_DEBUG) dumpConfig();
        
    return changed;
}

void
Amiga::setInspectionTarget(EventID id)
{
    suspend();
    inspectionTarget = id;
    agnus.scheduleRel<INS_SLOT>(0, inspectionTarget);
    agnus.serviceINSEvent();
    resume();
}

void
Amiga::clearInspectionTarget()
{
    setInspectionTarget(INS_NONE);
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
Amiga::_dump()
{
    AmigaConfiguration config = getConfig();
    DiskControllerConfig dc = config.diskController;

    msg("    poweredOn: %s\n", isPoweredOn() ? "yes" : "no");
    msg("   poweredOff: %s\n", isPoweredOff() ? "yes" : "no");
    msg("       paused: %s\n", isPaused() ? "yes" : "no");
    msg("      running: %s\n", isRunning() ? "yes" : "no");
    msg("\n");
    msg("Current configuration:\n\n");
    msg("          df0: %s %s\n",
             dc.connected[0] ? "yes" : "no", driveTypeName(config.df0.type));
    msg("          df1: %s %s\n",
             dc.connected[1] ? "yes" : "no", driveTypeName(config.df1.type));
    msg("          df2: %s %s\n",
             dc.connected[2] ? "yes" : "no", driveTypeName(config.df2.type));
    msg("          df3: %s %s\n",
             dc.connected[3] ? "yes" : "no", driveTypeName(config.df3.type));

    msg("\n");
    msg("         warp: %d", warpMode);
    msg("\n");
}

void
Amiga::powerOn()
{
    trace(RUN_DEBUG, "powerOn()\n");
    
    #ifdef DF0_DISK
        DiskFile *df0file = DiskFile::makeWithFile(DF0_DISK);
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
    trace(RUN_DEBUG, "_powerOn()\n");

    // Clear all runloop flags
    runLoopCtrl = 0;

    // Update the recorded debug information
    inspect();

    messageQueue.put(MSG_POWER_ON);
}

void
Amiga::powerOff()
{
    trace(RUN_DEBUG, "powerOff()\n");
    
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
    trace("_powerOff()\n");
    
    // Update the recorded debug information
    inspect();
    
    messageQueue.put(MSG_POWER_OFF);
}

void
Amiga::run()
{
    trace(RUN_DEBUG, "run()\n");
        
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
    trace(RUN_DEBUG, "_run()\n");
    
    // Start the emulator thread
    pthread_create(&p, NULL, threadMain, (void *)this);
    
    // Inform the GUI
    messageQueue.put(MSG_RUN);
}

void
Amiga::pause()
{
    trace(RUN_DEBUG, "pause()\n");

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
    trace(RUN_DEBUG, "_pause()\n");
    
    // When we reach this line, the emulator thread is already gone
    assert(p == (pthread_t)0);
    
    // Update the recorded debug information
    inspect();

    // Inform the GUI
    messageQueue.put(MSG_PAUSE);
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
        
        messageQueue.put(MSG_WARP_ON);
        
    } else {
        
        oscillator.restart();
        // restartTimer();
        messageQueue.put(MSG_WARP_OFF);
    }
}

void
Amiga::acquireThreadLock()
{
    if (state == STATE_RUNNING) {
        
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
        if (error) *error = ERR_ROM_MISSING;
        return false;
    }

    if (!mem.hasChipRam()) {
        msg("isReady: No Chip Ram found\n");
        if (error) *error = ERR_ROM_MISSING;
        return false;
    }
    
    if (mem.hasArosRom()) {

        if (!mem.hasExt()) {
            msg("isReady: Aros requires an extension Rom\n");
            if (error) *error = ERR_AROS_NO_EXTROM;
            return false;
        }

        if (mem.ramSize() < MB(1)) {
            msg("isReady: Aros requires at least 1 MB of memory\n");
            if (error) *error = ERR_AROS_RAM_LIMIT;
            return false;
        }
    }

    if (mem.chipRamSize() > KB(agnus.chipRamLimit())) {
        msg("isReady: Chip Ram exceeds Agnus limit\n");
        if (error) *error = ERR_CHIP_RAM_LIMIT;
        return false;
    }

    return true;
}

void
Amiga::suspend()
{
    pthread_mutex_lock(&stateChangeLock);
    
    trace(RUN_DEBUG, "Suspending (%d)...\n", suspendCounter);
    
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
    
    trace(RUN_DEBUG, "Resuming (%d)...\n", suspendCounter);
    
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
    trace(RUN_DEBUG, "Emulator thread started\n");
}

void
Amiga::threadDidTerminate()
{
    trace(RUN_DEBUG, "Emulator thread terminated\n");

    // Trash the thread pointer
    p = 0;
    
    // Pause all components
    HardwareComponent::pause();
        
    // Release the thread lock
    pthread_mutex_unlock(&threadLock);
}

void
Amiga::runLoop()
{
    trace(RUN_DEBUG, "runLoop()\n");

    // Prepare to run
    oscillator.restart();
    // restartTimer();
    
    // Enable or disable debugging features
    if (debugMode) {
        cpu.debugger.enableLogging();
    } else {
        cpu.debugger.disableLogging();
    }
    agnus.scheduleRel<INS_SLOT>(0, inspectionTarget);
    
    // Enter the loop
    while(1) {
        
        // Emulate the next CPU instruction
        cpu.execute();

        // Check if special action needs to be taken
        if (runLoopCtrl) {
            
            // Are we requested to take a snapshot?
            if (runLoopCtrl & RL_AUTO_SNAPSHOT) {
                trace(RUN_DEBUG, "RL_AUTO_SNAPSHOT\n");
                autoSnapshot = Snapshot::makeWithAmiga(this);
                messageQueue.put(MSG_AUTO_SNAPSHOT_TAKEN);
                clearControlFlags(RL_AUTO_SNAPSHOT);
            }
            if (runLoopCtrl & RL_USER_SNAPSHOT) {
                trace(RUN_DEBUG, "RL_USER_SNAPSHOT\n");
                userSnapshot = Snapshot::makeWithAmiga(this);
                messageQueue.put(MSG_USER_SNAPSHOT_TAKEN);
                clearControlFlags(RL_USER_SNAPSHOT);
            }

            // Are we requested to update the debugger info structs?
            if (runLoopCtrl & RL_INSPECT) {
                trace(RUN_DEBUG, "RL_INSPECT\n");
                inspect();
                clearControlFlags(RL_INSPECT);
            }

            // Did we reach a breakpoint?
            if (runLoopCtrl & RL_BREAKPOINT_REACHED) {
                inspect();
                messageQueue.put(MSG_BREAKPOINT_REACHED);
                trace(RUN_DEBUG, "BREAKPOINT_REACHED pc: %x\n", cpu.getPC());
                clearControlFlags(RL_BREAKPOINT_REACHED);
                break;
            }

            // Did we reach a watchpoint?
            if (runLoopCtrl & RL_WATCHPOINT_REACHED) {
                inspect();
                messageQueue.put(MSG_WATCHPOINT_REACHED);
                trace(RUN_DEBUG, "WATCHPOINT_REACHED pc: %x\n", cpu.getPC());
                clearControlFlags(RL_WATCHPOINT_REACHED);
                break;
            }

            // Are we requested to terminate the run loop?
            if (runLoopCtrl & RL_STOP) {
                clearControlFlags(RL_STOP);
                trace(RUN_DEBUG, "RL_STOP\n");
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
        messageQueue.put(MSG_AUTO_SNAPSHOT_TAKEN);
        
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
        messageQueue.put(MSG_USER_SNAPSHOT_TAKEN);
        
    } else {
        
        // Schedule the snapshot to be taken
        signalUserSnapshot();
    }
}

Snapshot *
Amiga::latestAutoSnapshot()
{
    Snapshot *result = autoSnapshot;
    autoSnapshot = NULL;
    return result;
}

Snapshot *
Amiga::latestUserSnapshot()
{
    Snapshot *result = userSnapshot;
    userSnapshot = NULL;
    return result;
}

void
Amiga::loadFromSnapshotUnsafe(Snapshot *snapshot)
{
    u8 *ptr;
    
    if (snapshot && (ptr = snapshot->getData())) {
        load(ptr);
        messageQueue.put(MSG_SNAPSHOT_RESTORED);
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
