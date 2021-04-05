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
CIA::scheduleNextExecution()
{
    if (isCIAA()) {
        agnus.scheduleAbs<CIAA_SLOT>(clock + CIA_CYCLES(1), CIA_EXECUTE);
    } else {
        agnus.scheduleAbs<CIAB_SLOT>(clock + CIA_CYCLES(1), CIA_EXECUTE);
    }
}

void
CIA::scheduleWakeUp()
{
    if (isCIAA()) {
        agnus.scheduleAbs<CIAA_SLOT>(wakeUpCycle, CIA_WAKEUP);
    } else {
        agnus.scheduleAbs<CIAB_SLOT>(wakeUpCycle, CIA_WAKEUP);
    }
}
