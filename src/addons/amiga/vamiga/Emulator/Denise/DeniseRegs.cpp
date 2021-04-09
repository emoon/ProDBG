// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Denise.h"
#include "Agnus.h"
#include "ControlPort.h"

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
    u16 result = controlPort1.joydat();
    trace(JOYREG_DEBUG, "peekJOY0DATR() = $%04X (%d)\n", result, result);

    return result;
}

u16
Denise::peekJOY1DATR()
{
    u16 result = controlPort2.joydat();
    trace(JOYREG_DEBUG, "peekJOY1DATR() = $%04X (%d)\n", result, result);

    return result;
}

void
Denise::pokeJOYTEST(u16 value)
{
    trace(JOYREG_DEBUG, "pokeJOYTEST(%04X)\n", value);

    controlPort1.pokeJOYTEST(value);
    controlPort2.pokeJOYTEST(value);
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
    i64 pixel = std::max(4 * agnus.pos.h - 4, 0);
    conChanges.insert(pixel, RegChange { SET_BPLCON0_DENISE, newValue });
    
    // Check if the HAM bit has changed
    if (ham(oldValue) ^ ham(newValue)) {
        pixelEngine.colChanges.insert(pixel, RegChange { BPLCON0, newValue } );
    }
    
    // Update value
    bplcon0 = newValue;
    
    // Update border color index, because the ECSENA bit might have changed
    updateBorderColor();
    
    // Check if the BPU bits have changed
    u16 oldBpuBits = (oldValue >> 12) & 0b111;
    u16 newBpuBits = (newValue >> 12) & 0b111;

    if (oldBpuBits != newBpuBits) {
        // trace("Changing BPU bits from %d to %d\n", oldBpuBits, newBpuBits);
    }
    
    // Report a suspicious BPU value
    trace(XFILES && newBpuBits > (hires(bplcon0) ? 4 : 6),
          "XFILES (BPLCON0): BPU = %d\n", newBpuBits);
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
Denise::setBPLCON2(u16 newValue)
{
    trace(BPLREG_DEBUG, "setBPLCON2(%X)\n", newValue);

    bplcon2 = newValue;
    
    debug(XFILES && PF1Px() > 4, "XFILES (BPLCON2): PF1P = %d\n", PF1Px());
    debug(XFILES && PF2Px() > 4, "XFILES (BPLCON2): PF2P = %d\n", PF2Px());
    
    // Record the register change
    i64 pixel = 4 * agnus.pos.h + 4;
    conChanges.insert(pixel, RegChange { SET_BPLCON2, newValue });    
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

template <isize x, Accessor s> void
Denise::pokeBPLxDAT(u16 value)
{
    assert(x < 6);
    trace(BPLREG_DEBUG, "pokeBPL%zdDAT(%X)\n", x + 1, value);

    if (s == ACCESSOR_AGNUS) {
        /*
        debug("BPL%dDAT written by Agnus (%x)\n", x, value);
        */
    }
    
    setBPLxDAT<x>(value);
}

template <isize x> void
Denise::setBPLxDAT(u16 value)
{
    assert(x < 6);
    trace(BPLDAT_DEBUG, "setBPL%zdDAT(%X)\n", x + 1, value);
        
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

template <isize x> void
Denise::pokeSPRxPOS(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%zdPOS(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0  (Ex = VSTART)
    // E7 E6 E5 E4 E3 E2 E1 E0 H8 H7 H6 H5 H4 H3 H2 H1  (Hx = HSTART)

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0POS + x, value } );
}

template <isize x> void
Denise::pokeSPRxCTL(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%zdCTL(%X)\n", x, value);

    // 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
    // L7 L6 L5 L4 L3 L2 L1 L0 AT  -  -  -  - E8 L8 H0  (Lx = VSTOP)
    
    // Disarm the sprite
    // CLR_BIT(armed, x);

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0CTL + x, value } );
}

template <isize x> void
Denise::pokeSPRxDATA(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%zdDATA(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;
    
    // Remember that the sprite was armed at least once in this rasterline
    SET_BIT(wasArmed, x);

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0DATA + x, value } );
}

template <isize x> void
Denise::pokeSPRxDATB(u16 value)
{
    assert(x < 8);
    trace(SPRREG_DEBUG, "pokeSPR%zdDATB(%X)\n", x, value);
    
    // If requested, let this sprite disappear by making it transparent
    if (GET_BIT(config.hiddenSprites, x)) value = 0;

    // Record the register change
    i64 pos = 4 * (agnus.pos.h + 1);
    sprChanges[x/2].insert(pos, RegChange { SET_SPR0DATB + x, value });
}

template <Accessor s, isize xx> void
Denise::pokeCOLORxx(u16 value)
{
    trace(COLREG_DEBUG, "pokeCOLOR%02zd(%X)\n", xx, value);

    u32 reg = 0x180 + 2*xx;
    i16 pos = agnus.pos.h;

    // If the CPU modifies color, the change takes effect one DMA cycle earlier
    if (s != ACCESSOR_AGNUS && agnus.pos.h != 0) pos--;
    
    // Record the color change
    pixelEngine.colChanges.insert(4 * pos, RegChange { reg, value } );
}

template void Denise::pokeBPLxDAT<0,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<0,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<1,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<1,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<2,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<2,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<3,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<3,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<4,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<4,ACCESSOR_AGNUS>(u16 value);
template void Denise::pokeBPLxDAT<5,ACCESSOR_CPU>(u16 value);
template void Denise::pokeBPLxDAT<5,ACCESSOR_AGNUS>(u16 value);

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

template void Denise::pokeCOLORxx<ACCESSOR_CPU, 0>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 0>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 1>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 1>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 2>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 2>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 3>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 3>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 4>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 4>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 5>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 5>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 6>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 6>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 7>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 7>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 8>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 8>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 9>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 9>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 10>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 10>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 11>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 11>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 12>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 12>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 13>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 13>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 14>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 14>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 15>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 15>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 16>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 16>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 17>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 17>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 18>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 18>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 19>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 19>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 20>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 20>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 21>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 21>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 22>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 22>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 23>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 23>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 24>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 24>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 25>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 25>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 26>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 26>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 27>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 27>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 28>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 28>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 29>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 29>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 30>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 30>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_CPU, 31>(u16 value);
template void Denise::pokeCOLORxx<ACCESSOR_AGNUS, 31>(u16 value);
