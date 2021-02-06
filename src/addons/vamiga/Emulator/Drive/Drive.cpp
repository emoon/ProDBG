// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

Drive::Drive(Amiga& ref, unsigned n) : AmigaComponent(ref), nr(n)
{
    assert(nr < 4);

    setDescription(nr == 0 ? "Df0" :
                   nr == 1 ? "Df1" :
                   nr == 2 ? "Df2" : "Df3");

    config.type = DRIVE_35_DD;
    config.mechanicalDelays = true;
    config.startDelay = MSEC(380);
    config.stopDelay = MSEC(80);
    config.stepDelay = USEC(2000);
}

void
Drive::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
}

long
Drive::getConfigItem(ConfigOption option)
{
    switch (option) {
            
        case OPT_DRIVE_TYPE:        return (long)config.type;
        case OPT_EMULATE_MECHANICS: return (long)config.mechanicalDelays;
            
        default: assert(false);
    }
}

bool
Drive::setConfigItem(unsigned dfn, ConfigOption option, long value)
{
    if (dfn != nr) return false;
 
    switch (option) {
                            
        case OPT_DRIVE_TYPE:
            
            if (!isDriveType(value)) {
                warn("Invalid drive type: %d\n", value);
                return false;
            }
            if (value != DRIVE_35_DD && value != DRIVE_35_HD) {
                warn("Unsupported type: %s\n", driveTypeName((DriveType)value));
                return false;
            }
            if (config.type == value) {
                return false;
            }
            
            config.type = (DriveType)value;
            trace("Setting type to %s\n", driveTypeName(config.type));
            return true;

        case OPT_EMULATE_MECHANICS:
        
            if (config.mechanicalDelays == value) {
                return false;
            }
                        
            config.mechanicalDelays = value;
            trace("Setting emulateMechanics to %d\n", config.mechanicalDelays);
            return true;

        default:
            return false;
    }
}

void
Drive::_inspect()
{
    synchronized {
        
        info.head = head;
        info.hasDisk = hasDisk();
        info.motor = getMotor();
    }
}

void
Drive::_dumpConfig()
{
    msg("              Type : %s\n", driveTypeName(config.type));
    msg(" Emulate mechanics : %s\n", config.mechanicalDelays ? "yes" : "no");
    msg("       Start delay : %d\n", config.startDelay);
    msg("        Stop delay : %d\n", config.stopDelay);
    msg("        Step delay : %d\n", config.stepDelay);
}

void
Drive::_dump()
{
    msg("                Nr: %d\n", nr);
    msg("          Id count: %d\n", idCount);
    msg("            Id bit: %d\n", idBit);
    msg("      motorSpeed(): %.2f\n", motorSpeed());
    msg("        getMotor(): %s\n", getMotor() ? "on" : "off");
    msg(" motorSpeedingUp(): %s\n", motorSpeedingUp() ? "yes" : "no");
    msg("motorAtFullSpeed(): %s\n", motorAtFullSpeed() ? "yes" : "no");
    msg("motorSlowingDown(): %s\n", motorSlowingDown() ? "yes" : "no");
    msg("    motorStopped(): %s\n", motorStopped() ? "yes" : "no");
    msg("         dskchange: %d\n", dskchange);
    msg("            dsklen: %X\n", dsklen);
    msg("               prb: %X\n", prb);
    msg("              Side: %d\n", head.side);
    msg("         Cyclinder: %d\n", head.cylinder);
    msg("            Offset: %d\n", head.offset);
    msg("   cylinderHistory: %X\n", cylinderHistory);
    msg("              Disk: %s\n", disk ? "yes" : "no");
    
    if (disk) disk->dump();
}

size_t
Drive::_size()
{
    SerCounter counter;

    applyToPersistentItems(counter);
    applyToHardResetItems(counter);
    applyToResetItems(counter);

    // Add the size of the boolean indicating whether a disk is inserted
    counter.count += sizeof(bool);

    if (hasDisk()) {

        // Add the disk type and disk state
        counter & disk->getType() & disk->getDensity();
        disk->applyToPersistentItems(counter);
        // counter.count += disk->geometry.diskSize;
    }

    return counter.count;
}

size_t
Drive::_load(u8 *buffer)
{
    SerReader reader(buffer);

    // Read own state
    applyToPersistentItems(reader);
    applyToHardResetItems(reader);
    applyToResetItems(reader);

    // Delete the current disk
    if (disk) {
        delete disk;
        disk = NULL;
    }

    // Check if the snapshot includes a disk
    bool diskInSnapshot;
    reader & diskInSnapshot;

    // If yes, create recreate the disk
    if (diskInSnapshot) {
        DiskType type;
        DiskDensity density;
        reader & type & density;
        
        disk = Disk::makeWithReader(reader, type, density);
    }

    trace(SNP_DEBUG, "Recreated from %d bytes\n", reader.ptr - buffer);
    return reader.ptr - buffer;
}

size_t
Drive::_save(u8 *buffer)
{
    SerWriter writer(buffer);

    // Write own state
    applyToPersistentItems(writer);
    applyToHardResetItems(writer);
    applyToResetItems(writer);

    // Indicate whether this drive has a disk is inserted
    writer & hasDisk();

    if (hasDisk()) {

        // Write the disk type
        writer & disk->getType() & disk->getDensity();

        // Write the disk's state
        disk->applyToPersistentItems(writer);
        // writer.copy(disk->oldData, disk->geometry.diskSize);
    }

    trace(SNP_DEBUG, "Serialized to %d bytes\n", writer.ptr - buffer);
    return writer.ptr - buffer;
}

bool
Drive::idMode()
{
    return motorStopped() || motorSpeedingUp();
}

u32
Drive::getDriveId()
{
    if (nr > 0) {
        
        // External floopy drives identify themselve as follows:
        //
        //     3.5" DD: 0xFFFFFFFF
        //     3.5" HD: 0xAAAAAAAA if an HD disk is inserted
        //              0xFFFFFFFF if no disk or a DD disk is inserted
        //     5.25"SD: 0x55555555
        
        switch(config.type) {
                
            case DRIVE_35_DD:
                return 0xFFFFFFFF;
                
            case DRIVE_35_HD:
                if (disk && disk->getDensity() == DISK_HD) return 0xAAAAAAAA;
                return 0xFFFFFFFF;
                
            case DRIVE_525_DD:
                return 0x55555555;
        }
    }
    
    // The internal floppy drive identifies itself as 0x00000000
    return 0x00000000;
}

u8
Drive::driveStatusFlags()
{
    // if (nr == 0) debug("driveStatusFlags() %d %d %d %d\n", isSelected(), idMode(), motorAtFullSpeed(), motorSlowingDown());

    u8 result = 0xFF;
    
    if (isSelected()) {
        
        // PA5: /DSKRDY
        if (idMode()) {
            if (idBit) result &= 0b11011111;
        } else if (hasDisk()) {
            if (motorAtFullSpeed() || motorSlowingDown()) result &= 0b11011111;
        }
        
        // PA4: /DSKTRACK0
        // debug("Head is at cyclinder %d\n", head.cylinder);
        if (head.cylinder == 0) { result &= 0b11101111; }
        
        // PA3: /DSKPROT
        if (!hasWriteEnabledDisk()) { result &= 0b11110111; }
        
        /* PA2: /DSKCHANGE
         * "Disk has been removed from the drive. The signal goes low whenever
         *  a disk is removed. It remains low until a disk is inserted AND a
         *  step pulse is received." [HRM]
         */
        if (!dskchange) result &= 0b11111011;
    }
    
    return result;
}

double
Drive::motorSpeed()
{
    // Quick exit if mechanics is not emulated
    if (!config.mechanicalDelays) return motor ? 100.0 : 0.0;
    
    // Determine the elapsed cycles since the last motor change
    Cycle elapsed = agnus.clock - switchCycle;
    assert(elapsed >= 0);
    
    // Compute the current speed
    if (motor) {
        if (config.startDelay == 0) return 100.0;
        return MIN(switchSpeed + 100.0 * (elapsed / config.startDelay), 100.0);
    } else {
        if (config.stopDelay == 0) return 0.0;
        return MAX(switchSpeed - 100.0 * (elapsed / config.stopDelay), 0.0);
    }
}

void
Drive::setMotor(bool value)
{
    // Only proceed if motor state will change
    if (motor == value) return;
    
    // Switch motor state
    switchSpeed = motorSpeed();
    switchCycle = agnus.clock;
    motor = value;
    
    // Reset the identification bit counter if motor has been turned off
    idCount = 0;
    
    // Inform the GUI
    messageQueue.put(value ? MSG_DRIVE_LED_ON : MSG_DRIVE_LED_OFF, nr);
    messageQueue.put(value ? MSG_DRIVE_MOTOR_ON : MSG_DRIVE_MOTOR_OFF, nr);
    
    debug(DSK_DEBUG, "Motor %s [%d]\n", motor ? "on" : "off", idCount);
}

bool
Drive::motorSpeedingUp()
{
    return motor && motorSpeed() < 100.0;
}

bool
Drive::motorAtFullSpeed()
{
    return motorSpeed() == 100.0;
}

bool
Drive::motorSlowingDown()
{
    return !motor && motorSpeed() > 0.0;
}

bool
Drive::motorStopped()
{
    return motorSpeed() == 0.0;
}

void
Drive::selectSide(int side)
{
    assert(side < 2);
    if (head.side != side) trace("*** Select side %d\n", side);

    head.side = side;
}

u8
Drive::readByte()
{
    // Case 1: No disk is inserted
    if (!disk) {
        return 0xFF;
    }

    // Case 2: A step operation is in progress
    if (config.mechanicalDelays && (agnus.clock - stepCycle) < config.stepDelay) {
      return 0xFF;
    }
    
    // Case 3: Normal operation
    return disk->readByte(head.cylinder, head.side, head.offset);
}

u8
Drive::readByteAndRotate()
{
    u8 result = readByte();
    if (motor) rotate();
    return result;
}

u16
Drive::readWordAndRotate()
{
    u8 byte1 = readByteAndRotate();
    u8 byte2 = readByteAndRotate();
    
    return HI_LO(byte1, byte2);
}

void
Drive::writeByte(u8 value)
{
    if (disk) {
        disk->writeByte(value, head.cylinder, head.side, head.offset);
    }
}

void
Drive::writeByteAndRotate(u8 value)
{
    writeByte(value);
    if (motor) rotate();
}

void
Drive::writeWordAndRotate(u16 value)
{
    writeByteAndRotate(HI_BYTE(value));
    writeByteAndRotate(LO_BYTE(value));
}

void
Drive::rotate()
{
    long last = disk ? disk->length.cylinder[head.cylinder][head.side] : 12668;
    if (++head.offset >= last) {
        
        // Start over at the beginning of the current cyclinder
        head.offset = 0;

        // If this drive is selected, we emulate a falling edge on the flag pin
        // of CIA B. This causes the CIA to trigger the INDEX interrupt if the
        // corresponding enable bit is set.
        if (isSelected()) ciab.emulateFallingEdgeOnFlagPin();
    }
}

void
Drive::findSyncMark()
{
    long length = disk->length.cylinder[head.cylinder][head.side];
    for (unsigned i = 0; i < length; i++) {
        
        if (readByteAndRotate() != 0x44) continue;
        if (readByteAndRotate() != 0x89) continue;
        break;
    }

    trace(DSK_DEBUG, "Moving to SYNC mark at offset %d\n", head.offset);
}

bool
Drive::readyToStep()
{
    if (config.mechanicalDelays) {
        return agnus.clock - stepCycle > 1060;
    } else {
        return true;
    }
}

void
Drive::step(int dir)
{    
    // Update disk change signal
    if (hasDisk()) dskchange = true;
 
    // Only proceed if the last head step was a while ago
    if (!readyToStep()) return;
    
    if (dir) {
        
        // Move drive head outwards (towards the lower tracks)
        if (head.cylinder > 0) {
            head.cylinder--;
            recordCylinder(head.cylinder);
        }
        if (DSK_CHECKSUM)
            debug("Stepping down to cylinder %d\n", head.cylinder);

    } else {
        
        // Move drive head inwards (towards the upper tracks)
        if (head.cylinder < 83) {
            head.cylinder++;
            recordCylinder(head.cylinder);
        }
        if (DSK_CHECKSUM)
            debug("Stepping up to cylinder %d\n", head.cylinder);
    }
    
    // Push drive head forward
    // head.offset = ALIGN_HEAD ? 0 : ((head.offset + 117) % Disk::trackSize);
    if (ALIGN_HEAD) head.offset = 0;

    // Inform the GUI
    if (pollsForDisk()) {
        messageQueue.put(MSG_DRIVE_HEAD_POLL, (nr << 8) | head.cylinder);
    } else {
        messageQueue.put(MSG_DRIVE_HEAD, (nr << 8) | head.cylinder);
    }
    
    // Remember when we've performed the step
    stepCycle = agnus.clock;
}

void
Drive::recordCylinder(u8 cylinder)
{
    cylinderHistory = (cylinderHistory << 8) | cylinder;
}

bool
Drive::pollsForDisk()
{
    // Disk polling is only performed if no disk is inserted
    if (hasDisk()) return false;

    /* Head polling sequences of different Kickstart versions:
     *
     * Kickstart 1.2 and 1.3: 0-1-0-1-0-1-...
     * Kickstart 2.0:         0-1-2-3-2-1-...
     */
    static const u64 signature[] = {

        // Kickstart 1.2 and 1.3
        0x010001000100,
        0x000100010001,

        // Kickstart 2.0
        0x020302030203,
        0x030203020302,
    };

    u64 mask = 0xFFFFFFFF;
    for (unsigned i = 0; i < sizeof(signature) / 8; i++) {
        if ((cylinderHistory & mask) == (signature[i] & mask)) return true;
    }

    return false;
}

bool
Drive::hasWriteEnabledDisk()
{
    return hasDisk() ? !disk->isWriteProtected() : false;
}

bool
Drive::hasWriteProtectedDisk()
{
    return hasDisk() ? disk->isWriteProtected() : false;
}

void
Drive::setWriteProtection(bool value)
{
    if (disk) {
        
        if (value && !disk->isWriteProtected()) {
            
            disk->setWriteProtection(true);
            messageQueue.put(MSG_DISK_PROTECT);
        }
        if (!value && disk->isWriteProtected()) {
            
            disk->setWriteProtection(false);
            messageQueue.put(MSG_DISK_UNPROTECT);
        }
    }
}

void
Drive::toggleWriteProtection()
{
    if (hasDisk()) {
        disk->setWriteProtection(!disk->isWriteProtected());
    }
}

void
Drive::ejectDisk()
{
    trace(DSK_DEBUG, "ejectDisk()\n");

    if (disk) {
        
        // Flag disk change in the CIAA::PA
        dskchange = false;
        
        // Get rid of the disk
        delete disk;
        disk = NULL;
        
        // Notify the GUI
        messageQueue.put(MSG_DISK_EJECT, nr);
    }
}

bool
Drive::isInsertable(DiskType t, DiskDensity d)
{
    debug(DSK_DEBUG, "isInsertable(%s, %s)\n", sDiskType(t), sDiskDensity(d));
    
    switch (config.type) {
            
        case DRIVE_35_DD:
            return t == DISK_35 && d == DISK_DD;
            
        case DRIVE_35_HD:
            return t == DISK_35;
            
        case DRIVE_525_DD:
            return t == DISK_525 && d == DISK_DD;
                        
        default:
            assert(false);
    }
    return false;
}

bool
Drive::isInsertable(DiskFile *file)
{
    debug(DSK_DEBUG, "isInsertable(DiskFile %p)\n", file);
   
    if (!file) return false;
    
    return isInsertable(file->getDiskType(), file->getDiskDensity());
}

bool
Drive::isInsertable(Disk *disk)
{
    debug(DSK_DEBUG, "isInsertable(Disk %p)\n", disk);
    
    if (!disk) return false;

    return isInsertable(disk->type, disk->density);
}

bool
Drive::insertDisk(Disk *disk)
{
    trace(DSK_DEBUG, "insertDisk(%p)", disk);

    if (isInsertable(disk)) {

        // Don't insert a disk if there is already one
        assert(!hasDisk());

        // Insert the disk and inform the GUI
        this->disk = disk;
        head.offset = 0;
        messageQueue.put(MSG_DISK_INSERT, nr);
        
        return true;
    }
    
    return false;
}

u64
Drive::fnv()
{
    return disk ? disk->getFnv() : 0;
}

void
Drive::PRBdidChange(u8 oldValue, u8 newValue)
{
    // -----------------------------------------------------------------
    // | /MTR  | /SEL3 | /SEL2 | /SEL1 | /SEL0 | /SIDE |  DIR  | STEP  |
    // -----------------------------------------------------------------

    bool oldMtr = oldValue & 0x80;
    bool oldSel = oldValue & (0b1000 << nr);
    bool oldStep = oldValue & 0x01;

    bool newMtr = newValue & 0x80;
    bool newSel = newValue & (0b1000 << nr);
    bool newStep = newValue & 0x01;
    
    bool newDir = newValue & 0x02;
    
    // Store a copy of the new PRB value
    prb = newValue;
    
    //
    // Drive motor
    //

    // The motor state can only change on a falling edge on the select line
    if (FALLING_EDGE(oldSel, newSel)) {
        
        // Emulate the identification shift register
        idCount = (idCount + 1) % 32;
        idBit = GET_BIT(getDriveId(), 31 - idCount);
        
        // Drive motor logic from SAE / UAE
        if (!oldMtr || !newMtr) {
            switchMotorOn();
        } else if (oldMtr) {
            switchMotorOff();
        }
        
        // debug("disk.select() sel df%d ($%08X) [$%08x, bit #%02d: %d]\n",
        //       nr, getDriveId(), getDriveId() << idCount, 31 - idCount, idBit);
    }
    
    //
    // Drive head
    //
    
    // Move head if STEP goes high and drive was selected
    if (RISING_EDGE(oldStep, newStep) && !oldSel) step(newDir);
    
    // Evaluate the side selection bit
    if (head.side != !(newValue & 0b100)) {
        // debug("Switching to side %d\n", !(newValue & 0b100));
    }
    head.side = !(newValue & 0b100);
}
