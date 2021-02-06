// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

u16
DiskController::peekDSKDATR()
{
    // TODO: Add Accessor as template parameter.
    // TODO: Use this method to read from the FIFO buffer if Accessor == AGNUS.

    // DSKDAT is a strobe register that cannot be accessed by the CPU
    return 0;
}

void
DiskController::pokeDSKLEN(u16 value)
{
    trace(DSKREG_DEBUG, "pokeDSKLEN(%X)\n", value);

    setDSKLEN(dsklen, value);
}

void
DiskController::setDSKLEN(u16 oldValue, u16 newValue)
{
    trace(DSKREG_DEBUG, "setDSKLEN(%x) [%d,%d,%d]\n",
          newValue, df0.head.cylinder, df0.head.side, df0.head.offset);

    Drive *drive = getSelectedDrive();

    dsklen = newValue;

    // Initialize checksum (for debugging only)
    if (DSK_CHECKSUM) {
        checkcnt = 0;
        check1 = fnv_1a_init32();
        check2 = fnv_1a_init32();
    }
    
    // Disable DMA if bit 15 (DMAEN) is zero
    if (!(newValue & 0x8000)) {

        setState(DRIVE_DMA_OFF);
        clearFifo();
    }
    
    // Enable DMA if bit 15 (DMAEN) has been written twice
    if (oldValue & newValue & 0x8000) {
        
        if (XFILES && state != DRIVE_DMA_OFF)
            trace("XFILES (DSKLEN): Written in DMA state %d\n", state);

        // Only proceed if there are bytes to process
        if ((dsklen & 0x3FFF) == 0) { paula.raiseIrq(INT_DSKBLK); return; }

        // In debug mode, reset head position to generate reproducable results
        if (ALIGN_HEAD && drive) drive->head.offset = 0;

        // Check if the WRITE bit (bit 14) also has been written twice
        if (oldValue & newValue & 0x4000) {
            
            setState(DRIVE_DMA_WRITE);
            clearFifo();
            
        } else {
            
            // Check the WORDSYNC bit in the ADKCON register
            if (GET_BIT(paula.adkcon, 10)) {
                
                // Wait with reading until a sync mark has been found
                setState(DRIVE_DMA_WAIT);
                clearFifo();
                
            } else {
                
                // Start reading immediately
                setState(DRIVE_DMA_READ);
                clearFifo();
            }
        }
    }
        
    // If turbo drives are emulated, perform DMA immediately
    if (turboMode()) performTurboDMA(drive);
}

void
DiskController::pokeDSKDAT(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKDAT\n");

    // TODO: Add Accessor as template parameter.
    // TODO: Use this method to fill the FIFO buffer if Accessor == AGNUS.
}

u16
DiskController::peekDSKBYTR()
{
    u16 result = computeDSKBYTR();
    
    debug(DSKREG_DEBUG, "peekDSKBYTR() = %x\n", result);
    return result;
}

u16
DiskController::computeDSKBYTR()
{
    /* 15      DSKBYT     Indicates whether this register contains valid data
     * 14      DMAON      Indicates whether disk DMA is actually enabled
     * 13      DISKWRITE  Matches the WRITE bit in DSKLEN
     * 12      WORDEQUAL  Indicates a match with the contents of DISKSYNC
     * 11 - 8             Unused
     *  7 - 0  DATA       Disk byte data
     */
    
    // DSKBYT and DATA
    u16 result = incoming;
    
    // Clear the DSKBYT bit, so it won't show up in the next read
    incoming &= 0x7FFF;
    
    // DMAON
    if (agnus.dskdma() && state != DRIVE_DMA_OFF) SET_BIT(result, 14);

    // DSKWRITE
    if (dsklen & 0x4000) SET_BIT(result, 13);
    
    // WORDEQUAL
    assert(agnus.clock >= syncCycle);
    if (agnus.clock - syncCycle <= USEC(2)) SET_BIT(result, 12);
    
    return result;
}

void
DiskController::pokeDSKSYNC(u16 value)
{
    debug(DSKREG_DEBUG, "pokeDSKSYNC(%x)\n", value);
    
    if (value != 0x4489) {
        
        trace(XFILES, "XFILES (DSKSYNC): Unusual sync mark $%04X\n", value);
        
        if (config.lockDskSync) {
            debug(DSKREG_DEBUG, "Write to DSKSYNC blocked (%x)\n", value);
            return;
        }
    }
    
    dsksync = value;
}

u8
DiskController::driveStatusFlags()
{
    u8 result = 0xFF;
    
    if (config.connected[0]) result &= df[0]->driveStatusFlags();
    if (config.connected[1]) result &= df[1]->driveStatusFlags();
    if (config.connected[2]) result &= df[2]->driveStatusFlags();
    if (config.connected[3]) result &= df[3]->driveStatusFlags();
    
    return result;
}

void
DiskController::PRBdidChange(u8 oldValue, u8 newValue)
{
    // debug("PRBdidChange: %X -> %X\n", oldValue, newValue);

    // Store a copy of the new value for reference
    prb = newValue;
    
    i8 oldSelected = selected;
    selected = -1;
    
    // Iterate over all connected drives
    for (unsigned i = 0; i < 4; i++) {
        if (!config.connected[i]) continue;
        
        // Inform the drive and determine the selected one
        df[i]->PRBdidChange(oldValue, newValue);
        if (df[i]->isSelected()) {
            selected = i;
        }
    }
        
    // Inform the GUI
    if (oldSelected != selected) {
        if (selected == -1) {
            // debug(DSKREG_DEBUG, "Deselecting df%d\n", oldSelected);
        } else {
            // debug(DSKREG_DEBUG, "Selecting df%d\n", selected);
        }
        if (selected != -1) messageQueue.put(MSG_DRIVE_SELECT, selected);
    }
}
