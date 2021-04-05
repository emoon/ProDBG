// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _EVENT_H
#define _EVENT_H

struct Event
{
    // Indicates when the event is due
    Cycle triggerCycle;

    // The event identifier
    EventID id;

    // An optional data value
    i64 data;

    template <class T>
    void applyToItems(T& worker)
    {
        worker

        & triggerCycle
        & id
        & data;
    }
};

#endif
