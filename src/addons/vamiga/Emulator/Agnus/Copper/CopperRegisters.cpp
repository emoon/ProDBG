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
Copper::pokeCOPCON(u16 value)
{
    trace(COPREG_DEBUG, "pokeCOPCON(%04X)\n", value);
    
    /* "This is a 1-bit register that when set true, allows the Copper to
     *  access the blitter hardware. This bit is cleared by power-on reset, so
     *  that the Copper cannot access the blitter hardware." [HRM]
     */
    cdang = (value & 0b10) != 0;
}

template <Accessor s> void
Copper::pokeCOPJMP1()
{
    trace(COPREG_DEBUG, "pokeCOPJMP1: Jumping to %X\n", cop1lc);

    if (s == AGNUS_ACCESS) {

        assert(false);

    }
    if (s == CPU_ACCESS) {

        if (agnus.blitter.isRunning()) {
            trace(XFILES, "pokeCOPJMP1: Blitter is running\n");
        }
        switchToCopperList(1);
    }
}

template <Accessor s> void
Copper::pokeCOPJMP2()
{
    trace(COPREG_DEBUG, "pokeCOPJMP2(): Jumping to %X\n", cop2lc);

    if (s == AGNUS_ACCESS) {

        assert(false);

    }
    if (s == CPU_ACCESS) {

        if (agnus.blitter.isRunning()) {
            trace(XFILES, "pokeCOPJMP2: Blitter is running\n");
        }
        switchToCopperList(2);
    }
}

void
Copper::pokeCOPINS(u16 value)
{
    trace(COPREG_DEBUG, "COPPC: %X pokeCOPINS(%04X)\n", coppc, value);

    /* COPINS is a dummy address that can be used to write the first or
     * the second instruction register, depending on the current state.
     */
    trace(XFILES, "Write to COPINS (%x)\n", value); 
    
    // TODO: The following is certainly wrong...
    /* if (state == COP_MOVE || state == COP_WAIT_OR_SKIP) {
        cop2ins = value;
    } else {
        cop1ins = value;
    }
    */
    cop1ins = value;
}

void
Copper::pokeCOP1LCH(u16 value)
{
    trace(COPREG_DEBUG, "pokeCOP1LCH(%04X)\n", value);

    if (HI_WORD(cop1lc) != value) {
        cop1lc = REPLACE_HI_WORD(cop1lc, value);
        cop1end = cop1lc;

        if (!activeInThisFrame && copList == 1) {
            coppc = cop1lc;
        }
    }
}

void
Copper::pokeCOP1LCL(u16 value)
{
    trace(COPREG_DEBUG, "pokeCOP1LCL(%04X)\n", value);

    value &= 0xFFFE;

    if (LO_WORD(cop1lc) != value) {
        cop1lc = REPLACE_LO_WORD(cop1lc, value);
        cop1end = cop1lc;
        
        if (!activeInThisFrame && copList == 1) {
            coppc = cop1lc;
        }
    }
}

void
Copper::pokeCOP2LCH(u16 value)
{
    trace(COPREG_DEBUG, "pokeCOP2LCH(%04X)\n", value);

    if (HI_WORD(cop2lc) != value) {
        cop2lc = REPLACE_HI_WORD(cop2lc, value);
        cop2end = cop2lc;

        if (!activeInThisFrame && copList == 2) {
            coppc = cop2lc;
        }
    }
}

void
Copper::pokeCOP2LCL(u16 value)
{
    trace(COPREG_DEBUG, "pokeCOP2LCL(%04X)\n", value);

    value &= 0xFFFE;

    if (LO_WORD(cop2lc) != value) {
        cop2lc = REPLACE_LO_WORD(cop2lc, value);
        cop2end = cop2lc;
        
        if (!activeInThisFrame && copList == 2) {
            coppc = cop2lc;
        }
    }
}

void
Copper::pokeNOOP(u16 value)
{
    trace(COPREG_DEBUG, "pokeNOOP(%04X)\n", value);
}

template void Copper::pokeCOPJMP1<CPU_ACCESS>();
template void Copper::pokeCOPJMP1<AGNUS_ACCESS>();
template void Copper::pokeCOPJMP2<CPU_ACCESS>();
template void Copper::pokeCOPJMP2<AGNUS_ACCESS>();
