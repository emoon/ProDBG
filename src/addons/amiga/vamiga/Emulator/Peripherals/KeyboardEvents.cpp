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
Keyboard::serviceKeyboardEvent(EventID id)
{
    u64 nr = agnus.slot[KBD_SLOT].data;

    switch(id) {
            
        case KBD_TIMEOUT:
            
            trace(KBD_DEBUG, "KBD_TIMEOUT\n");
            
            // A timeout has occured. Try to resynchronize with the Amiga.
            state = KB_SYNC;
            execute();
            break;
            
        case KBD_DAT:
            
            trace(KBD_DEBUG, "KBD_DAT [%d]\n", nr);
            
            if (nr < 8) {
                
                // Put a bit from the shift register onto the SP line
                ciaa.setSP(GET_BIT(shiftReg, 7 - nr));
                agnus.scheduleRel<KBD_SLOT>(USEC(20), KBD_CLK0);
                
            } else {
                
                // Put the SP line back to normal
                ciaa.setSP(1);
                agnus.scheduleRel<KBD_SLOT>(MSEC(143), KBD_TIMEOUT);
            }
            break;
            
        case KBD_CLK0:

            trace(KBD_DEBUG, "KBD_CLK0 [%d]\n", nr);

            // Pull the clock line low
            ciaa.emulateFallingEdgeOnCntPin();
            agnus.scheduleRel<KBD_SLOT>(USEC(20), KBD_CLK1);
            break;
            
        case KBD_CLK1:

            trace(KBD_DEBUG, "KBD_CLK1 [%d]\n", nr);

            // Pull the clock line high
            ciaa.emulateRisingEdgeOnCntPin();
            agnus.scheduleRel<KBD_SLOT>(USEC(20), KBD_DAT, nr + 1);
            break;

        case KBD_SYNC_DAT0:

            trace(KBD_DEBUG, "KBD_SYNC_DAT0\n");
            ciaa.setSP(0);
            agnus.scheduleRel<KBD_SLOT>(USEC(20), KBD_SYNC_CLK0);
            break;

        case KBD_SYNC_CLK0:

            trace(KBD_DEBUG, "KBD_SYNC_CLK0\n");
            ciaa.emulateFallingEdgeOnCntPin();
            agnus.scheduleRel<KBD_SLOT>(USEC(20), KBD_SYNC_CLK1);
            break;
            
        case KBD_SYNC_CLK1:
            
            trace(KBD_DEBUG, "KBD_SYNC_CLK1\n");
            ciaa.emulateRisingEdgeOnCntPin();
            agnus.scheduleRel<KBD_SLOT>(USEC(20), KBD_SYNC_DAT1);
            break;
            
        case KBD_SYNC_DAT1:
            
            trace(KBD_DEBUG, "KBD_SYNC_DAT1\n");
            ciaa.setSP(1);
            agnus.scheduleRel<KBD_SLOT>(MSEC(143), KBD_TIMEOUT);
            break;

        default:
            assert(false);
            break;
    }
}
