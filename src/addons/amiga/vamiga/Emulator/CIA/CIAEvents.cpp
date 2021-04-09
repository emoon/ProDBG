// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "CIA.h"
#include "Agnus.h"

void
CIA::scheduleNextExecution()
{
    if (isCIAA()) {
        agnus.scheduleAbs<SLOT_CIAA>(clock + CIA_CYCLES(1), CIA_EXECUTE);
    } else {
        agnus.scheduleAbs<SLOT_CIAB>(clock + CIA_CYCLES(1), CIA_EXECUTE);
    }
}

void
CIA::scheduleWakeUp()
{
    if (isCIAA()) {
        agnus.scheduleAbs<SLOT_CIAA>(wakeUpCycle, CIA_WAKEUP);
    } else {
        agnus.scheduleAbs<SLOT_CIAB>(wakeUpCycle, CIA_WAKEUP);
    }
}
