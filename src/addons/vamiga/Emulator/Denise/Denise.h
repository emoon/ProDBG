// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _DENISE_H
#define _DENISE_H

#include "AmigaComponent.h"
#include "Colors.h"
#include "PixelEngine.h"
#include "ScreenRecorder.h"

class Denise : public AmigaComponent {

    friend class PixelEngine;
    
    // Current configuration
    DeniseConfig config;

    // Result of the latest inspection
    DeniseInfo info;

    // Sprite information recorded in the previous frame (shown by the GUI)
    SpriteInfo latchedSpriteInfo[8];
    u64 latchedSpriteData[8][VPOS_CNT];
    
    // Sprite information recorded in the current frame (constantly changing)
    SpriteInfo spriteInfo[8];
    u64 spriteData[8][VPOS_CNT];
    
    
    //
    // Sub components
    //
    
public:
    
    // Color synthesizer for computing RGBA values
    PixelEngine pixelEngine = PixelEngine(amiga);

    // A screen recorder for creating video streams
    ScreenRecorder screenRecorder = ScreenRecorder(amiga);


    //
    // Counters
    //
    
    // Denise has been executed up to this clock cycle
    Cycle clock = 0;


    //
    // Registers
    //
    
    // Bitplane control registers
    u16 bplcon0;
    u16 bplcon1;
    u16 bplcon2;
    u16 bplcon3;

    // Bitplane control registers at cycle 0 in the current rasterline
    u16 initialBplcon0;
    u16 initialBplcon1;
    u16 initialBplcon2;

    // Color register index for the border color (0 = background color)
    u8 borderColor;
    
    // Bitplane data registers
    u16 bpldat[6];
    
    // Sprite collision registers
    u16 clxdat;
    u16 clxcon;

    /* Parallel-to-serial shift registers. Denise transfers the current values
     * of the BPLDAT registers into these shift registers after BPLDAT1 is
     * written to. This is emulated in function fillShiftRegister().
     *
     * Note: The upper two array elements are dummy elements. We need them in
     * order to pass the array as parameter to function transposeSSE().
     */
    u16 __attribute__ ((aligned (64))) shiftReg[8];

    // Bit slices computed out of the shift registers
    u8 __attribute__ ((aligned (64))) slice[16];
    
    // Flags indicating that the shift registers have been loaded
    bool armedEven;
    bool armedOdd;

    // Extracted from BPLCON1 to emulate horizontal scrolling
    i8 pixelOffsetOdd;
    i8 pixelOffsetEven;

    
    //
    // Register change management
    //

public:

    // Ringbuffer recording control register changes
    RegChangeRecorder<128> conChanges;

    // Ringbuffers recording sprite register changes (one for each sprite pair)
    RegChangeRecorder<128> sprChanges[4];


    //
    // Sprites
    //

    // Sprite data registers (SPRxDATA, SPRxDATAB)
    u16 sprdata[8];
    u16 sprdatb[8];

    // The position and control registers of all 8 sprites
    u16 sprpos[8];
    u16 sprctl[8];

    // The serial shift registers of all 8 sprites
    u16 ssra[8];
    u16 ssrb[8];
    
    /* Indicates which sprites are curently armed. An armed sprite is a sprite
     * that will be drawn in this line.
     */
    u8 armed;

    /* Remembers the sprites that were armed in the current rasterline. Note
     * that a sprite can be armed and disarmed multiple times in a rasterline
     * by manually modifying SPRxDATA and SPRxCTL, respectively.
     */
    u8 wasArmed;

    /* Sprite clipping window
     *
     * The clipping window determines where sprite pixels can be drawn.

     *  spriteClipBegin : The first possible sprite pixel in this rasterline
     *    spriteClipEnd : The last possible sprite pixel in this rasterline + 1
     *
     * The variables are set in the hsyncHandler to their expected values.
     * In general, sprites can be drawn if we are in a bitplane DMA line as
     * testes by function inBplDmaLine(). If BPLCON0 changes in the middle
     * of rasterline, the sprite clipping window is adjusted, too. The
     * following conditions are likely to apply on a real Amiga:
     *
     * 1. Enabling sprites is always possible, even at high DMA cycle numbers.
     * 2. Disbabling sprites only has an effect until the DDFSTRT position
     *    has been reached. If sprite drawing was enabled at that position,
     *    it can't be disabled in the same rasterline any more.
     */
    PixelPos spriteClipBegin;
    PixelPos spriteClipEnd;

    
    //
    // Playfield priorities
    //

private:

    // Playfield priorities (derived from BPLCON2)
    u16 prio1;
    u16 prio2;

    
    //
    // Rasterline data
    //

    /* Four important buffers are involved in the generation of pixel data:
     *
     * bBuffer: The bitplane data buffer
     *
     * While emulating the DMA cycles of a single rasterline, Denise writes
     * the fetched bitplane data into this buffer. It contains the raw
     * bitplane bits coming out the 6 serial shift registers.
     *
     * iBuffer: The color index buffer
     *
     * At the end of each rasterline, Denise translates the fetched bitplane
     * data to color register indices. In single-playfield mode, this is a
     * one-to-one-mapping. In dual-playfield mode, the bitplane data has to
     * be split into two color indices. Only one of them is kept depending on
     * the playfield priority bit.
     *
     * mBuffer: The multiplexed color index buffer
     *
     * This buffer contains the data from the iBuffer, multiplexed with the
     * color index data coming from the sprite synthesizer.
     *
     * zBuffer: The pixel depth buffer
     *
     * When the bBuffer is translated into the iBuffer, a depth buffer is build.
     * This buffer serves multiple purposes.
     *
     * 1. The depth buffer it is used to implement the display priority. For
     *    example, it is used to decide whether to draw a sprite pixel in front
     *    of or behind a particular playing field pixel. Note: The larger the
     *    value, the closer a pixel is. In traditonal z-buffers, it is the other
     *    way round.
     *
     * 2. The depth buffer is utilized to code meta-information about the pixels
     *    in the current rasterline. This is done by coding the pixel depth with
     *    special bit patterns storing that information. E.g., the pixel depth
     *    can be used to determine, if the pixel has been drawn in dual-
     *    playfield mode or if a sprite-to-sprite collision has taken place.
     *
     * The following bit format is utilized:
     *
     * _0_ SP0 SP1 _1_ SP2 SP3 _2_ SP4 SP5 _3_ SP6 SP7 _4_ DPF PF1 PF2
     *
     *  DPF : Set if the pixel is drawn in dual-playfield mode.
     *  PF1 : Set if the pixel is solid in playfield 1.
     *  PF1 : Set if the pixel is solid in playfield 2.
     *  SPx : Set if the pixel is solid in sprite x.
     *  _x_ : Playfield priority derived from the current value in BPLCON2.
     */
    u8 bBuffer[HPIXELS + (4 * 16) + 6];
    u8 iBuffer[HPIXELS + (4 * 16) + 6];
    u8 mBuffer[HPIXELS + (4 * 16) + 6];
    u16 zBuffer[HPIXELS + (4 * 16) + 6];

    static const u16 Z_0   = 0b10000000'00000000;
    static const u16 Z_SP0 = 0b01000000'00000000;
    static const u16 Z_SP1 = 0b00100000'00000000;
    static const u16 Z_1   = 0b00010000'00000000;
    static const u16 Z_SP2 = 0b00001000'00000000;
    static const u16 Z_SP3 = 0b00000100'00000000;
    static const u16 Z_2   = 0b00000010'00000000;
    static const u16 Z_SP4 = 0b00000001'00000000;
    static const u16 Z_SP5 = 0b00000000'10000000;
    static const u16 Z_3   = 0b00000000'01000000;
    static const u16 Z_SP6 = 0b00000000'00100000;
    static const u16 Z_SP7 = 0b00000000'00010000;
    static const u16 Z_4   = 0b00000000'00001000;
 
    // Dual-playfield bits (meta-information, not used for depth)
    static const u16 Z_DPF   = 0x1;  // Both playfields transparent
    static const u16 Z_DPF1  = 0x2;  // PF1 opaque, PF2 transparent
    static const u16 Z_DPF2  = 0x3;  // PF1 transparent, PF2 opaque
    static const u16 Z_DPF12 = 0x4;  // Both playfields opaque, PF1 visible
    static const u16 Z_DPF21 = 0x5;  // Both playfields opaque, PF2 visible
    static const u16 Z_DUAL  = 0x7;  // Mask covering all DPF bits

    constexpr static const u16 Z_SP[8] = { Z_SP0, Z_SP1, Z_SP2, Z_SP3, Z_SP4, Z_SP5, Z_SP6, Z_SP7 };
    static const u16 Z_SP01234567 = Z_SP0|Z_SP1|Z_SP2|Z_SP3|Z_SP4|Z_SP5|Z_SP6|Z_SP7;
    static const u16 Z_SP0246 = Z_SP0|Z_SP2|Z_SP4|Z_SP6;
    static const u16 Z_SP1357 = Z_SP1|Z_SP3|Z_SP5|Z_SP7;
    
    static bool isSpritePixel(u16 z) {
        return (z & Z_SP01234567) > (z & ~Z_SP01234567);
    }
    template <int nr> static bool isSpritePixel(u16 z) {
        assert(nr < 8); return (z & Z_SP[nr]) > (z & ~Z_SP[nr]);
    }
    static int upperPlayfield(u16 z) {
        return ((z & Z_DUAL) == Z_DPF2 || (z & Z_DUAL) == Z_DPF21) ? 2 : 1;
    }
    
    
    //
    // Initializing
    //
    
public:

    Denise(Amiga& ref);

    void _reset(bool hard) override;

    
    //
    // Configuring
    //

public:
    
    DeniseConfig getConfig() { return config; }

    long getConfigItem(ConfigOption option);
    bool setConfigItem(ConfigOption option, long value) override;
    
private:
    
    void _dumpConfig() override;

    
    //
    // Analyzing
    //

public:
    
    DeniseInfo getInfo() { return HardwareComponent::getInfo(info); }
    SpriteInfo getSpriteInfo(int nr);
    u16 getSpriteHeight(int nr) { return latchedSpriteInfo[nr].height; }
    u16 getSpriteColor(int nr, int reg) { return latchedSpriteInfo[nr].colors[reg]; }
    u64 getSpriteData(int nr, int line) { return latchedSpriteData[nr][line]; }
    
private:
    
    void _inspect() override;
    void _dump() override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        & config.revision
        & config.clxSprSpr
        & config.clxSprPlf
        & config.clxPlfPlf;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker

        & clock;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker
        
        & bplcon0
        & bplcon1
        & bplcon2
        & bplcon3
        & initialBplcon0
        & initialBplcon1
        & initialBplcon2
        & borderColor
        & bpldat
        & clxdat
        & clxcon
        & shiftReg
        & armedEven
        & armedOdd
        & pixelOffsetOdd
        & pixelOffsetEven
        & conChanges
        & sprChanges

        & sprdata
        & sprdatb
        & sprpos
        & sprctl
        & ssra
        & ssrb
        & armed
        & wasArmed
        & spriteClipBegin
        & spriteClipEnd
        & prio1
        & prio2;
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Accessing registers
    //
    
public:

    // JOYxDATR, JOYTEST
    u16 peekJOY0DATR();
    u16 peekJOY1DATR();
    void pokeJOYTEST(u16 value);

    // DENISEID
    u16 peekDENISEID();

    // BPLCON0
    void pokeBPLCON0(u16 value);
    void setBPLCON0(u16 oldValue, u16 newValue);
    void setBPLCON0(u16 newValue) { setBPLCON0(bplcon0, newValue); }

    static bool hires(u16 v) { return GET_BIT(v, 15); }
    bool hires() { return hires(bplcon0); }
    static bool lores(u16 v) { return !hires(v); }
    bool lores() { return lores(bplcon0); }
    static bool dbplf(u16 v) { return GET_BIT(v, 10); }
    bool dbplf() { return dbplf(bplcon0); }
    static bool lace(u16 v) { return GET_BIT(v, 2); }
    bool lace() { return lace(bplcon0); }
    static bool ham(u16 v) { return (v & 0x8800) == 0x0800; }
    bool ham() { return ham(bplcon0); }
    static bool ecsena(u16 v) { return GET_BIT(v, 0); }
    bool ecsena() { return ecsena(bplcon0); }

    /* Returns the Denise view of the BPU bits. The value determines how many
     * shift registers are loaded with the values of their corresponding
     * BPLxDAT registers at the end of a fetch unit. It is computed out of the
     * three BPU bits stored in BPLCON0, but not identical with them. The value
     * differs if the BPU bits reflect an invalid bit pattern.
     * Compare with Agnus::bpu() which returns the Agnus view of the BPU bits.
     */
    static int bpu(u16 v);
    int bpu() { return bpu(bplcon0); }

    // BPLCON1
    void pokeBPLCON1(u16 value);
    void setBPLCON1(u16 value);

    // BPLCON2
    void pokeBPLCON2(u16 value);
    void setBPLCON2(u16 value);
    static int PF2PRI(u16 v) { return GET_BIT(v, 6); }
    bool PF2PRI() { return PF2PRI(bplcon2); }

    // Computes the z buffer depth for playfield 1 or 2
    static u16 zPF(u16 priorityBits);
    static u16 zPF1(u16 bplcon2) { return zPF(bplcon2 & 7); }
    static u16 zPF2(u16 bplcon2) { return zPF((bplcon2 >> 3) & 7); }

    // BPLCON3
    void pokeBPLCON3(u16 value);
    void setBPLCON3(u16 value);
    static int BRDRBLNK(u16 v) { return GET_BIT(v, 5); }
    bool BRDRBLNK() { return BRDRBLNK(bplcon3); }

    // CLXDAT, CLXCON
    u16 peekCLXDAT();
    void pokeCLXCON(u16 value);
    template <int x> u16 getENSP() { return GET_BIT(clxcon, 12 + (x/2)); }
    u16 getENBP1() { return (clxcon >> 6) & 0b010101; }
    u16 getENBP2() { return (clxcon >> 6) & 0b101010; }
    u16 getMVBP1() { return clxcon & 0b010101; }
    u16 getMVBP2() { return clxcon & 0b101010; }
    
    // BPLxDAT
    template <int x, Accessor s> void pokeBPLxDAT(u16 value);
    template <int x> void setBPLxDAT(u16 value);

    // SPRxPOS, SPRxCTL
    template <int x> void pokeSPRxPOS(u16 value);
    template <int x> void pokeSPRxCTL(u16 value);

    // SPRxDATA, SPRxDATB
    template <int x> void pokeSPRxDATA(u16 value);
    template <int x> void pokeSPRxDATB(u16 value);

    // COLORxx
    template <Accessor s, int xx> void pokeCOLORxx(u16 value);

    
    //
    // Handling sprites
    //

public:
    
    // Returns the horizontal position of a sprite in sprite coordinates
    template <int x> i16 sprhpos() { return ((sprpos[x] & 0xFF) << 1) | (sprctl[x] & 0x01); }

    // Returns the horizontal position of a sprite in pixel coordinates
    template <int x> i16 sprhppos() { return 2 * (sprhpos<x>() + 1); }
    
    // Checks the z buffer and returns true if a sprite pixel is visible
    bool spritePixelIsVisible(int hpos);


    //
    // Handling bitplanes
    //

public:
    
    // Transfers the bitplane registers to the shift registers
    void fillShiftRegisters(bool odd = true, bool even = true);

    
    //
    // Synthesizing pixels
    //
    
public:

    template <bool hiresMode> void drawOdd(int offset);
    template <bool hiresMode> void drawEven(int offset);
    template <bool hiresMode> void drawBoth(int offset);
    void drawHiresOdd()  { if (armedOdd)  drawOdd <true>  (pixelOffsetOdd);  }
    void drawHiresEven() { if (armedEven) drawEven<true>  (pixelOffsetEven); }
    void drawHiresBoth();
    void drawLoresOdd()  { if (armedOdd)  drawOdd <false> (pixelOffsetOdd);  }
    void drawLoresEven() { if (armedEven) drawEven<false> (pixelOffsetEven); }
    void drawLoresBoth();

private:

    // Translate bitplane data to color register indices
    void translate();

    // Called by translate() in single-playfield mode
    void translateSPF(int from, int to);

    // Called by translate() in dual-playfield mode
    void translateDPF(bool pf2pri, int from, int to);
    template <bool pf2pri> void translateDPF(int from, int to);

public:

    // Draws all sprites
    void drawSprites();
    
    // Draws an sprite pair. Called by drawSprites()
    template <unsigned pair> void drawSpritePair();
    template <unsigned pair> void drawSpritePair(int hstrt, int hstop,
                                                 int strt1, int strt2,
                                                 bool armed1, bool armed2);

private:
    
    // Replays all recorded sprite register changes
    template <unsigned pair> void replaySpriteRegChanges();

    // Draws a single sprite pixel
    template <int x> void drawSpritePixel(int hpos);
    template <int x> void drawAttachedSpritePixelPair(int hpos);

    // Determines the color register index for drawing the border
    void updateBorderColor();

    // Draws the left and the right border
    void drawBorder(); 
    
    
    //
    // Collision checking
    //

public:

    // Checks for sprite-sprite collisions in the current rasterline
    template <int x> void checkS2SCollisions(int start, int end);

    // Checks for sprite-playfield collisions in the current rasterline
    template <int x> void checkS2PCollisions(int start, int end);

    // Checks for playfield-playfield collisions in the current rasterline
    void checkP2PCollisions();


    //
    // Delegation methods
    //
    
public:

    // Called by Agnus at the beginning of each frame
    void vsyncHandler();

    // Called by Agnus at the beginning of each rasterline
    void beginOfLine(int vpos);

    // Called by Agnus at the end of a rasterline
    void endOfLine(int vpos);

    // Called by Agnus if the DMACON register changes
    void pokeDMACON(u16 oldValue, u16 newValue);


    //
    // Debugging
    //

public:
    
    // Gathers the sprite data for the displayed sprite
    void recordSpriteData(unsigned x);

    // Dumps the bBuffer or the iBuffer to the console
    void dumpIBuffer() { dumpBuffer(iBuffer, sizeof(iBuffer)); }
    void dumpBBuffer() { dumpBuffer(bBuffer, sizeof(bBuffer)); }
    void dumpBuffer(u8 *buffer, size_t length);

};

#endif
