// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

template <isize nr> void
StateMachine<nr>::pokeAUDxLEN(u16 value)
{
    trace(AUDREG_DEBUG, "pokeAUD%luLEN(%X)\n", nr, value);

    audlenLatch = value;
}

template <isize nr> void
StateMachine<nr>::pokeAUDxPER(u16 value)
{
    trace(AUDREG_DEBUG, "pokeAUD%luPER(%X)\n", nr, value);
    
    audperLatch = value;
}

template <isize nr> void
StateMachine<nr>::pokeAUDxVOL(u16 value)
{
    trace(AUDREG_DEBUG, "pokeAUD%luVOL(%X)\n", nr, value);

    // 1. Only the lowest 7 bits are evaluated
    // 2. All values greater than 64 are treated as 64 (max volume)
    audvolLatch = MIN(value & 0x7F, 64);
}

template <isize nr> void
StateMachine<nr>::pokeAUDxDAT(u16 value)
{
    trace(AUDREG_DEBUG, "pokeAUD%luDAT(%X)\n", nr, value);
    
    auddat = value;
    enablePenlo = enablePenhi = true;
    
    if (AUDxON()) {
        
        // DMA mode
        switch(state) {
                
            case 0b000:
                
                move_000_001();
                break;
                
            case 0b001:
                
                move_001_101();
                break;
                
            case 0b101:
                
                move_101_010();
                break;
                
            case 0b010:
            case 0b011:
                
                if (!lenfin()) {
                    lencount();
                } else {
                    lencntrld();
                    AUDxDSR();
                    intreq2 = true;
                }
                break;
        }
        
    } else {
        
        // IRQ mode
        switch(state) {
                
            case 0b000:
                
                if (!AUDxIP()) move_000_010();
                break;
        }
    }
}

template void StateMachine<0>::pokeAUDxLEN(u16 value);
template void StateMachine<1>::pokeAUDxLEN(u16 value);
template void StateMachine<2>::pokeAUDxLEN(u16 value);
template void StateMachine<3>::pokeAUDxLEN(u16 value);

template void StateMachine<0>::pokeAUDxPER(u16 value);
template void StateMachine<1>::pokeAUDxPER(u16 value);
template void StateMachine<2>::pokeAUDxPER(u16 value);
template void StateMachine<3>::pokeAUDxPER(u16 value);

template void StateMachine<0>::pokeAUDxVOL(u16 value);
template void StateMachine<1>::pokeAUDxVOL(u16 value);
template void StateMachine<2>::pokeAUDxVOL(u16 value);
template void StateMachine<3>::pokeAUDxVOL(u16 value);

template void StateMachine<0>::pokeAUDxDAT(u16 value);
template void StateMachine<1>::pokeAUDxDAT(u16 value);
template void StateMachine<2>::pokeAUDxDAT(u16 value);
template void StateMachine<3>::pokeAUDxDAT(u16 value);
