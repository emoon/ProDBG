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
Blitter::pokeBLTCON0(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTCON0(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(2), SET_BLTCON0, value);
}

void
Blitter::setBLTCON0(u16 value)
{
    trace(BLT_GUARD && running, "BLTCON0 written while Blitter is running\n");

    bltcon0 = value;
}

void
Blitter::pokeBLTCON0L(u16 value)
{
    debug(MAX(BLTREG_DEBUG, ECSREG_DEBUG), "pokeBLTCON0L(%X)\n", value);

    // ECS only register
    if (agnus.isOCS()) return;

    agnus.recordRegisterChange(DMA_CYCLES(2), SET_BLTCON0L, value);
}

void
Blitter::setBLTCON0L(u16 value)
{
    trace(BLT_GUARD && running, "BLTCON0L written while Blitter is running\n");

    bltcon0 = HI_LO(HI_BYTE(bltcon0), LO_BYTE(value));
}

void
Blitter::setBLTCON0ASH(u16 ash)
{
    assert(ash <= 0xF);
    
    bltcon0 = (bltcon0 & 0x0FFF) | (ash << 12);
}

void
Blitter::pokeBLTCON1(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTCON1(%X)\n", value);
    agnus.recordRegisterChange(DMA_CYCLES(2), SET_BLTCON1, value);
}

void
Blitter::setBLTCON1(u16 value)
{
    trace(BLT_GUARD && running, "BLTCON1 written while Blitter is running\n");

    bltcon1 = value;
}

void
Blitter::setBLTCON1BSH(u16 bsh)
{
    assert(bsh <= 0xF);
    
    bltcon1 = (bltcon1 & 0x0FFF) | (bsh << 12);
}

void
Blitter::pokeBLTAPTH(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTAPTH(%X)\n", value);
    trace(BLT_GUARD && running, "BLTAPTH written while Blitter is running\n");

    bltapt = REPLACE_HI_WORD(bltapt, value);

    if (bltapt & ~agnus.ptrMask) {
        trace(BLT_GUARD, "BLTAPT out of range: %x\n", bltapt);
    }
}

void
Blitter::pokeBLTAPTL(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTAPTL(%X)\n", value);
    trace(BLT_GUARD && running, "BLTAPTL written while Blitter is running\n");

    bltapt = REPLACE_LO_WORD(bltapt, value & 0xFFFE);
}

void
Blitter::pokeBLTBPTH(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTBPTH(%X)\n", value);
    trace(BLT_GUARD && running, "BLTBPTH written while Blitter is running\n");

    bltbpt = REPLACE_HI_WORD(bltbpt, value);
    
    if (bltbpt & ~agnus.ptrMask) {
        trace(BLT_GUARD, "BLTBPT out of range: %x\n", bltbpt);
    }
}

void
Blitter::pokeBLTBPTL(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTBPTL(%X)\n", value);
    trace(BLT_GUARD && running, "BLTBPTL written while Blitter is running\n");

    bltbpt = REPLACE_LO_WORD(bltbpt, value & 0xFFFE);
}

void
Blitter::pokeBLTCPTH(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTCPTH(%X)\n", value);
    trace(BLT_GUARD && running, "BLTCPTH written while Blitter is running\n");

    bltcpt = REPLACE_HI_WORD(bltcpt, value);
    
    if (bltcpt & ~agnus.ptrMask) {
        trace(BLT_GUARD, "BLTCPT out of range: %x\n", bltcpt);
    }
}

void
Blitter::pokeBLTCPTL(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTCPTL(%X)\n", value);
    trace(BLT_GUARD && running, "BLTCPTL written while Blitter is running\n");

    bltcpt = REPLACE_LO_WORD(bltcpt, value & 0xFFFE);
}

void
Blitter::pokeBLTDPTH(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTDPTH(%X)\n", value);
    trace(BLT_GUARD && running, "BLTDPTH written while Blitter is running\n");

    bltdpt = REPLACE_HI_WORD(bltdpt, value);
    
    if (bltdpt & ~agnus.ptrMask) {
        trace(BLT_GUARD, "BLTDPT out of range: %x\n", bltdpt);
    }
}

void
Blitter::pokeBLTDPTL(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTDPTL(%X)\n", value);
    trace(BLT_GUARD && running, "BLTDPTL written while Blitter is running\n");

    bltdpt = REPLACE_LO_WORD(bltdpt, value & 0xFFFE);
}

void
Blitter::pokeBLTAFWM(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTAFWM(%X)\n", value);
    trace(BLT_GUARD && running, "BLTAFWM written while Blitter is running\n");

    bltafwm = value;
}

void
Blitter::pokeBLTALWM(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTALWM(%X)\n", value);
    trace(BLT_GUARD && running, "BLTALWM written while Blitter is running\n");

    bltalwm = value;
}

template <Accessor s> void
Blitter::pokeBLTSIZE(u16 value)
{
    debug(BLTTIM_DEBUG, "(%d,%d) BLTSIZE(%x)\n", agnus.pos.v, agnus.pos.h, value);
    debug(BLTREG_DEBUG, "pokeBLTSIZE(%X)\n", value);

    if (s == AGNUS_ACCESS) {
        agnus.recordRegisterChange(DMA_CYCLES(1), SET_BLTSIZE, value);
    } else {
        blitter.setBLTSIZE(value);
    }
}

void
Blitter::setBLTSIZE(u16 value)
{
    debug(BLTREG_DEBUG, "setBLTSIZE(%X)\n", value);
    trace(BLT_GUARD && running, "BLTSIZE written while Blitter is running\n");

    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // h9 h8 h7 h6 h5 h4 h3 h2 h1 h0 w5 w4 w3 w2 w1 w0
    bltsizeV = value >> 6;
    bltsizeH = value & 0x3F;

    // Overwrite with default values if zero
    if (!bltsizeV) bltsizeV = 0x0400;
    if (!bltsizeH) bltsizeH = 0x0040;

    // Warn if the previous Blitter operation is overwritten
    if (agnus.hasEvent<BLT_SLOT>()) {
        trace(XFILES, "XFILES: Overwriting Blitter event %d\n", agnus.slot[BLT_SLOT].id);
        // EXPERIMENTAL
        // endBlit();
    }
    
    agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_STRT1);
}

void
Blitter::pokeBLTSIZV(u16 value)
{
    debug(MAX(BLTREG_DEBUG, ECSREG_DEBUG), "pokeBLTSIZV(%X)\n", value);

    // ECS only register
    if (agnus.isOCS()) return;

    agnus.recordRegisterChange(DMA_CYCLES(2), SET_BLTSIZV, value);
}

void
Blitter::setBLTSIZV(u16 value)
{
    trace(BLT_GUARD && running, "BLTSIZV written while Blitter is running\n");

    // 15  14  13  12  11  10 09 08 07 06 05 04 03 02 01 00
    //  0 h14 h13 h12 h11 h10 h9 h8 h7 h6 h5 h4 h3 h2 h1 h0
    bltsizeV = value & 0x7FFF;
}

void
Blitter::pokeBLTSIZH(u16 value)
{
    debug(MAX(BLTREG_DEBUG, ECSREG_DEBUG), "pokeBLTSIZH(%X)\n", value);

    // ECS only register
    if (agnus.isOCS()) return;

    trace(BLT_GUARD && running, "BLTSIZH written while Blitter is running\n");

    // 15  14  13  12  11  10 09 08 07 06 05 04 03 02 01 00
    //  0   0   0   0   0 w10 w9 w8 w7 w6 w5 w4 w3 w2 w1 w0
    bltsizeH = value & 0x07FF;

    // Overwrite with default values if zero
    if (!bltsizeV) bltsizeV = 0x8000;
    if (!bltsizeH) bltsizeH = 0x0800;

    agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_STRT1);
}

void
Blitter::pokeBLTAMOD(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTAMOD(%X)\n", value);
    trace(BLT_GUARD && running, "BLTAMOD written while Blitter is running\n");

    bltamod = (i16)(value & 0xFFFE);
}
void
Blitter::pokeBLTBMOD(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTBMOD(%X)\n", value);
    trace(BLT_GUARD && running, "BLTBMOD written while Blitter is running\n");

    bltbmod = (i16)(value & 0xFFFE);
}

void
Blitter::pokeBLTCMOD(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTCMOD(%X)\n", value);
    trace(BLT_GUARD && running, "BLTCMOD written while Blitter is running\n");

    bltcmod = (i16)(value & 0xFFFE);
}

void
Blitter::pokeBLTDMOD(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTDMOD(%X)\n", value);
    trace(BLT_GUARD && running, "BLTDMOD written while Blitter is running\n");

    bltdmod = (i16)(value & 0xFFFE);
}

void
Blitter::pokeBLTADAT(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTADAT(%X)\n", value);
    trace(BLT_GUARD && running, "BLTADAT written while Blitter is running\n");

    anew = value;
}

void
Blitter::pokeBLTBDAT(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTBDAT(%X)\n", value);
    trace(BLT_GUARD && running, "BLTBDAT written while Blitter is running\n");

    bnew = value;
}

void
Blitter::pokeBLTCDAT(u16 value)
{
    debug(BLTREG_DEBUG, "pokeBLTCDAT(%X)\n", value);
    trace(BLT_GUARD && running, "BLTCDAT written while Blitter is running\n");

    chold = value;
}

void
Blitter::pokeDMACON(u16 oldValue, u16 newValue)
{
    bool oldBltDma = (oldValue & (DMAEN | BLTEN)) == (DMAEN | BLTEN);
    bool newBltDma = (newValue & (DMAEN | BLTEN)) == (DMAEN | BLTEN);
    
    // Check if Blitter DMA got switched on
    if (!oldBltDma && newBltDma) {

        // Perform pending blit operation (if any)
        if (agnus.hasEvent<BLT_SLOT>(BLT_STRT1)) {
            agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(0), BLT_STRT1);
        }
    }
    
    if (oldBltDma && !newBltDma) {
        trace(BLT_GUARD && running, "Blitter DMA off while Blitter is running\n");
    }
    if (agnus.bltpri(oldValue) != agnus.bltpri(newValue)) {
        trace(BLT_GUARD && running, "BLTPRI changed while Blitter is running\n");
    }
}

template void Blitter::pokeBLTSIZE<CPU_ACCESS>(u16 value);
template void Blitter::pokeBLTSIZE<AGNUS_ACCESS>(u16 value);
