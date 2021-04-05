// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

DiskController::DiskController(Amiga& ref) : AmigaComponent(ref)
{
    setDescription("DiskController");

    // Setup initial configuration
    config.connected[0] = true;
    config.connected[1] = false;
    config.connected[2] = false;
    config.connected[3] = false;
    config.speed = 1;
    config.lockDskSync = false;
    config.autoDskSync = false;
}

void
DiskController::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    prb = 0xFF;
    selected = -1;
    dsksync = 0x4489;
    
    if (hard) {
        assert(diskToInsert == NULL);
    }
}

long
DiskController::getConfigItem(ConfigOption option)
{
    switch (option) {
            
        case OPT_DRIVE_SPEED:   return config.speed;
        case OPT_AUTO_DSKSYNC:  return config.autoDskSync;
        case OPT_LOCK_DSKSYNC:  return config.lockDskSync;
        
        default: assert(false);
    }
}

long
DiskController::getConfigItem(unsigned dfn, ConfigOption option)
{
    switch (option) {
            
        case OPT_DRIVE_CONNECT:  return config.connected[dfn];
        
        default: assert(false);
    }
}

bool
DiskController::setConfigItem(ConfigOption option, long value)
{
    switch (option) {
            
        case OPT_DRIVE_SPEED:
            
            #ifdef FORCE_DRIVE_SPEED
            value = FORCE_DRIVE_SPEED;
            warn("Overriding drive speed: %d\n", value);
            #endif
            
            if (!isValidDriveSpeed(value)) {
                warn("Invalid drive speed: %d\n", value);
                return false;
            }
            if (config.speed == value) {
                return false;
            }
            
            config.speed = value;
            trace("Setting acceleration factor to %d\n", config.speed);
            scheduleFirstDiskEvent();
            return true;
                        
        case OPT_AUTO_DSKSYNC:

            if (config.autoDskSync == value) {
                return false;
            }

            config.autoDskSync = value;
            return true;
            
        case OPT_LOCK_DSKSYNC:
            
            if (config.lockDskSync == value) {
                return false;
            }
            
            config.lockDskSync = value;
            return true;
            
        default:
            return false;
    }
}

bool
DiskController::setConfigItem(unsigned dfn, ConfigOption option, long value)
{
    switch (option) {
            
        case OPT_DRIVE_CONNECT:
            
            // We don't allow the internal drive (Df0) to be disconnected
            if (dfn == 0 && value == false) return false;
            
            // Connect or disconnect the drive
            config.connected[dfn] = value;
            
            // Inform the GUI
            messageQueue.put(value ? MSG_DRIVE_CONNECT : MSG_DRIVE_DISCONNECT, dfn);
            messageQueue.put(MSG_CONFIG);
            return true;
            
        default:
            return false;
    }
}

void
DiskController::_dumpConfig()
{
    msg("          df0 : %s\n", config.connected[0] ? "connected" : "disconnected");
    msg("          df1 : %s\n", config.connected[1] ? "connected" : "disconnected");
    msg("          df2 : %s\n", config.connected[2] ? "connected" : "disconnected");
    msg("          df3 : %s\n", config.connected[3] ? "connected" : "disonnected");
    msg("        Speed : %d\n", config.speed);
    msg("  lockDskSync : %s\n", config.lockDskSync ? "yes" : "no");
    msg("  autoDskSync : %s\n", config.autoDskSync ? "yes" : "no");
}

void
DiskController::_inspect()
{
    synchronized {
        
        info.selectedDrive = selected;
        info.state = state;
        info.fifoCount = fifoCount;
        info.dsklen = dsklen;
        info.dskbytr =  computeDSKBYTR();
        info.dsksync = dsksync;
        info.prb = prb;
        
        for (unsigned i = 0; i < 6; i++) {
            info.fifo[i] = (fifo >> (8 * i)) & 0xFF;
        }
    }
}

void
DiskController::_dump()
{
    msg("     selected : %d\n", selected);
    msg("        state : %s\n", driveStateName(state));
    msg("    syncCycle : %lld\n", syncCycle);
    msg("     incoming : %02X\n", incoming);
    msg("         fifo : %llX (count = %d)\n", fifo, fifoCount);
    msg("\n");
    msg("       dsklen : %X\n", dsklen);
    msg("      dsksync : %X\n", dsksync);
    msg("          prb : %X\n", prb);
    msg("\n");
    msg("   spinning() : %d\n", spinning());
}

Drive *
DiskController::getSelectedDrive()
{
    assert(selected < 4);
    return selected < 0 ? NULL : df[selected];
}

bool
DiskController::spinning(unsigned driveNr)
{
    assert(driveNr < 4);
    return df[driveNr]->getMotor();
}

bool
DiskController::spinning()
{
    return df0.getMotor() || df1.getMotor() ||df2.getMotor() || df3.getMotor();
}

void
DiskController::setState(DriveState newState)
{
    if (state != newState) setState(state, newState);
}

void
DiskController::setState(DriveState oldState, DriveState newState)
{
    trace(DSK_DEBUG, "%s -> %s\n",
          driveStateName(oldState), driveStateName(newState));
    
    state = newState;
    
    switch (state) {

        case DRIVE_DMA_OFF:
            dsklen = 0;
            break;
            
        case DRIVE_DMA_WRITE:
            messageQueue.put(MSG_DRIVE_WRITE, selected);
            break;
            
        default:
            if (oldState == DRIVE_DMA_WRITE)
                messageQueue.put(MSG_DRIVE_READ, selected);
    }
}

void
DiskController::ejectDisk(int nr, Cycle delay)
{
    assert(nr >= 0 && nr <= 3);

    trace("ejectDisk(%d, %d)\n", nr, delay);

    amiga.suspend();
    agnus.scheduleRel<DCH_SLOT>(delay, DCH_EJECT, nr);
    amiga.resume();
}

void
DiskController::insertDisk(class Disk *disk, int nr, Cycle delay)
{
    assert(disk != NULL);
    assert(nr >= 0 && nr <= 3);

    trace(DSK_DEBUG, "insertDisk(%p, %d, %d)\n", disk, nr, delay);

    // The easy case: The emulator is not running
    if (!amiga.isRunning()) {

        df[nr]->ejectDisk();
        df[nr]->insertDisk(disk);
        return;
    }

    // The not so easy case: The emulator is running
    amiga.suspend();

    if (df[nr]->hasDisk()) {

        // Eject the old disk first
        df[nr]->ejectDisk();

        // Make sure there is enough time between ejecting and inserting.
        // Otherwise, the Amiga might not detect the change.
        delay = MAX((Cycle)SEC(1.5), delay);
    }

    diskToInsert = disk;
    agnus.scheduleRel<DCH_SLOT>(delay, DCH_INSERT, nr);
    
    amiga.resume();
}

void
DiskController::insertDisk(class ADFFile *file, int nr, Cycle delay)
{
    if (Disk *disk = Disk::makeWithFile(file)) {
        insertDisk(disk, nr, delay);
    }
}

void
DiskController::insertDisk(class IMGFile *file, int nr, Cycle delay)
{
    if (Disk *disk = Disk::makeWithFile(file)) {
        insertDisk(disk, nr, delay);
    }
}

void
DiskController::insertDisk(class DMSFile *file, int nr, Cycle delay)
{
    if (Disk *disk = Disk::makeWithFile(file)) {
        insertDisk(disk, nr, delay);
    }
}

void
DiskController::insertDisk(class EXEFile *file, int nr, Cycle delay)
{
    if (Disk *disk = Disk::makeWithFile(file)) {
        insertDisk(disk, nr, delay);
    }
}

void
DiskController::insertDisk(class DIRFile *file, int nr, Cycle delay)
{
    if (Disk *disk = Disk::makeWithFile(file)) {
        insertDisk(disk, nr, delay);
    }
}

void
DiskController::setWriteProtection(int nr, bool value)
{
    assert(nr >= 0 && nr <= 3);
    df[nr]->setWriteProtection(value);
}

void
DiskController::clearFifo()
{
    fifo = 0;
    fifoCount = 0;
}

u8
DiskController::readFifo()
{
    assert(fifoCount >= 1);
    
    // Remove and return the oldest byte
    fifoCount -= 1;
    return (fifo >> (8 * fifoCount)) & 0xFF;
}

u16
DiskController::readFifo16()
{
    assert(fifoCount >= 2);
    
    // Remove and return the oldest word
    fifoCount -= 2;
    return (fifo >> (8 * fifoCount)) & 0xFFFF;
}

void
DiskController::writeFifo(u8 byte)
{
    assert(fifoCount <= 6);
    
    // Remove oldest word if the FIFO is full
    if (fifoCount == 6) fifoCount -= 2;
    
    // Add the new byte
    fifo = (fifo << 8) | byte;
    fifoCount++;
}

bool
DiskController::compareFifo(u16 word)
{
    return fifoHasWord() && (fifo & 0xFFFF) == word;
}

void
DiskController::executeFifo()
{
    // Only proceed if a drive is selected
    Drive *drive = getSelectedDrive();

    switch (state) {
            
        case DRIVE_DMA_OFF:
        case DRIVE_DMA_WAIT:
        case DRIVE_DMA_READ:
            
            // Read a byte from the drive
            incoming = drive ? drive->readByteAndRotate() : 0;
            
            // Write byte into the FIFO buffer
            writeFifo(incoming);
            incoming |= 0x8000;
            
            // Check if we've reached a SYNC mark
            if (compareFifo(dsksync) ||
                (config.autoDskSync && syncCounter++ > 20000)) {

                // Save time stamp
                syncCycle = agnus.clock;

                // Trigger a word SYNC interrupt
                trace(DSK_DEBUG, "SYNC IRQ (dsklen = %d)\n", dsklen);
                paula.raiseIrq(INT_DSKSYN);

                // Enable DMA if the controller was waiting for it
                if (state == DRIVE_DMA_WAIT) {
                    setState(DRIVE_DMA_READ);
                    clearFifo();
                }
                
                // Reset the watchdog counter
                syncCounter = 0;
            }
            break;
            
        case DRIVE_DMA_WRITE:
        case DRIVE_DMA_FLUSH:
            
            // debug("DRIVE_DMA_WRITE\n");
            
            if (fifoIsEmpty()) {
                
                // Switch off DMA if the last byte has been flushed out
                if (state == DRIVE_DMA_FLUSH) setState(DRIVE_DMA_OFF);
                
            } else {
                
                // Read the outgoing byte from the FIFO buffer
                u8 outgoing = readFifo();
                
                // Write byte to disk
                if (drive) drive->writeByteAndRotate(outgoing);
            }
            break;
    }
}

void
DiskController::performDMA()
{
    Drive *drive = getSelectedDrive();
    
    // Only proceed if there are remaining bytes to process
    if ((dsklen & 0x3FFF) == 0) return;

    // Only proceed if DMA is enabled
    if (state != DRIVE_DMA_READ && state != DRIVE_DMA_WRITE) return;

    // How many words shall we read in?
    u32 count = drive ? config.speed : 1;

    // Perform DMA
    switch (state) {
            
        case DRIVE_DMA_READ:
            performDMARead(drive, count);
            break;
            
        case DRIVE_DMA_WRITE:
            performDMAWrite(drive, count);
            break;
            
        default: assert(false);
    }
}

void
DiskController::performDMARead(Drive *drive, u32 remaining)
{
    // Only proceed if the FIFO contains enough data
    if (!fifoHasWord()) return;

    do {
        // Read next word from the FIFO buffer
        u16 word = readFifo16();
        
        // Write word into memory
        if (DSK_CHECKSUM) {
            checkcnt++;
            check1 = fnv_1a_it32(check1, word);
            check2 = fnv_1a_it32(check2, agnus.dskpt & agnus.ptrMask);
        }
        agnus.doDiskDMA(word);

        // Finish up if this was the last word to transfer
        if ((--dsklen & 0x3FFF) == 0) {

            paula.raiseIrq(INT_DSKBLK);
            setState(DRIVE_DMA_OFF);

            if (DSK_CHECKSUM)
                debug("read: cnt = %d check1 = %x check2 = %x\n",
                           checkcnt, check1, check2);

            return;
        }
        
        // If the loop repeats, fill the Fifo with new data
        if (--remaining) {
            executeFifo();
            executeFifo();
        }
        
    } while (remaining);
}

void
DiskController::performDMAWrite(Drive *drive, u32 remaining)
{
    // Only proceed if the FIFO has enough free space
    if (!fifoCanStoreWord()) return;

    do {
        // Read next word from memory
        if (DSK_CHECKSUM) {
            checkcnt++;
            check2 = fnv_1a_it32(check2, agnus.dskpt & agnus.ptrMask);
        }
        u16 word = agnus.doDiskDMA();

        if (DSK_CHECKSUM) {
            check1 = fnv_1a_it32(check1, word);
        }

        // Write word into FIFO buffer
        assert(fifoCount <= 4);
        writeFifo(HI_BYTE(word));
        writeFifo(LO_BYTE(word));

        // Finish up if this was the last word to transfer
        if ((--dsklen & 0x3FFF) == 0) {

            paula.raiseIrq(INT_DSKBLK);

            /* The timing-accurate approach: Set state to DRIVE_DMA_FLUSH.
             * The event handler recognises this state and switched to
             * DRIVE_DMA_OFF once the FIFO has been emptied.
             */
            
            // setState(DRIVE_DMA_FLUSH);
            
            /* I'm unsure of the timing-accurate approach works properly,
             * because the disk IRQ would be triggered before the last byte
             * has been written.
             * Hence, we play safe here and flush the FIFO immediately.
             */
            while (!fifoIsEmpty()) {
                u8 value = readFifo();
                if (drive) drive->writeByteAndRotate(value);
            }
            setState(DRIVE_DMA_OFF);

            if (DSK_CHECKSUM)
                debug("write: cnt = %d check1 = %x check2 = %x\n",
                           checkcnt, check1, check2);

            return;
        }
        
        // If the loop repeats, do what the event handler would do in between.
        if (--remaining) {
            executeFifo();
            executeFifo();
            assert(fifoCanStoreWord());
        }
        
    } while (remaining);
}

void
DiskController::performTurboDMA(Drive *drive)
{
    // Only proceed if there is anything to read or write
    if ((dsklen & 0x3FFF) == 0) return;

    // Perform action depending on DMA state
    switch (state) {

        case DRIVE_DMA_WAIT:

            drive->findSyncMark();
            fallthrough;

        case DRIVE_DMA_READ:
            
            if (drive) performTurboRead(drive);
            if (drive) paula.raiseIrq(INT_DSKSYN);
            break;
            
        case DRIVE_DMA_WRITE:
            
            if (drive) performTurboWrite(drive);
            break;
            
        default:
            return;
    }
    
    // Trigger disk interrupt with some delay
    Cycle delay = MIMIC_UAE ? 2 * HPOS_CNT - agnus.pos.h + 30 : 512;
    paula.scheduleIrqRel(INT_DSKBLK, DMA_CYCLES(delay));

    setState(DRIVE_DMA_OFF);
}

void
DiskController::performTurboRead(Drive *drive)
{
    for (unsigned i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from disk
        u16 word = drive->readWordAndRotate();
        
        // Write word into memory
        if (DSK_CHECKSUM) {
            checkcnt++;
            check1 = fnv_1a_it32(check1, word);
            check2 = fnv_1a_it32(check2, agnus.dskpt & agnus.ptrMask);
        }
        mem.poke16 <AGNUS_ACCESS> (agnus.dskpt, word);
        agnus.dskpt += 2;
    }

    if (DSK_CHECKSUM) {
        debug("Turbo read %s: cyl: %d side: %d offset: %d checkcnt = %d check1 = %x check2 = %x\n", drive->getDescription(), drive->head.cylinder, drive->head.side, drive->head.offset, checkcnt, check1, check2);
    }
}

void
DiskController::performTurboWrite(Drive *drive)
{
    for (unsigned i = 0; i < (dsklen & 0x3FFF); i++) {
        
        // Read word from memory
        u16 word = mem.peek16 <AGNUS_ACCESS> (agnus.dskpt);
        
        if (DSK_CHECKSUM) {
            checkcnt++;
            check1 = fnv_1a_it32(check1, word);
            check2 = fnv_1a_it32(check2, agnus.dskpt & agnus.ptrMask);
        }

        agnus.dskpt += 2;

        // Write word to disk
        drive->writeWordAndRotate(word);
    }

    if (DSK_CHECKSUM) {
        debug("Turbo write %s: checkcnt = %d check1 = %x check2 = %x\n",
                   drive->getDescription(), checkcnt, check1, check2);
    }
}

