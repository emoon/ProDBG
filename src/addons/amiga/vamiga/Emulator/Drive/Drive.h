// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "DriveTypes.h"
#include "AmigaComponent.h"
#include "Disk.h"

class Drive : public AmigaComponent {
    
    friend class DiskController;
        
    // Number of the emulated drive (0 = df0, 1 = df1, etc.)
    const isize nr;

    // Current configuration
    DriveConfig config;

    // Result of the latest inspection
    DriveInfo info;

    // Drive motor status (on or off)
    bool motor;
    
    // Time stamp indicating the the latest change of the motor status
    Cycle switchCycle;
    
    // Recorded motor speed at 'motorCycle' in percent
    double switchSpeed;
    
    // Position of the currently transmitted identification bit
    u8 idCount;

    // Value of the currently transmitted identification bit
    bool idBit;

    // Records when the head started to step to another cylinder
    Cycle stepCycle;
    
    /* Disk change status. This variable controls the /CHNG bit in the CIA A
     * PRA register. Note that the variable only changes its value under
     * certain circumstances. If a head movement pulse is sent and no disk is
     * inserted, the variable is set to false (which is also the reset value).
     * It becomes true when a disk is ejected.
     */
    bool dskchange;
    
    // A copy of the DSKLEN register
    u8 dsklen;
    
    // A copy of the PRB register of CIA B
    u8 prb;
    
    // The current drive head location
    DriveHead head;
    
    /* History buffer storing the most recently visited tracks. The buffer is
     * used to detect the polling head movements that are issued by track disc
     * device to detect a newly inserted disk.
     */
    u64 cylinderHistory;

public:
    
    // The currently inserted disk (nullptr if the drive is empty)
    Disk *disk = nullptr;

    
    //
    // Initializing
    //

public:

    Drive(Amiga& ref, isize nr);
    
    const char *getDescription() const override;
    long getNr() { return nr; }

private:
    
    void _reset(bool hard) override;

    
    //
    // Configuring
    //
    
public:
    
    const DriveConfig &getConfig() const { return config; }
    
    long getConfigItem(Option option) const;
    bool setConfigItem(Option option, long value) override;
    bool setConfigItem(Option option, long id, long value) override;
    
    
    //
    // Analyzing
    //

public:
    
    DriveInfo getInfo() { return HardwareComponent::getInfo(info); }

private:
    
    void _inspect() override;
    void _dump(Dump::Category category, std::ostream& os) const override;

    
    //
    // Serializing
    //

private:

    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.type
        << config.startDelay
        << config.stopDelay
        << config.stepDelay;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker

        << motor
        << switchCycle
        << switchSpeed
        << idCount
        << idBit
        << stepCycle
        << dskchange
        << dsklen
        << prb
        << head.side
        << head.cylinder
        << head.offset
        << cylinderHistory;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    isize _size() override;
    isize _load(const u8 *buffer) override;
    isize _save(u8 *buffer) override;


    //
    // Accessing
    //

public:

    // Identification mode
    bool idMode() const;
    u32 getDriveId() const;

    // Operation
    u8 getCylinder() const { return head.cylinder; }
    
    
    //
    // Handling the drive status register flags
    //
    
    // Returns true if this drive is currently selected
    inline bool isSelected() const { return (prb & (0b1000 << nr)) == 0; }
    
    u8 driveStatusFlags() const;
    

    //
    // Operating the drive
    //
        
    // Returns the current motor speed in percent
    double motorSpeed() const;

    // Turns the drive motor on or off
    bool getMotor() const { return motor; }
    void setMotor(bool value);
    void switchMotorOn() { setMotor(true); }
    void switchMotorOff() { setMotor(false); }

    // Informs about the current drive motor state
    bool motorSpeedingUp() const;
    bool motorAtFullSpeed() const;
    bool motorSlowingDown() const;
    bool motorStopped() const;

    // Selects the active drive head (0 = lower, 1 = upper)
    void selectSide(isize side);

    // Reads a value from the drive head and optionally rotates the disk
    u8 readByte() const;
    u8 readByteAndRotate();
    u16 readWordAndRotate();

    // Writes a value to the drive head and optionally rotates the disk
    void writeByte(u8 value);
    void writeByteAndRotate(u8 value);
    void writeWordAndRotate(u16 value);

    // Emulate a disk rotation (moves head to the next byte)
    void rotate();

    // Rotates the disk to the next sync mark
    void findSyncMark();

    //
    // Moving the drive head
    //

    // Returns wheather the drive is ready to accept a stepping pulse
    bool readyToStep() const;
    
    // Moves the drive head (0 = inwards, 1 = outwards).
    void step(isize dir);

    // Records a cylinder change (needed for diskPollingMode() to work)
    void recordCylinder(u8 cylinder);

    /* Returns true if the drive is in disk polling mode
     * Disk polling mode is detected by analyzing the movement history that
     * has been recorded by recordCylinder()
     */
    bool pollsForDisk() const;

    
    //
    // Handling disks
    //

    bool hasDisk() const { return disk != nullptr; }
    bool hasDDDisk() const { return disk ? disk->density == DISK_DD : false; }
    bool hasHDDisk() const { return disk ? disk->density == DISK_HD : false; }
    bool hasModifiedDisk() const { return disk ? disk->isModified() : false; }
    void setModifiedDisk(bool value) { if (disk) disk->setModified(value); }
    
    bool hasWriteEnabledDisk() const;
    bool hasWriteProtectedDisk() const;
    void setWriteProtection(bool value); 
    void toggleWriteProtection();
    
    bool isInsertable(DiskDiameter t, DiskDensity d) const;
    bool isInsertable(DiskFile *file) const;
    bool isInsertable(Disk *disk) const;

    void ejectDisk();
    bool insertDisk(Disk *disk);
    bool insertBlankDisk();

    u64 fnv() const;
    
    //
    // Delegation methods
    //
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(u8 oldValue, u8 newValue);
};
