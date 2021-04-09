// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"
#include "EventTypes.h"

// Time stamp used for messages that never trigger
#define NEVER INT64_MAX

struct Event
{
    // Indicates when the event is due
    Cycle triggerCycle;

    // The event identifier
    EventID id;

    // An optional data value
    i64 data;

    template <class W>
    void operator<<(W& worker)
    {
        worker

        << triggerCycle
        << id
        << data;
    }
};
