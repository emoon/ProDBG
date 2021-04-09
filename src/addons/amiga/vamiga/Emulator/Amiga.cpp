// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Amiga.h"
#include "Snapshot.h"
#include <assert.h>

// Perform some consistency checks
static_assert(sizeof(i8) == 1,  "i8 size mismatch");
static_assert(sizeof(i16) == 2, "i16 size mismatch");
static_assert(sizeof(i32) == 4, "i32 size mismatch");
static_assert(sizeof(i64) == 8, "i64 size mismatch");
static_assert(sizeof(u8) == 1,  "u8 size mismatch");
static_assert(sizeof(u16) == 2, "u16 size mismatch");
static_assert(sizeof(u32) == 4, "u32 size mismatch");
static_assert(sizeof(u64) == 8, "u64 size mismatch");


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

    subComponents = std::vector<HardwareComponent *> {

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

    // Print some debug information
    if (SNP_DEBUG) {

        msg("             Agnus : %zu bytes\n", sizeof(Agnus));
        msg("       AudioFilter : %zu bytes\n", sizeof(AudioFilter));
        // msg("       AudioStream : %zu bytes\n", sizeof(AudioStream));
        msg("               CIA : %zu bytes\n", sizeof(CIA));
        msg("       ControlPort : %zu bytes\n", sizeof(ControlPort));
        msg("               CPU : %zu bytes\n", sizeof(CPU));
        msg("            Denise : %zu bytes\n", sizeof(Denise));
        msg("             Drive : %zu bytes\n", sizeof(Drive));
        msg("          Keyboard : %zu bytes\n", sizeof(Keyboard));
        msg("            Memory : %zu bytes\n", sizeof(Memory));
        msg("moira::Breakpoints : %zu bytes\n", sizeof(moira::Breakpoints));
        msg("moira::Watchpoints : %zu bytes\n", sizeof(moira::Watchpoints));
        msg("   moira::Debugger : %zu bytes\n", sizeof(moira::Debugger));
        msg("      moira::Moira : %zu bytes\n", sizeof(moira::Moira));
        msg("             Muxer : %zu bytes\n", sizeof(Muxer));
        msg("        Oscillator : %zu bytes\n", sizeof(Oscillator));
        msg("             Paula : %zu bytes\n", sizeof(Paula));
        msg("       PixelEngine : %zu bytes\n", sizeof(PixelEngine));
        msg("               RTC : %zu bytes\n", sizeof(RTC));
        msg("           Sampler : %zu bytes\n", sizeof(Sampler));
        msg("    ScreenRecorder : %zu bytes\n", sizeof(ScreenRecorder));
        msg("        SerialPort : %zu bytes\n", sizeof(SerialPort));
        msg("            Volume : %zu bytes\n", sizeof(Volume));
        msg("             Zorro : %zu bytes\n", sizeof(ZorroManager));
        msg("\n");
    }
}

Amiga::~Amiga()
{
    debug(RUN_DEBUG, "Destroying Amiga[%p]\n", this);
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
        case OPT_HIDDEN_SPRITES:
        case OPT_HIDDEN_LAYERS:
        case OPT_HIDDEN_LAYER_ALPHA:
        case OPT_CLX_SPR_SPR:
        case OPT_CLX_SPR_PLF:
        case OPT_CLX_PLF_PLF:
            return denise.getConfigItem(option);

        case OPT_PALETTE:
        case OPT_BRIGHTNESS:
        case OPT_CONTRAST:
        case OPT_SATURATION:
            return denise.pixelEngine.getConfigItem(option);

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
        case OPT_DRIVE_PAN:
        case OPT_STEP_VOLUME:
        case OPT_POLL_VOLUME:
        case OPT_INSERT_VOLUME:
        case OPT_EJECT_VOLUME:
            return df[id]->getConfigItem(option);

        case OPT_DEFAULT_FILESYSTEM:
        case OPT_DEFAULT_BOOTBLOCK:
            return df[id]->getConfigItem(option);

        case OPT_PULLUP_RESISTORS:
        case OPT_MOUSE_VELOCITY:
            if (id == PORT_1) return controlPort1.mouse.getConfigItem(option);
            if (id == PORT_2) return controlPort2.mouse.getConfigItem(option);
            assert(false);

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

    // Dump the current configuration in debug mode
    if (changed && CNF_DEBUG) dump(Dump::Config);

    return changed;
}

bool
Amiga::configure(Option option, long id, long value)
{
    // Propagate configuration request to all components
    bool changed = HardwareComponent::configure(option, id, value);

    // Inform the GUI if the configuration has changed
    if (changed) queue.put(MSG_CONFIG);

    // Dump the current configuration in debug mode
    if (changed && CNF_DEBUG) dump(Dump::Config);

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
Amiga::_dump(Dump::Category category, std::ostream& os) const
{
    if (category & Dump::Config) {

        if (CNF_DEBUG) {

            df0.dump(Dump::Config);
            paula.dump(Dump::Config);
            paula.muxer.dump(Dump::Config);
            ciaA.dump(Dump::Config);
            denise.dump(Dump::Config);
        }
    }

    if (category & Dump::State) {

        os << DUMP("Power") << ONOFF(isPoweredOn()) << std::endl;
        os << DUMP("Running") << YESNO(isRunning()) << std::endl;
        os << DUMP("Warp") << ONOFF(warpMode) << std::endl;
    }
}

void
Amiga::powerOn()
{
    assert(!isEmulatorThread());

    debug(RUN_DEBUG, "powerOn()\n");

    if (isPoweredOff() && isReady()) {

        assert(p == 0);

        // Perform a hard reset
        hardReset();

        // Switch state
        state = EMULATOR_STATE_PAUSED;

        // Power on all subcomponents
        HardwareComponent::powerOn();

        // Update the recorded debug information
        inspect();

        // Inform the GUI
        queue.put(MSG_POWER_ON);
    }
}

void
Amiga::_powerOn()
{
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
}

void
Amiga::powerOff()
{
    assert(!isEmulatorThread());
    assert(!isRunning());

    debug(RUN_DEBUG, "powerOff()\n");

    if (isPoweredOn()) {

        // Switch state
        state = EMULATOR_STATE_OFF;

        // Power off all subcomponents
        HardwareComponent::powerOff();

        // Update the recorded debug information
        inspect();

        // Inform the GUI
        queue.put(MSG_POWER_OFF);
    }
}

void
Amiga::_powerOff()
{
}

void
Amiga::run()
{
    assert(isPoweredOn());

    debug(RUN_DEBUG, "run()\n");

    if (!isRunning() && isReady()) {

        assert(p == 0);

        // Switch state
        state = EMULATOR_STATE_RUNNING;

        // Create the emulator thread
        pthread_create(&p, nullptr, threadMain, (void *)this);

        // Inform the GUI
        queue.put(MSG_RUN);
    }
}

void
Amiga::_run()
{
}

void
Amiga::pause()
{
    debug(RUN_DEBUG, "pause()\n");

    if (!isPaused()) {

        // Ask the emulator thread to terminate
        signalStop();

        // Wait until the emulator thread has terminated
        pthread_join(p, nullptr);

        // Update the recorded debug information
        inspect();

        // Inform the GUI
        queue.put(MSG_PAUSE);
    }
}

void
Amiga::shutdown()
{
    // Assure the Amiga is powered off
    assert(isPoweredOff());

    /* Send the SHUTDOWN message which is the last message ever send. The
     * purpose of this message is to signal the GUI that no more messages will
     * show up in the message queue. When the GUI receives this message, it
     * knows that the Amiga is powered off and the message queue empty. From
     * this time on, it is safe to destroy the Amiga object.
     */
    queue.put(MSG_SHUTDOWN);
}

void
Amiga::_pause()
{
}

void
Amiga::warpOn()
{
    assert(!isEmulatorThread());

    if (!warpMode) {
        HardwareComponent::warpOn();
    }
}

void
Amiga::warpOff()
{
    assert(!isEmulatorThread());

    if (warpMode) {
        HardwareComponent::warpOff();
    }
}

void
Amiga::_warpOn()
{
    HardwareComponent::_warpOn();
    queue.put(MSG_WARP_ON);
}

void
Amiga::_warpOff()
{
    HardwareComponent::_warpOff();
    oscillator.restart();
    queue.put(MSG_WARP_OFF);
}

void
Amiga::debugOn()
{
    assert(!isEmulatorThread());

    if (!debugMode) {
        HardwareComponent::debugOn();
    }
}

void
Amiga::debugOff()
{
    assert(!isEmulatorThread());

    if (debugMode) {
        HardwareComponent::debugOff();
    }
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
    debug(RUN_DEBUG, "Suspending (%zu)...\n", suspendCounter);

    if (suspendCounter || isRunning()) {
        pause();
        suspendCounter++;
    }
}

void
Amiga::resume()
{
    debug(RUN_DEBUG, "Resuming (%zu)...\n", suspendCounter);

    if (suspendCounter && --suspendCounter == 0) {
        run();
    }
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
}

void
Amiga::runLoop()
{
    debug(RUN_DEBUG, "runLoop()\n");

    HardwareComponent::run();

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

            // Are we requested to enter of exit warp mode?
            if (runLoopCtrl & RL_WARP_ON) {
                clearControlFlags(RL_WARP_ON);
                debug(RUN_DEBUG, "RL_WARP_ON\n");
                warpOn();
            }

            if (runLoopCtrl & RL_WARP_OFF) {
                clearControlFlags(RL_WARP_OFF);
                debug(RUN_DEBUG, "RL_WARP_OFF\n");
                warpOff();
            }
        }
    }

    // Switch state
    state = EMULATOR_STATE_PAUSED;
    HardwareComponent::pause();
}

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
