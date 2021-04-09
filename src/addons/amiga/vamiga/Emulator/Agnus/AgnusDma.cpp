// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Agnus.h"
#include "Denise.h"

/* A central element in the emulation of an Amiga is the accurate modeling of
* the DMA timeslot allocation table (Fig. 6-9 im the HRM, 3rd revision). All
* bitplane related events are managed in the BPL_SLOT. All disk, audio, and
* sprite related events are managed in the DAS_SLOT.
*
* vAmiga utilizes two event tables to schedule events in the DAS_SLOT and
* BPL_SLOT. Assuming that sprite DMA is enabled and Denise draws 6 bitplanes
* in lores mode starting at 0x28, the tables would look like this:
*
*     bplEvent[0x00] = EVENT_NONE   dasEvent[0x00] = EVENT_NONE
*     bplEvent[0x01] = EVENT_NONE   dasEvent[0x01] = BUS_REFRESH
*         ...                           ...
*     bplEvent[0x28] = EVENT_NONE   dasEvent[0x28] = EVENT_NONE
*     bplEvent[0x29] = BPL_L4       dasEvent[0x29] = DAS_S5_1
*     bplEvent[0x2A] = BPL_L6       dasEvent[0x2A] = EVENT_NONE
*     bplEvent[0x2B] = BPL_L2       dasEvent[0x2B] = DAS_S5_2
*     bplEvent[0x2C] = EVENT_NONE   dasEvent[0x2C] = EVENT_NONE
*     bplEvent[0x2D] = BPL_L3       dasEvent[0x2D] = DAS_S6_1
*     bplEvent[0x2E] = BPL_L5       dasEvent[0x2E] = EVENT_NONE
*     bplEvent[0x2F] = BPL_L1       dasEvent[0x2F] = DAS_S6_2
*         ...                           ...
*     bplEvent[0xE2] = BPL_EOL      dasEvent[0xE2] = BUS_REFRESH
*
* The BPL_EOL event doesn't perform DMA. It concludes the current line.
*
* All events in the BPL_SLOT can be superimposed by two drawing flags (bit 0
* and bit 1) that trigger the transfer of the data registers into the shift
* registers at the correct DMA cycle. Bit 0 controls the odd bitplanes and
* bit 1 controls the even bitplanes. Settings these flags changes the
* scheduled event, e.g.:
*
*     BPL_L4  becomes  BPL_L4_ODD   if bit 0 is set
*     BPL_L4  becomes  BPL_L4_EVEN  if bit 1 is set
*     BPL_L4  becomes  BPL_L4_ODD_EVEN  if both bits are set
*
* Each event table is accompanied by a jump table that points to the next
* event. Given the example tables above, the jump tables would look like this:
*
*     nextBplEvent[0x00] = 0x29     nextDasEvent[0x00] = 0x01
*     nextBplEvent[0x01] = 0x29     nextDasEvent[0x01] = 0x03
*           ...                           ...
*     nextBplEvent[0x28] = 0x29     nextDasEvent[0x28] = 0x29
*     nextBplEvent[0x29] = 0x2A     nextDasEvent[0x29] = 0x2B
*     nextBplEvent[0x2A] = 0x2B     nextDasEvent[0x2A] = 0x2B
*     nextBplEvent[0x2B] = 0x2D     nextDasEvent[0x2B] = 0x2D
*     nextBplEvent[0x2C] = 0x2D     nextDasEvent[0x2C] = 0x2D
*     nextBplEvent[0x2D] = 0x2E     nextDasEvent[0x2D] = 0x2F
*     nextBplEvent[0x2E] = 0x2F     nextDasEvent[0x2E] = 0x2F
*     nextBplEvent[0x2F] = 0x31     nextDasEvent[0x2F] = 0x31
*           ...                           ...
*     nextBplEvent[0xE2] = 0x00     nextDasEvent[0xE2] = 0x00
*
* Whenever one the DMA tables is modified, the corresponding jump table
* has to be updated, too.
*
* To quickly setup the event tables, vAmiga utilizes two static lookup
* tables. Depending on the current resoution, BPU value, or DMA status,
* segments of these lookup tables are copied to the event tables.
*
*      Table: bitplaneDMA[Resolution][Bitplanes][Cycle]
*
*             (Bitplane DMA events in a single rasterline)
*
*             Resolution : 0 or 1        (0 = LORES / 1 = HIRES)
*              Bitplanes : 0 .. 6        (Bitplanes in use, BPU)
*                  Cycle : 0 .. HPOS_MAX (DMA cycle)
*
*      Table: dasDMA[dmacon]
*
*             (Disk, Audio, and Sprite DMA events in a single rasterline)
*
*                 dmacon : Bits 0 .. 5 of register DMACON
*/

void
Agnus::initLookupTables()
{
    initBplEventTableLores();
    initBplEventTableHires();
    initDasEventTable();
}

void
Agnus::initBplEventTableLores()
{
    memset(bplDMA[0], 0, sizeof(bplDMA[0]));

    for (isize bpu = 0; bpu < 7; bpu++) {

        EventID *p = &bplDMA[0][bpu][0];

        // Iterate through all 22 fetch units
        for (isize i = 0; i <= 0xD8; i += 8, p += 8) {

            switch(bpu) {
                case 6: p[2] = BPL_L6;
                case 5: p[6] = BPL_L5;
                case 4: p[1] = BPL_L4;
                case 3: p[5] = BPL_L3;
                case 2: p[3] = BPL_L2;
                case 1: p[7] = BPL_L1;
            }
        }

        assert(bplDMA[0][bpu][HPOS_MAX] == EVENT_NONE);
        bplDMA[0][bpu][HPOS_MAX] = BPL_EOL;
    }
}

void
Agnus::initBplEventTableHires()
{
    memset(bplDMA[1], 0, sizeof(bplDMA[1]));

    for (isize bpu = 0; bpu < 7; bpu++) {

        EventID *p = &bplDMA[1][bpu][0];

        for (isize i = 0; i <= 0xD8; i += 8, p += 8) {

            switch(bpu) {
                case 6:
                case 5:
                case 4: p[0] = p[4] = BPL_H4;
                case 3: p[2] = p[6] = BPL_H3;
                case 2: p[1] = p[5] = BPL_H2;
                case 1: p[3] = p[7] = BPL_H1;
            }
        }

        assert(bplDMA[1][bpu][HPOS_MAX] == EVENT_NONE);
        bplDMA[1][bpu][HPOS_MAX] = BPL_EOL;
    }
}

void
Agnus::initDasEventTable()
{
    memset(dasDMA, 0, sizeof(dasDMA));

    for (isize dmacon = 0; dmacon < 64; dmacon++) {

        EventID *p = dasDMA[dmacon];

        p[0x01] = DAS_REFRESH;

        if (dmacon & DSKEN) {
            p[0x07] = DAS_D0;
            p[0x09] = DAS_D1;
            p[0x0B] = DAS_D2;
        }
        
        // Audio DMA is possible even in lines where the DMACON bits are false
        p[0x0D] = DAS_A0;
        p[0x0F] = DAS_A1;
        p[0x11] = DAS_A2;
        p[0x13] = DAS_A3;
        
        if (dmacon & SPREN) {
            p[0x15] = DAS_S0_1;
            p[0x17] = DAS_S0_2;
            p[0x19] = DAS_S1_1;
            p[0x1B] = DAS_S1_2;
            p[0x1D] = DAS_S2_1;
            p[0x1F] = DAS_S2_2;
            p[0x21] = DAS_S3_1;
            p[0x23] = DAS_S3_2;
            p[0x25] = DAS_S4_1;
            p[0x27] = DAS_S4_2;
            p[0x29] = DAS_S5_1;
            p[0x2B] = DAS_S5_2;
            p[0x2D] = DAS_S6_1;
            p[0x2F] = DAS_S6_2;
            p[0x31] = DAS_S7_1;
            p[0x33] = DAS_S7_2;
        }

        p[0xDF] = DAS_SDMA;

        p[0x52] = DAS_TICK2;
        p[0x66] = DAS_TICK;
    }
}

template <> bool Agnus::auddma<0>(u16 v) { return (v & DMAEN) && (v & AUD0EN); }
template <> bool Agnus::auddma<1>(u16 v) { return (v & DMAEN) && (v & AUD1EN); }
template <> bool Agnus::auddma<2>(u16 v) { return (v & DMAEN) && (v & AUD2EN); }
template <> bool Agnus::auddma<3>(u16 v) { return (v & DMAEN) && (v & AUD3EN); }

void
Agnus::enableBplDmaOCS()
{
    if (pos.h + 2 < ddfstrtReached || bpldma(dmaconAtDDFStrt)) {
        
        updateBplEvents(dmacon, bplcon0, pos.h + 2);
        updateBplEvent();
    }
}

void
Agnus::disableBplDmaOCS()
{
    updateBplEvents(dmacon, bplcon0, pos.h + 2);
    updateBplEvent();
}

void
Agnus::enableBplDmaECS()
{
    // if (pos.h + 2 < ddfstrtReached || bpldma(dmaconAtDDFStrt)) {
    if (pos.h + 2 < ddfstrtReached) {

        updateBplEvents(dmacon, bplcon0, pos.h + 2);
        updateBplEvent();
        return;
    }
    
    if (pos.h + 2 >= ddfstopReached) return;
    
    // debug("Enable DMA ECS: %d %d %d %d (%x)\n", ddfstrt, ddfstrtReached, ddfstop, ddfstopReached, bplcon1);
    
    i16 posh = pos.h + 4;
    // debug("posh = %d MAX = %d\n", posh, MAX(posh, ddfstrtReached));
    ddfLores.compute(std::max(posh, ddfstrtReached), ddfstopReached, bplcon1);
    ddfHires.compute(std::max(posh, ddfstrtReached), ddfstopReached, bplcon1);
    hsyncActions |= HSYNC_PREDICT_DDF;
    
    updateBplEvents();
    updateBplEvent();
    updateDrawingFlags(false);
}

void
Agnus::disableBplDmaECS()
{
    updateBplEvents(dmacon, bplcon0, pos.h + 2);
    updateBplEvent();
}

template <BusOwner owner> bool
Agnus::busIsFree() const
{
    // Deny if the bus is already in use
    if (busOwner[pos.h] != BUS_NONE) return false;

    switch (owner) {

        case BUS_COPPER:
        {
            // Deny if Copper DMA is disabled
            if (!copdma()) return false;

            // Deny in cycle E0
            if (unlikely(pos.h == 0xE0)) return false;
            return true;
        }
        case BUS_BLITTER:
        {
            // Deny if Blitter DMA is disabled
            if (!bltdma()) return false;
            
            // Deny if the CPU has precedence
            if (bls && !bltpri()) return false;

            return true;
        }
    }

    assert(false);
    return false;
}

template <BusOwner owner> bool
Agnus::allocateBus()
{
    // Deny if the bus has been allocated already
    if (busOwner[pos.h] != BUS_NONE) return false;

    switch (owner) {

        case BUS_COPPER:
        {
            // Assign bus to the Copper
            busOwner[pos.h] = BUS_COPPER;
            return true;
        }
        case BUS_BLITTER:
        {
            // Deny if Blitter DMA is off
            if (!bltdma()) return false;

            // Deny if the CPU has precedence
            if (bls && !bltpri()) return false;

            // Assign the bus to the Blitter
            busOwner[pos.h] = BUS_BLITTER;
            return true;
        }
    }

    assert(false);
    return false;
}

u16
Agnus::doDiskDMA()
{
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (dskpt);
    dskpt += 2;

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = result;
    stats.usage[BUS_DISK]++;

    return result;
}

template <int channel> u16
Agnus::doAudioDMA()
{
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (audpt[channel]);
    audpt[channel] += 2;

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_AUDIO;
    busValue[pos.h] = result;
    stats.usage[BUS_AUDIO]++;

    return result;
}

template <int bitplane> u16
Agnus::doBitplaneDMA()
{
    assert(bitplane >= 0 && bitplane <= 5);
    const BusOwner owner = BusOwner(BUS_BPL1 + bitplane);
    
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (bplpt[bitplane]);
    bplpt[bitplane] += 2;

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = owner;
    busValue[pos.h] = result;
    stats.usage[owner]++;

    return result;
}

template <int channel> u16
Agnus::doSpriteDMA()
{
    assert(channel >= 0 && channel <= 7);
    const BusOwner owner = BusOwner(BUS_SPRITE0 + channel);

    u16 result = mem.peek16 <ACCESSOR_AGNUS> (sprpt[channel]);
    sprpt[channel] += 2;

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = owner;
    busValue[pos.h] = result;
    stats.usage[owner]++;

    return result;
}

u16
Agnus::doCopperDMA(u32 addr)
{
    u16 result = mem.peek16 <ACCESSOR_AGNUS> (addr);

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = result;
    stats.usage[BUS_COPPER]++;

    return result;
}

u16
Agnus::doBlitterDMA(u32 addr)
{
    // Assure that the Blitter owns the bus when this function is called
    assert(busOwner[pos.h] == BUS_BLITTER);

    u16 result = mem.peek16 <ACCESSOR_AGNUS> (addr);

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_BLITTER;
    busValue[pos.h] = result;
    stats.usage[BUS_BLITTER]++;

    return result;
}

void
Agnus::doDiskDMA(u16 value)
{
    mem.poke16 <ACCESSOR_AGNUS> (dskpt, value);
    dskpt += 2;

    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_DISK;
    busValue[pos.h] = value;
    stats.usage[BUS_DISK]++;
}

void
Agnus::doCopperDMA(u32 addr, u16 value)
{
    mem.pokeCustom16<ACCESSOR_AGNUS>(addr, value);
    
    assert(pos.h < HPOS_CNT);
    busOwner[pos.h] = BUS_COPPER;
    busValue[pos.h] = value;
    stats.usage[BUS_COPPER]++;
}

void
Agnus::doBlitterDMA(u32 addr, u16 value)
{
    mem.poke16 <ACCESSOR_AGNUS> (addr, value);
    
    assert(pos.h < HPOS_CNT);
    assert(busOwner[pos.h] == BUS_BLITTER); // Bus is already allocated
    busValue[pos.h] = value;
    stats.usage[BUS_BLITTER]++;
}

void
Agnus::clearBplEvents()
{
    for (isize i = 0; i < HPOS_MAX; i++) bplEvent[i] = EVENT_NONE;
    for (isize i = 0; i < HPOS_MAX; i++) nextBplEvent[i] = HPOS_MAX;
}

void
Agnus::updateBplEvents(u16 dmacon, u16 bplcon0, int first, int last)
{
    assert(first >= 0 && last < HPOS_CNT);

    bool hires = Denise::hires(bplcon0);
    int channels = bpu(bplcon0);
    assert(channels <= 6);

    // Set number of bitplanes to 0 if we are not in a bitplane DMA line
    if (!inBplDmaLine(dmacon, bplcon0)) channels = 0;

    // Do the same if DDFSTRT is never reached in this line
    if (ddfstrtReached == -1) channels = 0;
    
    // Allocate slots
    if (hires) {
        
        for (isize i = first; i <= last; i++)
            bplEvent[i] =
            inHiresDmaAreaOdd(i) ? bplDMA[1][channels][i] :
            inHiresDmaAreaEven(i) ? bplDMA[1][channels][i] : EVENT_NONE;
        
        // Add extra shift register events if the even/odd DDF windows differ
        // These events are like BPL_H0 events without performing DMA.
        for (isize i = ddfHires.strtEven; i < ddfHires.strtOdd; i++)
            if ((i & 3) == 3 && bplEvent[i] == EVENT_NONE) bplEvent[i] = BPL_SR;
        for (isize i = ddfHires.stopOdd; i < ddfHires.stopEven; i++)
            if ((i & 3) == 3 && bplEvent[i] == EVENT_NONE) bplEvent[i] = BPL_SR;

    } else {
        
        for (isize i = first; i <= last; i++)
            bplEvent[i] =
            inLoresDmaAreaOdd(i) ? bplDMA[0][channels][i] :
            inLoresDmaAreaEven(i) ? bplDMA[0][channels][i] : EVENT_NONE;
    
        // Add extra shift register events if the even/odd DDF windows differ
        // These events are like BPL_L0 events without performing DMA.
        for (isize i = ddfLores.strtEven; i < ddfLores.strtOdd; i++)
             if ((i & 7) == 7 && bplEvent[i] == EVENT_NONE) bplEvent[i] = BPL_SR;
        for (isize i = ddfLores.stopOdd; i < ddfLores.stopEven; i++)
             if ((i & 7) == 7 && bplEvent[i] == EVENT_NONE) bplEvent[i] = BPL_SR;
    }
        
    // Make sure the table ends with a BPL_EOL event
    bplEvent[HPOS_MAX] = BPL_EOL;

    // Update the drawing flags and update the jump table
    updateDrawingFlags(hires);
}

void
Agnus::updateDrawingFlags(bool hires)
{
    assert(scrollHiresEven < 8);
    assert(scrollHiresOdd  < 8);
    assert(scrollLoresEven < 8);
    assert(scrollHiresOdd  < 8);
    
    // Superimpose the drawing flags (bits 0 and 1)
    // Bit 0 is used to for odd bitplanes and bit 1 for even bitplanes
    if (hires) {
        for (isize i = scrollHiresOdd; i < HPOS_CNT; i += 4)
            bplEvent[i] = (EventID)(bplEvent[i] | 1);
        for (isize i = scrollHiresEven; i < HPOS_CNT; i += 4)
            bplEvent[i] = (EventID)(bplEvent[i] | 2);
    } else {
        for (isize i = scrollLoresOdd; i < HPOS_CNT; i += 8)
            bplEvent[i] = (EventID)(bplEvent[i] | 1);
        for (isize i = scrollLoresEven; i < HPOS_CNT; i += 8)
            bplEvent[i] = (EventID)(bplEvent[i] | 2);
    }
    updateBplJumpTable();
}

void
Agnus::clearDasEvents()
{
    updateDasEvents(0);
}

void
Agnus::updateDasEvents(u16 dmacon)
{
    assert(dmacon < 64);

    // Allocate slots and renew the jump table
    for (isize i = 0; i < 0x38; i++) dasEvent[i] = dasDMA[dmacon][i];
    updateDasJumpTable(0x38);
}

void
Agnus::updateBplJumpTable(i16 end)
{
    assert(end <= HPOS_MAX);

    u8 next = nextBplEvent[end];
    for (isize i = end; i >= 0; i--) {
        nextBplEvent[i] = next;
        if (bplEvent[i]) next = i;
    }
}

void
Agnus::updateDasJumpTable(i16 end)
{
    assert(end <= HPOS_MAX);

    u8 next = nextDasEvent[end];
    for (isize i = end; i >= 0; i--) {
        nextDasEvent[i] = next;
        if (dasEvent[i]) next = i;
    }
}

void
Agnus::dumpEventTable(const EventID *table, char str[256][3], isize from, isize to) const
{
    char r1[256], r2[256], r3[256], r4[256], r5[256];
    isize i;

    for (i = 0; i <= to - from; i++) {

        isize digit1 = (from + i) / 16;
        isize digit2 = (from + i) % 16;

        r1[i] = (digit1 < 10) ? digit1 + '0' : (digit1 - 10) + 'A';
        r2[i] = (digit2 < 10) ? digit2 + '0' : (digit2 - 10) + 'A';
        r3[i] = str[table[from + i]][0];
        r4[i] = str[table[from + i]][1];
        r5[i] = str[table[from + i]][2];
    }
    r1[i] = r2[i] = r3[i] = r4[i] = r5[i] = 0;

    msg("%s\n", r1);
    msg("%s\n", r2);
    msg("%s\n", r3);
    msg("%s\n", r4);
    msg("%s\n", r5);
}

void
Agnus::dumpBplEventTable(int from, int to) const
{
    char str[256][3];

    memset(str, '?', sizeof(str));
    
    // Events
    for (isize i = 0; i < 4; i++) {
        str[i][0] = '.';                str[i][1] = '.';
        str[(int)BPL_L1 + i][0]  = 'L'; str[(int)BPL_L1 + i][1]  = '1';
        str[(int)BPL_L2 + i][0]  = 'L'; str[(int)BPL_L2 + i][1]  = '2';
        str[(int)BPL_L3 + i][0]  = 'L'; str[(int)BPL_L3 + i][1]  = '3';
        str[(int)BPL_L4 + i][0]  = 'L'; str[(int)BPL_L4 + i][1]  = '4';
        str[(int)BPL_L5 + i][0]  = 'L'; str[(int)BPL_L5 + i][1]  = '5';
        str[(int)BPL_L6 + i][0]  = 'L'; str[(int)BPL_L6 + i][1]  = '6';
        str[(int)BPL_H1 + i][0]  = 'H'; str[(int)BPL_H1 + i][1]  = '1';
        str[(int)BPL_H2 + i][0]  = 'H'; str[(int)BPL_H2 + i][1]  = '2';
        str[(int)BPL_H3 + i][0]  = 'H'; str[(int)BPL_H3 + i][1]  = '3';
        str[(int)BPL_H4 + i][0]  = 'H'; str[(int)BPL_H4 + i][1]  = '4';
        str[(int)BPL_EOL + i][0] = 'E'; str[(int)BPL_EOL + i][1] = 'O';
    }

    // Drawing flags
    for (isize i = 0; i < 256; i += 4) str[i][2] = '.';
    for (isize i = 1; i < 256; i += 4) str[i][2] = 'o';
    for (isize i = 2; i < 256; i += 4) str[i][2] = 'e';
    for (isize i = 3; i < 256; i += 4) str[i][2] = 'b';

    dumpEventTable(bplEvent, str, from, to);
}

void
Agnus::dumpBplEventTable() const
{
    // Dump the event table
    msg("Event table:\n\n");
    msg("ddfstrt = %X dffstop = %X\n", ddfstrt, ddfstop);
    msg("ddfLoresOdd:  (%X - %X)\n", ddfLores.strtOdd, ddfLores.stopOdd);
    msg("ddfLoresEven: (%X - %X)\n", ddfLores.strtEven, ddfLores.stopEven);
    msg("ddfHiresOdd:  (%X - %X)\n", ddfHires.strtOdd, ddfHires.stopOdd);
    msg("ddfHiresEven: (%X - %X)\n", ddfHires.strtEven, ddfHires.stopEven);

    dumpBplEventTable(0x00, 0x4F);
    dumpBplEventTable(0x50, 0x9F);
    dumpBplEventTable(0xA0, 0xE2);

    // Dump the jump table
    msg("\nJump table:\n\n");
    isize i = nextBplEvent[0];
    msg("0 -> %zx", i);
    while (i) {
        assert(i < HPOS_CNT);
        assert(nextBplEvent[i] == 0 || nextBplEvent[i] > i);
        i = nextBplEvent[i];
        msg(" -> %zx", i);
    }
    msg("\n");
}

void
Agnus::dumpDasEventTable(int from, int to) const
{
    char str[256][3];

    memset(str, '?', sizeof(str));
    str[(int)EVENT_NONE][0]  = '.'; str[(int)EVENT_NONE][1]  = '.';
    str[(int)DAS_REFRESH][0] = 'R'; str[(int)DAS_REFRESH][1] = 'E';
    str[(int)DAS_D0][0]      = 'D'; str[(int)DAS_D0][1]      = '0';
    str[(int)DAS_D1][0]      = 'D'; str[(int)DAS_D1][1]      = '1';
    str[(int)DAS_D2][0]      = 'D'; str[(int)DAS_D2][1]      = '2';
    str[(int)DAS_A0][0]      = 'A'; str[(int)DAS_A0][1]      = '0';
    str[(int)DAS_A1][0]      = 'A'; str[(int)DAS_A1][1]      = '1';
    str[(int)DAS_A2][0]      = 'A'; str[(int)DAS_A2][1]      = '2';
    str[(int)DAS_A3][0]      = 'A'; str[(int)DAS_A3][1]      = '3';
    str[(int)DAS_S0_1][0]    = '0'; str[(int)DAS_S0_1][1]    = '1';
    str[(int)DAS_S0_2][0]    = '0'; str[(int)DAS_S0_2][1]    = '2';
    str[(int)DAS_S1_1][0]    = '1'; str[(int)DAS_S1_1][1]    = '1';
    str[(int)DAS_S1_2][0]    = '1'; str[(int)DAS_S1_2][1]    = '2';
    str[(int)DAS_S2_1][0]    = '2'; str[(int)DAS_S2_1][1]    = '1';
    str[(int)DAS_S2_2][0]    = '2'; str[(int)DAS_S2_2][1]    = '2';
    str[(int)DAS_S3_1][0]    = '3'; str[(int)DAS_S3_1][1]    = '1';
    str[(int)DAS_S3_2][0]    = '3'; str[(int)DAS_S3_2][1]    = '2';
    str[(int)DAS_S4_1][0]    = '4'; str[(int)DAS_S4_1][1]    = '1';
    str[(int)DAS_S4_2][0]    = '4'; str[(int)DAS_S4_2][1]    = '2';
    str[(int)DAS_S5_1][0]    = '5'; str[(int)DAS_S5_1][1]    = '1';
    str[(int)DAS_S5_2][0]    = '5'; str[(int)DAS_S5_2][1]    = '2';
    str[(int)DAS_S6_1][0]    = '6'; str[(int)DAS_S6_1][1]    = '1';
    str[(int)DAS_S6_2][0]    = '6'; str[(int)DAS_S6_2][1]    = '2';
    str[(int)DAS_S7_1][0]    = '7'; str[(int)DAS_S7_1][1]    = '1';
    str[(int)DAS_S7_2][0]    = '7'; str[(int)DAS_S7_2][1]    = '2';
    str[(int)DAS_SDMA][0]    = 'S'; str[(int)DAS_SDMA][1]    = 'D';
    str[(int)DAS_TICK][0]    = 'T'; str[(int)DAS_TICK][1]    = 'K';
    str[(int)DAS_TICK2][0]   = 'T'; str[(int)DAS_TICK2][1]   = '2';

    for (isize i = 1; i < 256; i++) str[i][2] = ' ';
    
    dumpEventTable(dasEvent, str, from, to);
}

void
Agnus::dumpDasEventTable() const
{
    // Dump the event table
    dumpDasEventTable(0x00, 0x4F);
    dumpDasEventTable(0x50, 0x9F);
    dumpDasEventTable(0xA0, 0xE2);
}

template u16 Agnus::doAudioDMA<0>();
template u16 Agnus::doAudioDMA<1>();
template u16 Agnus::doAudioDMA<2>();
template u16 Agnus::doAudioDMA<3>();

template u16 Agnus::doBitplaneDMA<0>();
template u16 Agnus::doBitplaneDMA<1>();
template u16 Agnus::doBitplaneDMA<2>();
template u16 Agnus::doBitplaneDMA<3>();
template u16 Agnus::doBitplaneDMA<4>();
template u16 Agnus::doBitplaneDMA<5>();

template u16 Agnus::doSpriteDMA<0>();
template u16 Agnus::doSpriteDMA<1>();
template u16 Agnus::doSpriteDMA<2>();
template u16 Agnus::doSpriteDMA<3>();
template u16 Agnus::doSpriteDMA<4>();
template u16 Agnus::doSpriteDMA<5>();
template u16 Agnus::doSpriteDMA<6>();
template u16 Agnus::doSpriteDMA<7>();

template bool Agnus::allocateBus<BUS_COPPER>();
template bool Agnus::allocateBus<BUS_BLITTER>();

template bool Agnus::busIsFree<BUS_COPPER>() const;
template bool Agnus::busIsFree<BUS_BLITTER>() const;
