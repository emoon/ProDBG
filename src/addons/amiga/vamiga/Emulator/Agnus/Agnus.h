// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AgnusTypes.h"
#include "AmigaComponent.h"
#include "Beam.h"
#include "Blitter.h"
#include "ChangeRecorder.h"
#include "Copper.h"
#include "DDF.h"
#include "DmaDebugger.h"
#include "Event.h"
#include "Frame.h"
#include "Memory.h"

#define isPrimarySlot(s) ((s) <= SLOT_SEC)
#define isSecondarySlot(s) ((s) > SLOT_SEC && (s) < SLOT_COUNT)

/* Hsync handler action flags
 *
 *       HSYNC_PREDICT_DDF : Forces the hsync handler to recompute the
 *                           display data fetch window.
 *  HSYNC_UPDATE_BPL_TABLE : Forces the hsync handler to update the bitplane
 *                           DMA event table.
 *  HSYNC_UPDATE_DAS_TABLE : Forces the hsync handler to update the disk,
 *                          audio, sprite DMA event table.
 */
#define HSYNC_PREDICT_DDF       0b001
#define HSYNC_UPDATE_BPL_TABLE  0b010
#define HSYNC_UPDATE_DAS_TABLE  0b100

/* Bitplane event modifiers
 *
 *                DRAW_ODD : Starts the shift registers of the odd bitplanes
 *                           to generate pixels.
 *               DRAW_EVEN : Starts the shift registers of the even bitplanes
 *                           to generate pixels.
 */
#define DRAW_ODD                0b001
#define DRAW_EVEN               0b010

class Agnus : public AmigaComponent {
    
    // Current configuration
    AgnusConfig config;

    // Result of the latest inspection
    AgnusInfo info;
    EventInfo eventInfo;

    // Current workload
    AgnusStats stats;


    //
    // Sub components
    //
    
public:

    Copper copper = Copper(amiga);
    Blitter blitter = Blitter(amiga);
    DmaDebugger dmaDebugger = DmaDebugger(amiga);

    
    //
    // Events
    //
    
public:
    
    // Event slots
    Event slot[SLOT_COUNT];
    
private:
    
    // Next trigger cycle
    Cycle nextTrigger = NEVER;
    

    //
    // Event tables
    //
    
    // Lookup tables
    EventID bplDMA[2][7][HPOS_CNT];  // [Hires][No of bitplanes][DMA cycle]
    EventID dasDMA[64][HPOS_CNT];    // [Bits 0 .. 5 of DMACON]

    // Currently scheduled events
    EventID bplEvent[HPOS_CNT];
    EventID dasEvent[HPOS_CNT];

    // Jump tables connecting the scheduled events
    u8 nextBplEvent[HPOS_CNT];
    u8 nextDasEvent[HPOS_CNT];
    

    //
    // Execution control
    //

public:
    
    // Action flags controlling the HSYNC handler
    u64 hsyncActions;

    // Pending register changes (used for emulating change delays)
    RegChangeRecorder<8> changeRecorder;

    
    //
    // Counters
    //
    
    // Agnus has been emulated up to this master clock cycle
    Cycle clock;

    // The current beam position
    Beam pos;

    // Information about the current frame
    Frame frame;

    
    //
    // Registers
    //

    // Memory mask (determines the width of all DMA memory pointer registers)
    u32 ptrMask;
    
    // A copy of BPLCON0 and BPLCON1 (Denise has its own copies)
    u16 bplcon0;
    u16 bplcon1;
    
    // The DMA control register
    u16 dmacon;

    // The disk DMA pointer
    u32 dskpt;

    // The audio DMA pointers and pointer latches
    u32 audpt[4];
    u32 audlc[4];

    // The bitplane DMA pointers
    u32 bplpt[6];

    // The bitplane modulo registers for odd bitplanes
    i16 bpl1mod;

    // The bitplane modulo registers for even bitplanes
    i16 bpl2mod;

    // The sprite DMA pointers
    u32 sprpt[8];


    //
    // Derived values
    //
    
    /* Values of BPLCON0 and DMACON at the DDFSTRT trigger cycle. Both
     * variables are set at the beginning of each rasterline and updated
     * on-the-fly if BPLCON0 or DMACON changes before the trigger conditions
     * has been reached.
     */
    u16 bplcon0AtDDFStrt;
    u16 dmaconAtDDFStrt;
    
    /* This value is updated in the hsync handler with the lowest 6 bits of
     * dmacon if the master enable bit is 1 or set to 0 if the master enable
     * bit is 0. It is used as an offset into the DAS lookup tables.
     */
    u16 dmaDAS;

    /* Horizontal shift values derived from BPLCON1. All four values are
     * extracted in setBPLCON1() and utilized to emulate horizontal scrolling.
     * They control at which DMA cycles the BPLDAT registers are transfered
     * into the shift registers.
     */
    i8 scrollLoresOdd;
    i8 scrollLoresEven;
    i8 scrollHiresOdd;
    i8 scrollHiresEven;

    // Set in the hsync handler to remember the returned value of inBplDmaLine()
    bool bplDmaLine;

    
    //
    // Data bus
    //

public:
    
    // Recorded DMA values for all cycles in the current rasterline
    u16 busValue[HPOS_CNT];

    // Recorded DMA usage for all cycles in the current rasterline
    BusOwner busOwner[HPOS_CNT];

    
    //
    // Signals from other components
    //
    
private:

    // DMA request from Paula
    bool audxDR[4];
    bool audxDSR[4];
    
    /* Blitter slow down. The BLS signal indicates that the CPU's request to
     * access the bus has been denied for three or more consecutive cycles.
     */
    bool bls;


    //
    // Display Data Fetch (DDF)
    //

public:

    /* Register DDFSTRT and DDFSTOP define the area where the system performs
     * bitplane DMA. From a hardware engineer's point of view, these registers
     * are completely independent of DIWSTRT and DIWSTOP. From a software
     * engineer's point of view they appear closely related though. To get
     * graphics output right, bitplane DMA has to start closely before the
     * display window opens (left border ends) and to stop closely after the
     * display window closes (right border begins).
     * DDFSTRT and DDFSTOP have a resolution of four lowres pixels (unlike
     * DIWSTRT and DIWSTOP which have a resolution of one lores pixels).
     *
     * I haven't found detailed information about the how the DDF logic is
     * implemented in hardware inside Agnus. If you have such information,
     * please let me know. For the time being, I base my implementation on the
     * following assumptions:
     *
     * 1. The four-pixel resolution is achieved by ignoring the two lower bits
     *    in DDFSTRT and DDFSTOP.
     * 2. The actual DMA start position depends solely on DDFSTRT. In hires
     *    mode, the start position always matches DDFSTRT. In lores mode, it
     *    matches DDFSTRT only if DDFSTRT is dividable by 8. Otherwise, the
     *    value is rounded up to the next position dividable by eight (because
     *    the lower two bits are always 0, this is equivalent to adding 4).
     * 3. The actual DMA stop position depends on both DDFSTRT and DDFSTOP.
     *    Hence, if DDFSTRT changes, the stop position needs to be recomputed
     *    even if DDFSTOP hasn't changed.
     * 4. Agnus switches bitplane DMA on and off by constantly comparing the
     *    horizontal raster position with the DMA start and stop positions that
     *    have been computed out of DDFSTRT and DDFSTOP. Hence, if DDFSTRT
     *    changes before DMA is switched on, the changed values takes effect
     *    immediately (i.e., in the same rasterline). If it changes when DMA is
     *    already on, the change takes effect in the next rasterline.
     * 5. The values written to DDFSTRT and DDFSTOP are not clipped if they
     *    describe a position outside the two hardware stops (at 0x18 and 0xD8).
     *    E.g., if a very small value is written to DDFSTRT, Agnus starts
     *    incrementing the bitplane pointers even if the left hardware stop is
     *    not crossed yet. Agnus simply refused to perform DMA until the
     *    hardware stop has been crossed.
     */

    // The display data fetch registers
    u16 ddfstrt;
    u16 ddfstop;

    /* At the end of a rasterline, these variables conain the DMA cycles
     * where the hpos counter matched ddfstrt or ddfstop, respectively. A
     * value of -1 indicates that no matching event took place.
     */
    i16 ddfstrtReached;
    i16 ddfstopReached;

    /* At the end of a rasterline, this variable contains the DDF state.
     */
    DDFState ddfState;

    /* This variable is used to emulate the OCS "scanline effect". If DDFSTRT
     * is set to value smaller than the left hardware stop at 0x18, early DMA
     * access is enables every other line. In this case, this variable stores
     * the number of the next line where early DMA is possible.
     */
    i16 ocsEarlyAccessLine;

    // DDF flipflops
    bool ddfVFlop;

    // Display data fetch window in lores and hires mode
    DDF<false> ddfLores;
    DDF<true> ddfHires;
    
    
    //
    // Display Window (DIW)
    //

    /* The Amiga limits the visible screen area by an upper, a lower, a left,
     * and a right border. The border encloses an area called the Display
     * Window (DIW). The color of the pixels inside the display window depends
     * on the bitplane data. The pixels of the border area are always drawn in
     * the background color (which might change inside the border area).
     * The size of the display window is controlled by two registers called
     * DIWSTRT and DIWSTOP. They contain the vertical and horizontal positions
     * at which the window starts and stops. The resolution of vertical start
     * and stop is one scan line. The resolution of horizontal start and stop
     * is one low-resolution pixel.
     *
     * I haven't found detailed information about the how the DIW logic is
     * implemented in hardware inside Agnus. If you have such information,
     * please let me know. For the time being, I base my implementation on the
     * following assumptions:
     *
     * 1. Denise contains a single flipflop controlling the display window
     *    horizontally. The flop is cleared inside the border area and set
     *    inside the display area.
     * 2. When hpos matches the position in DIWSTRT, the flipflop is set.
     * 3. When hpos matches the position in DIWSTOP, the flipflop is reset.
     * 4. The smallest valid value for DIWSTRT is $02. If it is smaller, it is
     *    not recognised.
     * 5. The largest valid value for DIWSTOP is $(1)C7. If it is larger, it is
     *    not recognised.
     */

    // Register values as they have been written by pokeDIWSTRT/STOP()
    u16 diwstrt;
    u16 diwstop;

    /* Extracted display window coordinates
     *
     * The coordinates are computed out of diwstrt and diwstop and set in
     * pokeDIWSTRT/STOP(). The following horizontal values are possible:
     *
     *    diwHstrt : $02  ... $FF   or -1
     *    diwHstop : $100 ... $1C7  or -1
     *
     * A -1 is assigned if DIWSTRT or DIWSTOP are written with values that
     * result in coordinates outside the valid range.
     */
    i16 diwHstrt;
    i16 diwHstop;
    i16 diwVstrt;
    i16 diwVstop;

    /* Value of the DIW flipflops. Variable vFlop stores the value of the
     * vertical DIW flipflop. The value is updated at the beginning of each
     * rasterline and cannot change thereafter. Variable hFlop stores the value
     * of the horizontal DIW flipflop as it was at the beginning of the
     * rasterline. To find out the value of the horizontal flipflop inside or
     * at the end of a rasterline, hFlopOn and hFlopOff need to be evaluated.
     */
    bool diwVFlop;
    bool diwHFlop;

    /* At the end of a rasterline, these variable conains the pixel coordinates
     * where the hpos counter matched diwHstrt or diwHstop, respectively. A
     * value of -1 indicates that no matching event took place.
     */
    i16 diwHFlopOn;
    i16 diwHFlopOff;


    //
    // Sprites
    //

    /* The vertical trigger positions of all 8 sprites. Note that Agnus knows
     * nothing about the horizontal trigger positions (only Denise does).
     */
    i16 sprVStrt[8];
    i16 sprVStop[8];

    // The current DMA states of all 8 sprites
    SprDMAState sprDmaState[8];


    //
    // Initializing
    //
    
public:
    
    Agnus(Amiga& ref);

    const char *getDescription() const override { return "Agnus"; }

private:
    
    void initLookupTables();
    void initBplEventTableLores();
    void initBplEventTableHires();
    void initDasEventTable();

    void _reset(bool hard) override;
    
    
    //
    // Configuring
    //
    
public:
    
    const AgnusConfig &getConfig() const { return config; }
    
    long getConfigItem(Option option) const;
    bool setConfigItem(Option option, long value) override;
    
    bool isOCS() const { return config.revision == AGNUS_OCS; }
    bool isECS() const { return config.revision != AGNUS_OCS; }
    
    // Returns the chip identification bits of this Agnus (shows up in VPOSR)
    i16 idBits();
    
    // Returns the maximum amout of Chip Ram in KB this Agnus can handle
    isize chipRamLimit();
        
    // Returns the line in which the VERTB interrupt gets triggered
    int vStrobeLine() { return isECS() || MIMIC_UAE ? 0 : 1; }
    
    // Returns the connected bits in DDFSTRT / DDFSTOP
    u16 ddfMask() { return isOCS() ? 0xFC : 0xFE; }
    
    /* Returns true if Agnus is able to access to the Slow Ram area. The ECS
     * revision of Agnus has a special feature that makes Slow Ram accessible
     * for DMA. In the 512 MB Chip Ram + 512 Slow Ram configuration, the Slow
     * Ram is mapped into the second Chip Ram segment. The OCS Agnus does not
     * have this feature. It has access to Chip Ram, only.
     */
    bool slowRamIsMirroredIn();
        
    
    //
    // Analyzing
    //
    
public:
    
    AgnusInfo getInfo() { return HardwareComponent::getInfo(info); }
    EventInfo getEventInfo();
    EventSlotInfo getEventSlotInfo(isize nr);
    
private:
    
    void _inspect() override;
    void _dump(Dump::Category category, std::ostream& os) const override;
    
    void inspectEvents(EventInfo &info) const;
    void inspectEvents() { synchronized { inspectEvents(eventInfo); } }
    void inspectEventSlot(EventInfo &info, EventSlot nr) const;
    void inspectEventSlot(EventSlot nr) { inspectEventSlot(eventInfo, nr); }

public:
    
    AgnusStats getStats() { return stats; }
    
private:
    
    void clearStats();
    void updateStats();
    
    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.revision
        << ptrMask;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker

        << clock;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker
        
        >> slot
        << nextTrigger

        << bplEvent
        << dasEvent
        << nextBplEvent
        << nextDasEvent

        << hsyncActions
        >> changeRecorder

        >> pos
        >> frame

        << bplcon0
        << bplcon1
        << dmacon
        << dskpt
        << audpt
        << audlc
        << bplpt
        << bpl1mod
        << bpl2mod
        << sprpt

        << bplcon0AtDDFStrt
        << dmaconAtDDFStrt
        << dmaDAS
        << scrollLoresOdd
        << scrollLoresEven
        << scrollHiresOdd
        << scrollHiresEven
        << bplDmaLine
        
        << busValue
        << busOwner

        << audxDR
        << audxDSR
        << bls

        << ddfstrt
        << ddfstop
        << ddfstrtReached
        << ddfstopReached
        << ddfState
        << ocsEarlyAccessLine
        << ddfVFlop
        >> ddfLores
        >> ddfHires

        << diwstrt
        << diwstop
        << diwHstrt
        << diwHstop
        << diwVstrt
        << diwVstop
        << diwVFlop
        << diwHFlop
        << diwHFlopOn
        << diwHFlopOff

        << sprVStrt
        << sprVStop
        << sprDmaState;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Examining the current frame
    //

public:

    /* Returns the number of master cycles in the current frame. The result
     * depends on the number of lines that are drawn. This values varies
     * between long and short frames.
     */
    Cycle cyclesInFrame() const;

    /* Returns the master cycle belonging to beam position (0,0). The first
     * function treats (0,0) as the upper left position of the current frame.
     * The second function referes to the next frame.
     */
    Cycle startOfFrame() const;
    Cycle startOfNextFrame() const;

    // Indicates if the provided master cycle belongs to a specific frame.
    bool belongsToPreviousFrame(Cycle cycle) const;
    bool belongsToCurrentFrame(Cycle cycle) const;
    bool belongsToNextFrame(Cycle cycle) const;


    //
    // Examining the current rasterline
    //

public:

    // Indicates if the electron beam is inside the VBLANK area
    bool inVBlank() const { return pos.v < 26; }

    // Indicates if the current rasterline is the last line in this frame
    bool inLastRasterline() const { return pos.v == frame.lastLine(); }

    // Indicates if bitplane DMA is enabled in the current rasterline
    bool inBplDmaLine() const { return inBplDmaLine(dmacon, bplcon0); }
    bool inBplDmaLine(u16 dmacon, u16 bplcon0) const;

    // Indicates if the electron beam is inside a certain DMA area
    bool inLoresDmaAreaEven(i16 pos) const {
        return !(pos & 4) && pos >= ddfLores.strtEven && pos < ddfLores.stopEven; }
    bool inLoresDmaAreaOdd(i16 pos) const {
        return (pos & 4) && pos >= ddfLores.strtOdd && pos < ddfLores.stopOdd; }
    bool inHiresDmaAreaEven(i16 pos) const {
        return !(pos & 2) && pos >= ddfHires.strtEven && pos < ddfHires.stopEven; }
    bool inHiresDmaAreaOdd(i16 pos) const {
        return (pos & 2) && pos >= ddfHires.strtOdd && pos < ddfHires.stopOdd; }
    
    // Returns the pixel position for the current horizontal position
    Pixel ppos(i16 posh) const { return (posh * 4) + 2; }
    Pixel ppos() const { return ppos(pos.h); }

    
    //
    // Working with the beam position
    //

public:

    /* Translates a beam position to a master cycle. The beam position must be
     * a position inside the current frame.
     */
    Cycle beamToCycle(Beam beam) const;

    /* Translates a master cycle to a beam position. The beam position must
     * belong to the current frame.
     */
    Beam cycleToBeam(Cycle cycle) const;

    /* Advances a beam position by a given number of cycles. Note that only
     * the horizontal component is wrapped over.
     */
    Beam addToBeam(Beam beam, Cycle cycles) const;


    //
    // Controlling DMA
    //

    // Returns true if the Blitter has priority over the CPU
    static bool bltpri(u16 v) { return GET_BIT(v, 10); }
    bool bltpri() const { return bltpri(dmacon); }

    // Returns true if a certain DMA channel is enabled
    template <int x> static bool auddma(u16 v);
    static bool bpldma(u16 v) { return (v & DMAEN) && (v & BPLEN); }
    static bool copdma(u16 v) { return (v & DMAEN) && (v & COPEN); }
    static bool bltdma(u16 v) { return (v & DMAEN) && (v & BLTEN); }
    static bool sprdma(u16 v) { return (v & DMAEN) && (v & SPREN); }
    static bool dskdma(u16 v) { return (v & DMAEN) && (v & DSKEN); }
    template <int x> bool auddma() const { return auddma<x>(dmacon); }
    bool bpldma() const { return bpldma(dmacon); }
    bool copdma() const { return copdma(dmacon); }
    bool bltdma() const { return bltdma(dmacon); }
    bool sprdma() const { return sprdma(dmacon); }
    bool dskdma() const { return dskdma(dmacon); }
    
    void enableBplDmaOCS();
    void disableBplDmaOCS();
    void enableBplDmaECS();
    void disableBplDmaECS();

    
    //
    // Managing DMA pointers
    //
    
    // Disk DMA
    void pokeDSKPTH(u16 value);
    void pokeDSKPTL(u16 value);

    // Audio DMA
    template <int x> void pokeAUDxLCH(u16 value);
    template <int x> void pokeAUDxLCL(u16 value);
    template <int x> void reloadAUDxPT() { audpt[x] = audlc[x]; }
    
    // Bitplane DMA
    template <int x> void pokeBPLxPTH(u16 value);
    template <int x> void pokeBPLxPTL(u16 value);
    template <int x> void setBPLxPTH(u16 value);
    template <int x> void setBPLxPTL(u16 value);

    void pokeBPL1MOD(u16 value);
    void setBPL1MOD(u16 value);
    void pokeBPL2MOD(u16 value);
    void setBPL2MOD(u16 value);

    template <int x> void addBPLMOD() {
        bplpt[x] += (x % 2) ? bpl2mod : bpl1mod;
    }

    // Sprite DMA
    template <int x> void pokeSPRxPTH(u16 value);
    template <int x> void setSPRxPTH(u16 value);
    template <int x> void pokeSPRxPTL(u16 value);
    template <int x> void setSPRxPTL(u16 value);
    template <int x> void pokeSPRxPOS(u16 value);
    template <int x> void pokeSPRxCTL(u16 value);

private:
    
    // Checks whether a write to a pointer register sould be dropped
    bool dropWrite(BusOwner owner);

    
    //
    // Performing DMA
    //

public:
        
    /* Checks if the bus is currently available for the specified resource.
     */
    template <BusOwner owner> bool busIsFree() const;

    /* Attempts to allocate the bus for the specified resource.
     * Returns true if the bus was successfully allocated.
     * On success, the bus owner is recorded in the busOwner array.
     */
    template <BusOwner owner> bool allocateBus();

    // Performs a DMA read
    u16 doDiskDMA();
    template <int channel> u16 doAudioDMA();
    template <int channel> u16 doBitplaneDMA();
    template <int channel> u16 doSpriteDMA();
    u16 doCopperDMA(u32 addr);
    u16 doBlitterDMA(u32 addr);

    // Performs a DMA write
    void doDiskDMA(u16 value);
    void doCopperDMA(u32 addr, u16 value);
    void doBlitterDMA(u32 addr, u16 value);

    // Transmits a DMA request from Agnus to Paula
    template <int channel> void setAudxDR() { audxDR[channel] = true; }
    template <int channel> void setAudxDSR() { audxDSR[channel] = true; }

    // Getter and setter for the BLS signal (Blitter slow down)
    bool getBLS() { return bls; }
    void setBLS(bool value) { bls = value; }


    //
    // Managing the DMA time slot tables
    //
    
public:

    // Removes all events from the BPL event table
    void clearBplEvents();

    // Renews all events in the BPL event table
    void updateBplEvents(u16 dmacon, u16 bplcon0, int first = 0, int last = HPOS_MAX);
    void updateBplEvents(int first = 0, int last = HPOS_MAX) {
        updateBplEvents(dmacon, bplcon0, first, last); }
    void updateDrawingFlags(bool hires);
        
    // Removes all events from the DAS event table
    void clearDasEvents();

    // Renews all events in the the DAS event table
    void updateDasEvents(u16 dmacon);

private:

    // Updates the jump table for the bplEvent table
    void updateBplJumpTable(i16 end = HPOS_MAX);

    // Updates the jump table for the dasEvent table
    void updateDasJumpTable(i16 end = HPOS_MAX);

    // Dumps an event table for debugging
    void dumpEventTable(const EventID *table, char str[256][3], isize from, isize to) const;

public:
    
    // Dumps the BPL or DAS event table for debugging
    void dumpBplEventTable(int from, int to) const;
    void dumpBplEventTable() const;
    void dumpDasEventTable(int from, int to) const;
    void dumpDasEventTable() const;

    
    //
    // Accessing registers
    //
    
public:

    // DMACONR, DMACON
    u16 peekDMACONR();
     void pokeDMACON(u16 value);
     void setDMACON(u16 oldValue, u16 newValue);
    
    // VHPOSR, VHPOS, VPOSR, VPOS
    u16 peekVHPOSR();
    void pokeVHPOS(u16 value);
    u16 peekVPOSR();
    void pokeVPOS(u16 value);
    
    // DIWSTRT, DIWSTOP
    template <Accessor s> void pokeDIWSTRT(u16 value);
    template <Accessor s> void pokeDIWSTOP(u16 value);
    void setDIWSTRT(u16 value);
    void setDIWSTOP(u16 value);

    // DDFSTRT, DDFSTOP
    void pokeDDFSTRT(u16 value);
    void pokeDDFSTOP(u16 value);
    void setDDFSTRT(u16 old, u16 value);
    void setDDFSTOP(u16 old, u16 value);

    // BPLCON0 and BPLCON1
    void pokeBPLCON0(u16 value);
    void setBPLCON0(u16 oldValue, u16 newValue);
    void setBPLCON0(u16 newValue) { setBPLCON0(bplcon0, newValue); }
    bool hires() { return GET_BIT(bplcon0, 15); }
    bool lores() { return GET_BIT(bplcon0, 10); }
    bool ersy() { return GET_BIT(bplcon0, 1); }

    void pokeBPLCON1(u16 value);
    void setBPLCON1(u16 oldValue, u16 newValue);
    void setBPLCON1(u16 newValue) { setBPLCON1(bplcon1, newValue); }
    
    
    //
    // Managing the data fetch window
    //
    
    // Sets up the likely DDF values for the next rasterline
    void predictDDF();

private:

    void computeDDFWindow();
    void computeDDFWindowOCS();
    void computeDDFWindowECS();
    // void computeStandardDDFWindow(i16 strt, i16 stop);

    //
    //
    //
    
public:

    /* Returns the Agnus view of the BPU bits. The value determines the number
     * of enabled DMA channels. It is computed out of the three BPU bits stored
     * in BPLCON0, but not identical with them. The value differs if the BPU
     * bits reflect an invalid bit pattern. Compare with Denise::bpu() which
     * returns the Denise view of the BPU bits.
     */
    static int bpu(u16 v);
    int bpu() { return bpu(bplcon0); }


    //
    // Operating the device
    //
    
public:

    // Executes the device for a single cycle
    void execute();

    // Executes the device until the target clock is reached
    void executeUntil(Cycle targetClock);

    // Executes the device to the beginning of the next E clock cycle
    void syncWithEClock();

    // Returns true if the device is in sync with the E clock
    // bool inSyncWithEClock();

    // Executes the device until the CPU can acquire the bus
    void executeUntilBusIsFree();
    void executeUntilBusIsFreeForCIA();
    
    // Schedules a register to change its value
    void recordRegisterChange(Cycle delay, u32 addr, u16 value);

private:

    // Performs all pending register changes
    void updateRegisters();

    // Executes the first sprite DMA cycle
    template <int nr> void executeFirstSpriteCycle();

    // Executes the second sprite DMA cycle
    template <int nr> void executeSecondSpriteCycle();

    // Updates the sprite DMA status in cycle 0xDF
    void updateSpriteDMA();

    // Finishes up the current rasterline
    void hsyncHandler();

    // Finishes up the current frame
    void vsyncHandler();


    //
    // Handling events
    //

public:

    // Triggers the vertical blank interrupt
    void serviceVblEvent();

private:
    
    // Schedule the next VBL event
    void scheduleStrobe0Event();
    void scheduleStrobe1Event();
    void scheduleStrobe2Event();

    //
    // Class extensions
    //

#include "EventHandler.h"

};
