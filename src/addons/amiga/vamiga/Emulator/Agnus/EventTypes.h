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

//
// Enumerations
//

enum_long(EventID)
{
    EVENT_NONE = 0,
    
    //
    // Events in the primary event table
    //

    // REG slot
    REG_CHANGE = 1,
    // REG_HSYNC,
    REG_EVENT_COUNT,

    // CIA slots
    CIA_EXECUTE = 1,
    CIA_WAKEUP,
    CIA_EVENT_COUNT,
    
    // BPL slot
    BPL_L1  = 0x04,
    BPL_L2  = 0x08,
    BPL_L3  = 0x0C,
    BPL_L4  = 0x10,
    BPL_L5  = 0x14,
    BPL_L6  = 0x18,
    BPL_H1  = 0x1C,
    BPL_H2  = 0x20,
    BPL_H3  = 0x24,
    BPL_H4  = 0x28,
    BPL_SR  = 0x2C,
    BPL_EOL = 0x30,
    BPL_EVENT_COUNT = 0x34,

    // DAS slot
    DAS_REFRESH = 1,
    DAS_D0,
    DAS_D1,
    DAS_D2,
    DAS_A0,
    DAS_A1,
    DAS_A2,
    DAS_A3,
    DAS_S0_1,
    DAS_S0_2,
    DAS_S1_1,
    DAS_S1_2,
    DAS_S2_1,
    DAS_S2_2,
    DAS_S3_1,
    DAS_S3_2,
    DAS_S4_1,
    DAS_S4_2,
    DAS_S5_1,
    DAS_S5_2,
    DAS_S6_1,
    DAS_S6_2,
    DAS_S7_1,
    DAS_S7_2,
    DAS_SDMA,
    DAS_TICK,
    DAS_TICK2,
    DAS_EVENT_COUNT,

    // Copper slot
    COP_REQ_DMA = 1,
    COP_WAKEUP,
    COP_WAKEUP_BLIT,
    COP_FETCH,
    COP_MOVE,
    COP_WAIT_OR_SKIP,
    COP_WAIT1,
    COP_WAIT2,
    COP_WAIT_BLIT,
    COP_SKIP1,
    COP_SKIP2,
    COP_JMP1,
    COP_JMP2,
    COP_VBLANK,
    COP_EVENT_COUNT,
    
    // Blitter slot
    BLT_STRT1 = 1,
    BLT_STRT2,
    BLT_COPY_SLOW,
    BLT_COPY_FAKE,
    BLT_LINE_FAKE,
    BLT_EVENT_COUNT,
        
    // SEC slot
    SEC_TRIGGER = 1,
    SEC_EVENT_COUNT,
    
    //
    // Events in secondary event table
    //

    // Audio channels
    CHX_PERFIN = 1,
    CHX_EVENT_COUNT,

    // Disk controller slot
    DSK_ROTATE = 1,
    DSK_EVENT_COUNT,

    // Disk change slot
    DCH_INSERT = 1,
    DCH_EJECT,
    DCH_EVENT_COUNT,

    // Strobe slot
    VBL_STROBE0 = 1,
    VBL_STROBE1,
    VBL_STROBE2,
    VBL_EVENT_COUNT,
    
    // IRQ slot
    IRQ_CHECK = 1,
    IRQ_EVENT_COUNT,

    // IPL slot
    IPL_CHANGE = 1,
    IPL_EVENT_COUNT,

    // Keyboard
    KBD_TIMEOUT = 1,
    KBD_DAT,
    KBD_CLK0,
    KBD_CLK1,
    KBD_SYNC_DAT0,
    KBD_SYNC_CLK0,
    KBD_SYNC_DAT1,
    KBD_SYNC_CLK1,
    KBD_EVENT_COUNT,

    // Serial data out (UART)
    TXD_BIT = 1,
    TXD_EVENT_COUNT,

    // Serial data out (UART)
    RXD_BIT = 1,
    RXD_EVENT_COUT,

    // Potentiometer
    POT_DISCHARGE = 1,
    POT_CHARGE,
    POT_EVENT_COUNT,
    
    // Screenshots
    SCR_TAKE = 1,
    SCR_EVENT_COUNT,
    
    // Inspector slot
    INS_NONE = 1,
    INS_AMIGA,
    INS_CPU,
    INS_MEM,
    INS_CIA,
    INS_AGNUS,
    INS_PAULA,
    INS_DENISE,
    INS_PORTS,
    INS_EVENTS,
    INS_EVENT_COUNT,

    // Rasterline slot
    RAS_HSYNC = 1,
    RAS_EVENT_COUNT

};

static inline bool isRegEvent(EventID id) { return id < REG_EVENT_COUNT; }
static inline bool isCiaEvent(EventID id) { return id < CIA_EVENT_COUNT; }
static inline bool isBplEvent(EventID id) { return id < BPL_EVENT_COUNT; }
static inline bool isDasEvent(EventID id) { return id < DAS_EVENT_COUNT; }
static inline bool isCopEvent(EventID id) { return id < COP_EVENT_COUNT; }
static inline bool isBltEvent(EventID id) { return id < BLT_EVENT_COUNT; }

static inline bool isBplxEvent(EventID id, int x)
{
    switch(id & ~0b11) {

        case BPL_L1: case BPL_H1: return x == 1;
        case BPL_L2: case BPL_H2: return x == 2;
        case BPL_L3: case BPL_H3: return x == 3;
        case BPL_L4: case BPL_H4: return x == 4;
        case BPL_L5:              return x == 5;
        case BPL_L6:              return x == 6;

        default:
            return false;
    }
}
