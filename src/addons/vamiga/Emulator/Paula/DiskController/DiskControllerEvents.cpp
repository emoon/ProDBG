// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
DiskController::serviceDiskEvent()
{        
    // Receive next byte from the selected drive
    executeFifo();
    
    // Schedule next event
    scheduleNextDiskEvent();
}

void
DiskController::scheduleFirstDiskEvent()
{
    dskEventDelay = 0.0;
    
    if (turboMode()) {
        agnus.cancel<DSK_SLOT>();
    } else {
        agnus.scheduleImm<DSK_SLOT>(DSK_ROTATE);
    }
}

void
DiskController::scheduleNextDiskEvent()
{
    /* Advance the delay counter to achieve a disk rotation speed of 300rpm.
     * Rotation speed can be measured with AmigaTestKit.adf which calculates
     * the delay between consecutive index pulses. 300rpm corresponds to a
     * index pulse delay of 200ms.
     */
    dskEventDelay += 55.98;
    DMACycle rounded = DMACycle(round(dskEventDelay));
    dskEventDelay -= rounded;
    
    if (turboMode()) {
        agnus.cancel<DSK_SLOT>();
    } else {
        agnus.scheduleRel<DSK_SLOT>(DMA_CYCLES(rounded), DSK_ROTATE);
    }
}

void
DiskController::serviceDiskChangeEvent()
{
    if (agnus.slot[DCH_SLOT].id == EVENT_NONE) return;
    
    int n = (int)agnus.slot[DCH_SLOT].data;
    assert(n >= 0 && n <= 3);

    switch (agnus.slot[DCH_SLOT].id) {

        case DCH_INSERT:

            trace(DSK_DEBUG, "DCH_INSERT (df%d)\n", n);

            assert(diskToInsert != NULL);
            df[n]->insertDisk(diskToInsert);
            diskToInsert = NULL;
            break;

        case DCH_EJECT:

            trace(DSK_DEBUG, "DCH_EJECT (df%d)\n", n);

            df[n]->ejectDisk();
            break;

        default:
            assert(false);
    }

    agnus.cancel<DCH_SLOT>();
}
