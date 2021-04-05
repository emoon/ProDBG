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
Denise::pokeDMACON(u16 oldValue, u16 newValue)
{
    if (Agnus::bpldma(newValue)) {

        // Bitplane DMA on
        trace(DMA_DEBUG, "Bitplane DMA switched on\n");

    } else {

        // Bitplane DMA off
        trace(DMA_DEBUG, "Bitplane DMA switched off\n");
    }
}

u16
Denise::peekJOY0DATR()
{
    u16 result = amiga.controlPort1.joydat();
    trace(JOYREG_DEBUG, "peekJOY0DATR() = $%04X (%d)\n", result, result);

    return result;
}

u16
Denise::peekJOY1DATR()
{
    u16 result = amiga.controlPort2.joydat();
    trace(JOYREG_DEBUG, "peekJOY1DATR() = $%04X (%d)\n", result, result);

    return result;
}

void
Denise::pokeJOYTEST(u16 value)
{
    trace(JOYREG_DEBUG, "pokeJOYTEST(%04X)\n", value);

    amiga.controlPort1.pokeJOYTEST(value);
    amiga.controlPort2.pokeJOYTEST(value);
}

u16
Denise::peekDENISEID()
{
    u16 result;

    if (config.revision == DENISE_ECS) {
        result = 0xFFFC;                           // ECS
    } else {
        result = mem.peekCustomFaulty16(0xDFF07C); // OCS
    }

    trace(ECSREG_DEBUG, "peekDENISEID() = $%04X (%d)\n", result, result);
    return result;
}

void
Denise::pokeBPLCON0(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON0(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), SET_BPLCON0_DENISE, value);
}

void
Denise::setBPLCON0(u16 oldValue, u16 newValue)
{
    trace(BPLREG_DEBUG, "setBPLCON0(%X,%X)\n", oldValue, newValue);

    // Record the register change
    i64 pixel = MAX(4 * agnus.pos.h - 4, 0);
    conChanges.insert(pixel, RegChange { SET_BPLCON0_DENISE, newValue });
    
    if (ham(oldValue) ^ ham(newValue)) {
        pixelEngine.colChanges.insert(pixel, RegChange { BPLCON0, newValue } );
    }
    
    // Update value
    bplcon0 = newValue;
    
    // Update border color index, because the ECSENA bit might have changed
    updateBorderColor();
    
    // Report a suspicious BPU value
    if (((bplcon0 >> 12) & 0b111) > (hires(bplcon0) ? 5 : 7)) {
        trace(XFILES, "XFILES (BPLCON0): BPU = %d\n", (bplcon0 >> 12) & 0b111);
    }
}

void
Denise::pokeBPLCON1(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON1(%X)\n", value);

    // Record the register change
    agnus.recordRegisterChange(DMA_CYCLES(1), SET_BPLCON1_DENISE, value);
}

void
Denise::setBPLCON1(u16 value)
{
    trace(BPLREG_DEBUG, "setBPLCON1(%X)\n", value);

    bplcon1 = value & 0xFF;

    pixelOffsetOdd  = (bplcon1 & 0b00000001) << 1;
    pixelOffsetEven = (bplcon1 & 0b00010000) >> 3;
}

void
Denise::pokeBPLCON2(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON2(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), SET_BPLCON2, value);
}

void
Denise::setBPLCON2(u16 value)
{
    trace(BPLREG_DEBUG, "setBPLCON2(%X)\n", value);

    bplcon2 = value;

    // Record the pixel coordinate where the change takes place
    conChanges.insert(4 * agnus.pos.h + 4, RegChange { SET_BPLCON2, value });
}

void
Denise::pokeBPLCON3(u16 value)
{
    trace(BPLREG_DEBUG, "pokeBPLCON3(%X)\n", value);

    agnus.recordRegisterChange(DMA_CYCLES(1), SET_BPLCON3, value);
}

void
Denise::setBPLCON3(u16 value)
{
    trace(BPLREG_DEBUG, "setBPLCON3(%X)\n", value);

    bplcon3 = value;
    
    // Update border color index, because the BRDRBLNK bit might have changed
    updateBorderColor();
}

u16
Denise::peekCLXDAT()
{
    u16 result = clxdat | 0x8000;
    clxdat = 0;
    
    trace(CLXREG_DEBUG, "peekCLXDAT() = %x\n", result);
    return result;
}

void
Denise::pokeCLXCON(u16 value)
{
    trace(CLXREG_DEBUG, "pokeCLXCON(%x)\n", value);
    clxcon = value;
}

template <int x, Accessor s> void
Denise::pokeBPLxDAT(u16 value)
{
    assert(x < 6);
    trace(BPLREG_DEBUG, "pokeBPL%dDAT(%X)\n", x + 1, value);

    if (s == AGNUS_ACCESS) {
        /*
        debug("BPL%dDAT written by Agnus (%x)\n", x, value);
        */
    }
    
    setBPLxDAT<x>(value);
}

template <int x> void
Denise::setBPLxDAT(u16 value)
{
    assert(x < 6);
    trace(BPLDAT_DEBUG, "setBPL%dDAT(%X)\n", x + 1, value);
        
    bpldat[x] = value;

    if (x == 0) {
        if (hires()) {
            denise.fillShiftRegisters(agnus.ddfHires.inRangeOdd(agnus.pos.h),
                                      agnus.ddfHires.inRangeEven(agnus.pos.h));
        } else {
            denise.fillShiftRegisters(agnus.ddfLores.inRangeOdd(agnus.pos.h),
                                      agnus.ddfLores.inRangeEven(agnus.pos.h));
        }
    }
}

template <int x> void
Denise::pokeSPRxPOS(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%dPOS(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Ex = VSTART)
    // E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Hx = HSTART)

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0POS + x, value } );
}

template <int x> void
Denise::pokeSPRxCTL(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%dCTL(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)
    
    // Disarm the sprite
    // CLR_BIT(armed, x);

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0CTL + x, value } );
}

template <int x> void
Denise::pokeSPRxDATA(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%dDATA(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;
    
    // Remember that the sprite was armed at least once in this rasterline
    SET_BIT(wasArmed, x);

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0DATA + x, value } );
}

template <int x> void
Denise::pokeSPRxDATB(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%dDATB(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0DATB + x, value });
}

template <Accessor s, int xx> void
Denise::pokeCOLORxx(u16 value)
{
    trace(COLREG_DEBUG, "pokeCOLOR%02d(%X)\n", xx, value);

    u32 reg = 0x180 + 2*xx;
    i16 pos = agnus.pos.h;

    // If the CPU modifies color, the change takes effect one DMA cycle earlier
    if (s != AGNUS_ACCESS && agnus.pos.h != 0) pos--;
    
    // Record the color change
    pixelEngine.colChanges.insert(4 * pos, RegChange { reg, value } );
}

template void Denise::pokeBPLxDAT<0,CPU_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<0,AGNUS_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<1,CPU_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<1,AGNUS_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<2,CPU_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<2,AGNUS_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<3,CPU_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<3,AGNUS_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<4,CPU_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<4,AGNUS_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<5,CPU_ACCESS>(u16 value);
template void Denise::pokeBPLxDAT<5,AGNUS_ACCESS>(u16 value);

template void Denise::setBPLxDAT<0>(u16 value);
template void Denise::setBPLxDAT<1>(u16 value);
template void Denise::setBPLxDAT<2>(u16 value);
template void Denise::setBPLxDAT<3>(u16 value);
template void Denise::setBPLxDAT<4>(u16 value);
template void Denise::setBPLxDAT<5>(u16 value);

template void Denise::pokeSPRxPOS<0>(u16 value);
template void Denise::pokeSPRxPOS<1>(u16 value);
template void Denise::pokeSPRxPOS<2>(u16 value);
template void Denise::pokeSPRxPOS<3>(u16 value);
template void Denise::pokeSPRxPOS<4>(u16 value);
template void Denise::pokeSPRxPOS<5>(u16 value);
template void Denise::pokeSPRxPOS<6>(u16 value);
template void Denise::pokeSPRxPOS<7>(u16 value);

template void Denise::pokeSPRxCTL<0>(u16 value);
template void Denise::pokeSPRxCTL<1>(u16 value);
template void Denise::pokeSPRxCTL<2>(u16 value);
template void Denise::pokeSPRxCTL<3>(u16 value);
template void Denise::pokeSPRxCTL<4>(u16 value);
template void Denise::pokeSPRxCTL<5>(u16 value);
template void Denise::pokeSPRxCTL<6>(u16 value);
template void Denise::pokeSPRxCTL<7>(u16 value);

template void Denise::pokeSPRxDATA<0>(u16 value);
template void Denise::pokeSPRxDATA<1>(u16 value);
template void Denise::pokeSPRxDATA<2>(u16 value);
template void Denise::pokeSPRxDATA<3>(u16 value);
template void Denise::pokeSPRxDATA<4>(u16 value);
template void Denise::pokeSPRxDATA<5>(u16 value);
template void Denise::pokeSPRxDATA<6>(u16 value);
template void Denise::pokeSPRxDATA<7>(u16 value);

template void Denise::pokeSPRxDATB<0>(u16 value);
template void Denise::pokeSPRxDATB<1>(u16 value);
template void Denise::pokeSPRxDATB<2>(u16 value);
template void Denise::pokeSPRxDATB<3>(u16 value);
template void Denise::pokeSPRxDATB<4>(u16 value);
template void Denise::pokeSPRxDATB<5>(u16 value);
template void Denise::pokeSPRxDATB<6>(u16 value);
template void Denise::pokeSPRxDATB<7>(u16 value);

template void Denise::pokeCOLORxx<CPU_ACCESS, 0>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 0>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 1>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 1>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 2>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 2>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 3>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 3>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 4>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 4>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 5>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 5>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 6>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 6>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 7>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 7>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 8>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 8>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 9>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 9>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 10>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 10>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 11>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 11>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 12>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 12>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 13>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 13>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 14>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 14>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 15>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 15>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 16>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 16>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 17>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 17>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 18>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 18>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 19>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 19>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 20>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 20>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 21>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 21>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 22>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 22>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 23>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 23>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 24>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 24>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 25>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 25>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 26>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 26>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 27>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 27>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 28>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 28>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 29>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 29>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 30>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 30>(u16 value);
template void Denise::pokeCOLORxx<CPU_ACCESS, 31>(u16 value);
template void Denise::pokeCOLORxx<AGNUS_ACCESS, 31>(u16 value);
