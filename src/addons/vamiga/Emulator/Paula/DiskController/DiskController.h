// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"

class DiskController : public AmigaComponent {

    friend class Drive;
    
    // Current configuration
    DiskControllerConfig config;

    // Result of the latest inspection
    DiskControllerInfo info;

    // Temorary storage for a disk waiting to be inserted
    class Disk *diskToInsert = nullptr;

    // The currently selected drive (-1 if no drive is selected)
    i8 selected = -1;

    // The current drive state (off, read, or write)
    DriveState state;

    // Timestamp of the latest DSKSYNC match
    Cycle syncCycle;

    /* Watchdog counter for SYNC marks. This counter is incremented after each
     * disk rotation and reset when a SYNC mark was found. It is used to
     * implement the auto DSKSYNC feature which forces the DSKSYNC interrupt to
     * trigger even if no SYNC mark is present.
     */
    i16 syncCounter = 0;
    
    // Used to synchronize the schedulign of the DSK_ROTATE event
    double dskEventDelay = 0;
    
    
    //
    // Data buffers
    //
    
    // The latest incoming byte (value shows up in DSKBYTER)
    u16 incoming;
        
    /* The drive controller's FIFO buffer. On each DSK_ROTATE event, a byte is
     * read from the selected drive and put into this buffer. Each Disk DMA
     * operation will read two bytes from the buffer and store them at the
     * desired location.
     */
    u64 fifo;
    
    // Number of bytes stored in the FIFO buffer
    u8 fifoCount;
    
    
    //
    // Registers
    //
    
    // Disk DMA block length
    u16 dsklen;
    
    // Disk SYNC word
    u16 dsksync;
    
    // A copy of the PRB register of CIA B
    u8 prb;
    
    
    //
    // Debugging
    //
    
    // For debugging, a FNV-32 checksum is computed for each DMA operation
    u32 check1;
    u32 check2;
    u64 checkcnt;


    //
    // Initializing
    //
    
public:
    
    DiskController(Amiga& ref);

    const char *getDescription() const override { return "DiskController"; }
    
    void _reset(bool hard) override;
    
    
    //
    // Configuring
    //
    
public:
    
    const DiskControllerConfig &getConfig() const { return config; }
    bool turboMode() const { return config.speed == -1; }

    long getConfigItem(Option option) const;
    long getConfigItem(Option option, long id) const;
    
    bool setConfigItem(Option option, long value) override;
    bool setConfigItem(Option option, long id, long value) override;

private:
    
    void _dumpConfig() const override;

        
    //
    // Analyzing
    //
    
public:
    
    DiskControllerInfo getInfo() { return HardwareComponent::getInfo(info); }
    
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
        worker

        & config.connected
        & config.speed
        & config.lockDskSync
        & config.autoDskSync;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & selected
        & state
        & syncCycle
        & syncCounter
        & dskEventDelay
        & incoming
        & fifo
        & fifoCount
        & dsklen
        & dsksync
        & prb;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Accessing
    //

public:
    
    // Returns the number of the currently selected drive
    i8 getSelected() const { return selected; }

    // Returns the currently selected (nullptr if none is selected)
    // TODO: Return reference
    class Drive *getSelectedDrive();

    // Indicates if the motor of the specified drive is switched on
    bool spinning(isize driveNr) const;

    // Indicates if the motor of at least one drive is switched on
    bool spinning() const;
    
    // Returns the current drive state
    DriveState getState() const { return state; }
    
private:
    
    // Changes the current drive state
    void setState(DriveState s);
    void setState(DriveState oldState, DriveState newState);

    
    //
    // Accessing registers
    //
    
public:
    
    // OCR register 0x008 (r)
    u16 peekDSKDATR() const;
    
    // OCR register 0x024 (w)
    void pokeDSKLEN(u16 value);
    void setDSKLEN(u16 oldValue, u16 newValue);
    
    // OCR register 0x026 (w)
    void pokeDSKDAT(u16 value);
    
    // OCR register 0x01A (r)
    u16 peekDSKBYTR();
    u16 computeDSKBYTR();
    
    // OCR register 0x07E (w)
    void pokeDSKSYNC(u16 value);
    
    // Read handler for the PRA register of CIA A
    u8 driveStatusFlags() const;
    
    // Write handler for the PRB register of CIA B
    void PRBdidChange(u8 oldValue, u8 newValue);
    
    
    //
    // Handling disks
    //

    // Ejects a disk from the specified drive
    void ejectDisk(isize nr, Cycle delay = 0);

    // Inserts a disk into the specified drive
    void insertDisk(class Disk *disk, isize nr, Cycle delay = 0);
    void insertDisk(class DiskFile *file, isize nr, Cycle delay = 0);
    
    // Write protects or unprotects a disk
    void setWriteProtection(isize nr, bool value);

        
    //
    // Serving events
    //
    
public:

    // Services an event in the disk controller slot
    void serviceDiskEvent();

    // Schedules the first or next event in the disk controller slot
    void scheduleFirstDiskEvent();
    void scheduleNextDiskEvent();

    // Services an event in the disk change slot
    void serviceDiskChangeEvent();

    
    //
    // Working with the FIFO buffer
    //
    
private:
    
    // Informs about the current FIFO fill state
    bool fifoIsEmpty() const { return fifoCount == 0; }
    bool fifoIsFull() const { return fifoCount == 6; }
    bool fifoHasWord() const { return fifoCount >= 2; }
    bool fifoCanStoreWord() const { return fifoCount <= 4; }

    // Clears the FIFO buffer
    void clearFifo();
    
    // Reads a byte or word from the FIFO buffer
    u8 readFifo();
    u16 readFifo16();

    // Writes a word into the FIFO buffer
    void writeFifo(u8 byte);

    
    // Returns true if the next word to read matches the specified value
    bool compareFifo(u16 word) const;

    /* Emulates a data transfert between the selected drive and the FIFO
     * buffer. This function is executed periodically in serviceDiskEvent().
     * The exact operation is dependent of the current DMA state. If DMA is
     * off, no action is taken. If a read mode is active, the FIFO is filled
     * with data from the drive. If a write mode is active, data from the FIFO
     * is written to the drive head.
     */
    void executeFifo();
         
    
    //
    // Performing DMA
    //

public:

    /* The emulator supports two basic disk DMA modes:
     *
     *     1. Standard DMA mode    (more compatible, but slow)
     *     2. Turbo DMA mode       (fast, but less compatible)
     *
     * In standard DMA mode, performDMA() is invoked three times per raster
     * line, in each of the three DMA slots. Communication with the drive is
     * managed by a FIFO buffer. Data is never read directly from or written
     * to the drive. It is always exchanged via the FIFO.
     *
     * The FIFO buffer supports two emulation modes:
     *
     *     1. Asynchronous        (more compatible)
     *     2. Synchronous         (less compatibile)
     *
     * If the FIFO buffer is emulated asynchronously, the event scheduler
     * is utilized to execute a DSK_ROTATE event from time to time. Whenever
     * this event triggers, a byte is read from the disk drive and fed into
     * the buffer. If the FIFO buffer is emulated synchronously, the DSK_ROTATE
     * events have no effect. Instead, the FIFO buffer is filled at the same
     * time when the drive DMA slots are processed. Synchronous mode is
     * slightly faster, because the FIFO can never run out of data. It is filled
     * exactly at the time when data is needed.
     *
     * To speed up emulation, standard drives can be run with an acceleration
     * factor greater than 1. In this case, multiple words are transferred
     * in each disk drive DMA slot. The first word is taken from the Fifo as
     * usual. All other words are emulated on-the-fly, with the same mechanism
     * as used in synchronous Fifo mode.
     *
     * Turbo DMA is applied iff the drive is configured as a turbo drive.
     * In this mode, data is transferred immediately when the DSKLEN
     * register is written to. This mode is fast, but far from being accurate.
     * Neither does it uses the disk DMA slots, nor does it interact with
     * the FIFO buffer.

     */
  
    // Performs DMA in standard mode
    void performDMA();
    void performDMARead(Drive *drive, u32 count);
    void performDMAWrite(Drive *drive, u32 count);
     
    // Performs DMA in turbo mode
    void performTurboDMA(Drive *d);
    void performTurboRead(Drive *drive);
    void performTurboWrite(Drive *drive);
};
