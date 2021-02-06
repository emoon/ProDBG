// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

template <int nr> void
StateMachine<nr>::serviceEvent()
{
    const EventSlot slot = (EventSlot)(CH0_SLOT+nr);

    trace(AUD_DEBUG, "CHX_PERFIN state = %d\n", state);
    assert(agnus.slot[slot].id == CHX_PERFIN);

    switch (state) {

        case 0b010:

            move_010_011();
            return;

        case 0b011:

            (AUDxON() || !AUDxIP()) ? move_011_010() : move_011_000();
            return;

        default:
            assert(false);
    }
}

template void StateMachine<0>::serviceEvent();
template void StateMachine<1>::serviceEvent();
template void StateMachine<2>::serviceEvent();
template void StateMachine<3>::serviceEvent();
