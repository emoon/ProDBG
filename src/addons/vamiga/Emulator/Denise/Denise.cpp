// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"
#include "SSEUtils.h"

Denise::Denise(Amiga& ref) : AmigaComponent(ref)
{    
    subComponents = vector<HardwareComponent *> {
        
        &pixelEngine,
        &screenRecorder
    };

    config.hiddenSprites = 0;
    config.hiddenLayers = 0;
    config.hiddenLayerAlpha = 128;
    config.clxSprSpr = true;
    config.clxSprPlf = true;
    config.clxPlfPlf = true;
    
    memset(spriteInfo, 0, sizeof(spriteInfo));
    memset(latchedSpriteInfo, 0, sizeof(latchedSpriteInfo));
}

void
Denise::_reset(bool hard)
{
    RESET_SNAPSHOT_ITEMS(hard)
    
    memset(bBuffer, 0, sizeof(bBuffer));
    memset(iBuffer, 0, sizeof(iBuffer));
    memset(mBuffer, 0, sizeof(mBuffer));
    memset(zBuffer, 0, sizeof(zBuffer));
}

long
Denise::getConfigItem(Option option) const
{
    switch (option) {
            
        case OPT_DENISE_REVISION:     return config.revision;
        case OPT_BRDRBLNK:            return config.borderblank;
        case OPT_HIDDEN_SPRITES:      return config.hiddenSprites;
        case OPT_HIDDEN_LAYERS:       return config.hiddenLayers;
        case OPT_HIDDEN_LAYER_ALPHA:  return config.hiddenLayerAlpha;
        case OPT_CLX_SPR_SPR:         return config.clxSprSpr;
        case OPT_CLX_SPR_PLF:         return config.clxSprPlf;
        case OPT_CLX_PLF_PLF:         return config.clxPlfPlf;
            
        default:
            assert(false);
            return 0;
    }
}

bool
Denise::setConfigItem(Option option, long value)
{
    switch (option) {
            
        case OPT_DENISE_REVISION:
            
            if (!DeniseRevisionEnum::verify(value)) return false;
            if (config.revision == value) {
                return false;
            }
            
            config.revision = (DeniseRevision)value;
            return true;
            
        case OPT_BRDRBLNK:

            if (config.borderblank == value) {
                return false;
            }
            
            config.borderblank = value;
            return true;
            
        case OPT_HIDDEN_SPRITES:
            
            if (config.hiddenSprites == value) {
                return false;
            }
            
            config.hiddenSprites = value;
            return true;
            
        case OPT_HIDDEN_LAYERS:
            
            if (config.hiddenLayers == value) {
                return false;
            }

            config.hiddenLayers = value;
            return true;
            
        case OPT_HIDDEN_LAYER_ALPHA:
            
            if (config.hiddenLayerAlpha == value) {
                return false;
            }
            
            config.hiddenLayerAlpha = value;
            return true;

        case OPT_CLX_SPR_SPR:
            
            if (config.clxSprSpr == value) {
                return false;
            }

            config.clxSprSpr = value;
            return true;
            
        case OPT_CLX_SPR_PLF:
            
            if (config.clxSprPlf == value) {
                return false;
            }

            config.clxSprPlf = value;
            return true;
            
        case OPT_CLX_PLF_PLF:
            
            if (config.clxPlfPlf == value) {
                return false;
            }

            config.clxPlfPlf = value;
            return true;

        default:
            return false;
    }
}

void
Denise::_dumpConfig() const
{
    msg("          revision : %s\n", DeniseRevisionEnum::key(config.revision));
    msg("       borderblank : %s\n", config.borderblank ? "yes" : "no");
    msg("     hiddenSprites : %02X\n", config.hiddenSprites);
    msg("      hiddenLayers : %04X\n", config.hiddenLayers);
    msg("  hiddenLayerAlpha : %d\n", config.hiddenLayerAlpha);
    msg("         clxSprSpr : %s\n", config.clxSprSpr ? "yes" : "no");
    msg("         clxSprPlf : %s\n", config.clxSprPlf ? "yes" : "no");
    msg("         clxPlfPlf : %s\n", config.clxPlfPlf ? "yes" : "no");
}

void
Denise::_inspect()
{
    synchronized {
        
        info.bplcon0 = bplcon0;
        info.bplcon1 = bplcon1;
        info.bplcon2 = bplcon2;
        info.bpu = bpu();
        
        info.diwstrt = agnus.diwstrt;
        info.diwstop = agnus.diwstop;
        info.diwHstrt = agnus.diwHstrt;
        info.diwHstop = agnus.diwHstop;
        info.diwVstrt = agnus.diwVstrt;
        info.diwVstop = agnus.diwVstop;
        
        info.joydat[0] = amiga.controlPort1.joydat();
        info.joydat[1] = amiga.controlPort2.joydat();
        info.clxdat = 0;
        
        for (isize i = 0; i < 6; i++) {
            info.bpldat[i] = bpldat[i];
        }
        for (isize i = 0; i < 32; i++) {
            info.colorReg[i] = pixelEngine.getColor(i);
            info.color[i] = pixelEngine.getRGBA(i);
        }
    }
}

void
Denise::_dump() const
{
}

SpriteInfo
Denise::getSpriteInfo(isize nr)
{
    SpriteInfo result;
    synchronized { result = latchedSpriteInfo[nr]; }
    return result;
}

int
Denise::bpu(u16 v)
{
    // Extract the three BPU bits and check for hires mode
    int bpu = (v >> 12) & 0b111;
    bool hires = GET_BIT(v, 15);

    if (hires) {
        // return bpu < 5 ? bpu : 0; // Disable all bitplanes if value is invalid
        return bpu < 7 ? bpu : 6; 
    } else {
        return bpu < 7 ? bpu : 6; // Enable six bitplanes if value is invalid
    }
}

u16
Denise::zPF(u16 prioBits)
{
    switch (prioBits) {

        case 0: return Z_0;
        case 1: return Z_1;
        case 2: return Z_2;
        case 3: return Z_3;
        case 4: return Z_4;
    }

    return 0;
}

bool
Denise::spritePixelIsVisible(Pixel hpos) const
{
    u16 z = zBuffer[hpos];
    return (z & Z_SP01234567) > (z & ~Z_SP01234567);
}

void
Denise::fillShiftRegisters(bool odd, bool even)
{
    if (odd) armedOdd = true;
    if (even) armedEven = true;
    
    spriteClipBegin = MIN(spriteClipBegin, agnus.ppos() + 2);
    
    switch (bpu()) {
        case 6: shiftReg[5] = bpldat[5];
        case 5: shiftReg[4] = bpldat[4];
        case 4: shiftReg[3] = bpldat[3];
        case 3: shiftReg[2] = bpldat[2];
        case 2: shiftReg[1] = bpldat[1];
        case 1: shiftReg[0] = bpldat[0];
    }
    
    // On Intel machines, call the optimized SSE code
    #if (defined(__i386__) || defined(__x86_64__)) && defined(__MACH__)
    
    if (!NO_SSE) {
        transposeSSE(shiftReg, slice);
        return;
    }
    
    #endif
    
    // On all other machines, fallback to the slower standard implementation
    u32 mask = 0x8000;
    for (isize i = 0; i < 16; i++, mask >>= 1) {
        
        slice[i] =
        (!!(shiftReg[0] & mask) << 0) |
        (!!(shiftReg[1] & mask) << 1) |
        (!!(shiftReg[2] & mask) << 2) |
        (!!(shiftReg[3] & mask) << 3) |
        (!!(shiftReg[4] & mask) << 4) |
        (!!(shiftReg[5] & mask) << 5);
    }
}

template <bool hiresMode> void
Denise::drawOdd(Pixel offset)
{
    assert(!hiresMode || (agnus.pos.h & 0x3) == agnus.scrollHiresOdd);
    assert( hiresMode || (agnus.pos.h & 0x7) == agnus.scrollLoresOdd);

    static const u16 masks[7] = {
       0b000000,         // 0 bitplanes
       0b000001,         // 1 bitplanes
       0b000001,         // 2 bitplanes
       0b000101,         // 3 bitplanes
       0b000101,         // 4 bitplanes
       0b010101,         // 5 bitplanes
       0b010101          // 6 bitplanes
    };
    
    u16 mask = masks[bpu()];
    Pixel currentPixel = agnus.ppos() + offset;
    
    for (isize i = 0; i < 16; i++) {
        
        u8 index = slice[i] & mask;
        
        if (hiresMode) {
            
            // Synthesize one hires pixel
            assert(currentPixel < isizeof(bBuffer));
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b101010) | index;
            currentPixel++;
            
        } else {
            
            // Synthesize two lores pixels
            assert(currentPixel + 1 < isizeof(bBuffer));
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b101010) | index;
            currentPixel++;
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b101010) | index;
            currentPixel++;
        }
    }
 
    // Disarm and clear the shift registers
    armedOdd = false;
    shiftReg[0] = shiftReg[2] = shiftReg[4] = 0;
}

template <bool hiresMode> void
Denise::drawEven(Pixel offset)
{
    assert(!hiresMode || (agnus.pos.h & 0x3) == agnus.scrollHiresEven);
    assert( hiresMode || (agnus.pos.h & 0x7) == agnus.scrollLoresEven);
    
    static const u16 masks[7] = {
       0b000000,         // 0 bitplanes
       0b000000,         // 1 bitplanes
       0b000010,         // 2 bitplanes
       0b000010,         // 3 bitplanes
       0b001010,         // 4 bitplanes
       0b001010,         // 5 bitplanes
       0b101010          // 6 bitplanes
    };
    
    u16 mask = masks[bpu()];
    i16 currentPixel = agnus.ppos() + offset;
    
    for (isize i = 0; i < 16; i++) {

        u8 index = slice[i] & mask;

        if (hiresMode) {
            
            // Synthesize one hires pixel
            assert(currentPixel < isizeof(bBuffer));
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b010101) | index;
            currentPixel++;

        } else {
            
            // Synthesize two lores pixels
            assert(currentPixel + 1 < isizeof(bBuffer));
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b010101) | index;
            currentPixel++;
            bBuffer[currentPixel] = (bBuffer[currentPixel] & 0b010101) | index;
            currentPixel++;
        }
    }
 
    // Disarm and clear the shift registers
    armedEven = false;
    shiftReg[1] = shiftReg[3] = shiftReg[5] = 0;
}

template <bool hiresMode> void
Denise::drawBoth(Pixel offset)
{
    static const u16 masks[7] = {
        0b000000,         // 0 bitplanes
        0b000001,         // 1 bitplanes
        0b000011,         // 2 bitplanes
        0b000111,         // 3 bitplanes
        0b001111,         // 4 bitplanes
        0b011111,         // 5 bitplanes
        0b111111          // 6 bitplanes
    };
    
    u16 mask = masks[bpu()];
    i16 currentPixel = agnus.ppos() + offset;
    
    for (isize i = 0; i < 16; i++) {
        
        u8 index = slice[i] & mask;
        
        if (hiresMode) {
            
            // Synthesize one hires pixel
            assert(currentPixel < isizeof(bBuffer));
            bBuffer[currentPixel++] = index;
            
        } else {
            
            // Synthesize two lores pixels
            assert(currentPixel + 1 < isizeof(bBuffer));
            bBuffer[currentPixel++] = index;
            bBuffer[currentPixel++] = index;
        }
    }
    
    // Disarm and clear the shift registers
    armedEven = armedOdd = false;
    for (isize i = 0; i < 6; i++) shiftReg[i] = 0;
}

void
Denise::drawHiresBoth()
{
    if (armedOdd && armedEven && pixelOffsetOdd == pixelOffsetEven) {

        assert((agnus.pos.h & 0x3) == agnus.scrollHiresOdd);
        assert((agnus.pos.h & 0x3) == agnus.scrollHiresEven);
        drawBoth<true>(pixelOffsetOdd);

    } else {
    
        drawHiresOdd();
        drawHiresEven();
    }
}

void
Denise::drawLoresBoth()
{
    if (armedOdd && armedEven && pixelOffsetOdd == pixelOffsetEven) {

        assert((agnus.pos.h & 0x7) == agnus.scrollLoresOdd);
        assert((agnus.pos.h & 0x7) == agnus.scrollLoresEven);
        drawBoth<false>(pixelOffsetOdd);

    } else {
    
        drawLoresOdd();
        drawLoresEven();
    }
}

void
Denise::translate()
{
    Pixel pixel = 0;

    // Start with the playfield state as it was at the beginning of the line
    PFState state;
    state.pf2pri = PF2PRI(initialBplcon2);
    state.ham    = ham(initialBplcon0);
    state.prio1  = zPF1(initialBplcon2);
    state.prio2  = zPF2(initialBplcon2);
    bool dual    = dbplf(initialBplcon0);

    // Add a dummy register change to ensure we draw until the line ends
    conChanges.insert(sizeof(bBuffer), RegChange { SET_NONE, 0 });

    // Iterate over all recorded register changes
    for (isize i = conChanges.begin(); i != conChanges.end(); i = conChanges.next(i)) {

        Cycle trigger = conChanges.keys[i];
        RegChange &change = conChanges.elements[i];

        // Translate a chunk of bitplane data
        if (dual) {
            translateDPF(pixel, trigger, state);
        } else {
            translateSPF(pixel, trigger, state);
        }
        pixel = trigger;

        // Apply the register change
        switch (change.addr) {

            case SET_BPLCON0_DENISE:
                dual = dbplf(bplcon0);
                state.ham = ham(change.value);
                break;

            case SET_BPLCON2:
                state.pf2pri = PF2PRI(change.value);
                state.prio1 = zPF1(change.value);
                state.prio2 = zPF2(change.value);
                break;

            default:
                assert(change.addr == SET_NONE);
                break;
        }
    }

    // Clear the history cache
    conChanges.clear();
}

void
Denise::translateSPF(Pixel from, Pixel to, PFState &state)
{
    /* Check for invalid bitplane modes. If the priority of the second bitplane
     * is set to an invalid value (> 4), Denise ignores the data from the first
     * four bitplanes whereever the fifth bitplane is set to 1. Some demos such
     * as "Planet Rocklobster" (Oxyron) show that this kind of bitplane
     * elimination does not happen in HAM mode.
     *
     * Relevant tests in the vAmigaTS test suite:
     * Denise/BPLCON0/invprio0 to Denise/BPLCON0/invprio3
     */
    
    if (unlikely(!state.prio2 && !state.ham)) {
        
        for (Pixel i = from; i < to; i++) {

             u8 s = bBuffer[i];

             assert(PixelEngine::isRgbaIndex(s));
             iBuffer[i] = mBuffer[i] = (s & 0x10) ? (s & 0x30) : s;
             zBuffer[i] = 0;
         }
        return;
    }
    
    // Translate the usual way
    for (Pixel i = from; i < to; i++) {
        
        u8 s = bBuffer[i];
        
        assert(PixelEngine::isRgbaIndex(s));
        iBuffer[i] = mBuffer[i] = s;
        zBuffer[i] = s ? state.prio2 : 0;
    }
}

void
Denise::translateDPF(Pixel from, Pixel to, PFState &state)
{
    if (state.pf2pri) {
        translateDPF<true>(from, to, state);
    } else {
        translateDPF<false>(from, to, state);
    }
}

template <bool pf2pri> void
Denise::translateDPF(Pixel from, Pixel to, PFState &state)
{
    /* If the priority of a playfield is set to an illegal value (prio1 or
     * prio2 will be 0 in that case), all pixels are drawn transparent.
     */
    u8 mask1 = state.prio1 ? 0b1111 : 0b0000;
    u8 mask2 = state.prio2 ? 0b1111 : 0b0000;

    for (Pixel i = from; i < to; i++) {

        u8 s = bBuffer[i];

        // Determine color indices for both playfields
        u8 index1 = (((s & 1) >> 0) | ((s & 4) >> 1) | ((s & 16) >> 2));
        u8 index2 = (((s & 2) >> 1) | ((s & 8) >> 2) | ((s & 32) >> 3));

        if (index1) {
            if (index2) {

                // PF1 is solid, PF2 is solid
                if (pf2pri) {
                    iBuffer[i] = mBuffer[i] = (index2 | 0b1000) & mask2;
                    zBuffer[i] = state.prio2 | Z_DPF21;
                } else {
                    iBuffer[i] = mBuffer[i] = index1 & mask1;
                    zBuffer[i] = state.prio1 | Z_DPF12;
                }

            } else {

                // PF1 is solid, PF2 is transparent
                iBuffer[i] = mBuffer[i] = index1 & mask1;
                zBuffer[i] = state.prio1 | Z_DPF1;
            }

        } else {
            if (index2) {

                // PF1 is transparent, PF2 is solid
                iBuffer[i] = mBuffer[i] = (index2 | 0b1000) & mask2;
                zBuffer[i] = state.prio2 | Z_DPF2;

            } else {

                // PF1 is transparent, PF2 is transparent
                iBuffer[i] = mBuffer[i] = 0;
                zBuffer[i] = Z_DPF;
            }
        }
    }
}

void
Denise::drawSprites()
{
    if (wasArmed) {
        
        if (wasArmed & 0b11000000) drawSpritePair<3>();
        if (wasArmed & 0b00110000) drawSpritePair<2>();
        if (wasArmed & 0b00001100) drawSpritePair<1>();
        if (wasArmed & 0b00000011) drawSpritePair<0>();
        
        // Record sprite data in debug mode
        if (amiga.inDebugMode()) {
            for (isize i = 0; i < 8; i++) {
                if (GET_BIT(wasArmed, i)) recordSpriteData(i);
            }
        }
    }
    
    /* If a sprite was armed, the code above has been executed which means
     * that all recorded register changes have been applied and the relevant
     * sprite registers are all up to date at this time. For unarmed sprites,
     * however, the register change buffers may contain unprocessed entried.
     * We replay those to get the sprite registers up to date.
     */
    if (!sprChanges[3].isEmpty()) replaySpriteRegChanges<3>();
    if (!sprChanges[2].isEmpty()) replaySpriteRegChanges<2>();
    if (!sprChanges[1].isEmpty()) replaySpriteRegChanges<1>();
    if (!sprChanges[0].isEmpty()) replaySpriteRegChanges<0>();
}

template <isize pair> void
Denise::drawSpritePair()
{
    assert(pair < 4);

    const isize sprite1 = 2 * pair;
    const isize sprite2 = 2 * pair + 1;

    Pixel strt1 = sprhppos<sprite1>();
    Pixel strt2 = sprhppos<sprite2>();
    bool armed1 = GET_BIT(armed, sprite1);
    bool armed2 = GET_BIT(armed, sprite2);
    Pixel strt = 0;
    
    // Iterate over all recorded register changes
    if (!sprChanges[pair].isEmpty()) {
        
        isize begin = sprChanges[pair].begin();
        isize end = sprChanges[pair].end();
        
        for (isize i = begin; i != end; i = sprChanges[pair].next(i)) {
            
            Cycle trigger = sprChanges[pair].keys[i];
            RegChange &change = sprChanges[pair].elements[i];
            
            // Draw a chunk of pixels
            drawSpritePair<pair>(strt, trigger, strt1, strt2, armed1, armed2);
            strt = trigger;
            
            // Apply the recorded register change
            switch (change.addr) {
                    
                case SET_SPR0DATA + sprite1:
                    sprdata[sprite1] = change.value;
                    SET_BIT(armed, sprite1);
                    armed1 = true;
                    break;
                    
                case SET_SPR0DATA + sprite2:
                    sprdata[sprite2] = change.value;
                    SET_BIT(armed, sprite2);
                    armed2 = true;
                    break;
                    
                case SET_SPR0DATB + sprite1:
                    sprdatb[sprite1] = change.value;
                    break;
                    
                case SET_SPR0DATB + sprite2:
                    sprdatb[sprite2] = change.value;
                    break;
                                        
                case SET_SPR0POS + sprite1:
                    sprpos[sprite1] = change.value;
                    strt1 = sprhppos<sprite1>();
                    break;
                    
                case SET_SPR0POS + sprite2:
                    sprpos[sprite2] = change.value;
                    strt2 = sprhppos<sprite2>();
                    break;
                    
                case SET_SPR0CTL + sprite1:
                    sprctl[sprite1] = change.value;
                    strt1 = sprhppos<sprite1>();
                    CLR_BIT(armed, sprite1);
                    armed1 = false;
                    break;
                    
                case SET_SPR0CTL + sprite2:
                    sprctl[sprite2] = change.value;
                    strt2 = sprhppos<sprite2>();
                    CLR_BIT(armed, sprite2);
                    armed2 = false;
                    break;

                default:
                    assert(false);
            }
        }
    }
    
    // Draw until the end of the line
    drawSpritePair<pair>(strt, sizeof(mBuffer) - 1,
                         strt1, strt2,
                         armed1, armed2);
    
    sprChanges[pair].clear();
}

template <isize pair> void
Denise::replaySpriteRegChanges()
{
    assert(pair < 4);
    
    const isize sprite1 = 2 * pair;
    const isize sprite2 = 2 * pair + 1;
    
    isize begin = sprChanges[pair].begin();
    isize end = sprChanges[pair].end();
    
    for (isize i = begin; i != end; i = sprChanges[pair].next(i)) {
        
        RegChange &change = sprChanges[pair].elements[i];
        
        // Apply the recorded register change
        switch (change.addr) {
                
            case SET_SPR0DATA + sprite1:
                sprdata[sprite1] = change.value;
                break;
                
            case SET_SPR0DATA + sprite2:
                sprdata[sprite2] = change.value;
                break;
                
            case SET_SPR0DATB + sprite1:
                sprdatb[sprite1] = change.value;
                break;
                
            case SET_SPR0DATB + sprite2:
                sprdatb[sprite2] = change.value;
                break;
                
            case SET_SPR0POS + sprite1:
                sprpos[sprite1] = change.value;
                break;
                
            case SET_SPR0POS + sprite2:
                sprpos[sprite2] = change.value;
                break;
                
            case SET_SPR0CTL + sprite1:
                sprctl[sprite1] = change.value;
                break;
                
            case SET_SPR0CTL + sprite2:
                sprctl[sprite2] = change.value;
                break;
                
            default:
                assert(false);
        }
    }
    
    sprChanges[pair].clear();
}

template <isize pair> void
Denise::drawSpritePair(Pixel hstrt, Pixel hstop, Pixel strt1, Pixel strt2,
                       bool armed1, bool armed2)
{
    assert(pair < 4);
    
    // Only proceeed if we are outside the VBLANK area
    if (agnus.pos.v < 26) return;
    
    const isize sprite1 = 2 * pair;
    const isize sprite2 = 2 * pair + 1;

    assert(hstrt <= isizeof(mBuffer));
    assert(hstop <= isizeof(mBuffer));

    assert(armed1 == !!GET_BIT(armed, sprite1));
    assert(armed2 == !!GET_BIT(armed, sprite2));

    bool attached = GET_BIT(sprctl[sprite2], 7);

    for (Pixel hpos = hstrt; hpos < hstop; hpos += 2) {

        if (hpos == strt1 && armed1) {
            ssra[sprite1] = sprdata[sprite1];
            ssrb[sprite1] = sprdatb[sprite1];
        }
        if (hpos == strt2 && armed2) {
            ssra[sprite2] = sprdata[sprite2];
            ssrb[sprite2] = sprdatb[sprite2];
        }

        if (ssra[sprite1] | ssrb[sprite1] | ssra[sprite2] | ssrb[sprite2]) {
            
            if (hpos >= spriteClipBegin && hpos < spriteClipEnd) {
                                
                if (attached) {
                    drawAttachedSpritePixelPair<sprite2>(hpos);
                } else {
                    drawSpritePixel<sprite1>(hpos);
                    drawSpritePixel<sprite2>(hpos);
                }
            }
            
            ssra[sprite1] = (u16)(ssra[sprite1] << 1);
            ssrb[sprite1] = (u16)(ssrb[sprite1] << 1);
            ssra[sprite2] = (u16)(ssra[sprite2] << 1);
            ssrb[sprite2] = (u16)(ssrb[sprite2] << 1);
        }
    }

    // Perform collision checks (if enabled)
    if (config.clxSprSpr) {
        checkS2SCollisions<2 * pair>(strt1, strt1 + 31);
        checkS2SCollisions<2 * pair + 1>(strt2, strt2 + 31);
    }
    if (config.clxSprPlf) {
        checkS2PCollisions<2 * pair>(strt1, strt1 + 31);
        checkS2PCollisions<2 * pair + 1>(strt2, strt2 + 31);
    }
}

template <isize x> void
Denise::drawSpritePixel(Pixel hpos)
{
    assert(hpos >= spriteClipBegin && hpos < spriteClipEnd);

    u8 a = (ssra[x] >> 15);
    u8 b = (ssrb[x] >> 14) & 2;
    u8 col = a | b;

    if (col) {

        u16 z = Z_SP[x];
        int base = 16 + 2 * (x & 6);

        if (z > zBuffer[hpos]) mBuffer[hpos] = base | col;
        if (z > zBuffer[hpos + 1]) mBuffer[hpos + 1] = base | col;
        zBuffer[hpos] |= z;
        zBuffer[hpos + 1] |= z;
    }
}

template <isize x> void
Denise::drawAttachedSpritePixelPair(Pixel hpos)
{
    assert(IS_ODD(x));
    assert(hpos >= spriteClipBegin && hpos < spriteClipEnd);

    u8 a1 = !!GET_BIT(ssra[x-1], 15);
    u8 b1 = !!GET_BIT(ssrb[x-1], 15) << 1;
    u8 a2 = !!GET_BIT(ssra[x], 15) << 2;
    u8 b2 = !!GET_BIT(ssrb[x], 15) << 3;
    assert(a1 == ((ssra[x-1] >> 15)));
    assert(b1 == ((ssrb[x-1] >> 14) & 0b0010));
    assert(a2 == ((ssra[x] >> 13) & 0b0100));
    assert(b2 == ((ssrb[x] >> 12) & 0b1000));

    u8 col = a1 | b1 | a2 | b2;

    if (col) {

        u16 z = Z_SP[x];

        if (z > zBuffer[hpos]) {
            mBuffer[hpos] = 0b10000 | col;
            zBuffer[hpos] |= z;
        }
        if (z > zBuffer[hpos+1]) {
            mBuffer[hpos+1] = 0b10000 | col;
            zBuffer[hpos+1] |= z;
        }
    }
}

void
Denise::updateBorderColor()
{
    if (config.borderblank && ecsena() && BRDRBLNK()) {
        borderColor = 64; // Pure black
    } else {
        borderColor = 0;  // Background color
    }
    
    if (BORDER_DEBUG) {
        borderColor = 65; // Debug color
    }
}

void
Denise::drawBorder()
{
    // Check if the horizontal flipflop was set somewhere in this rasterline
    bool hFlopWasSet = agnus.diwHFlop || agnus.diwHFlopOn != -1;

    // Check if the whole line is blank (drawn in background color)
    bool lineIsBlank = !agnus.diwVFlop || !hFlopWasSet;

    if (lineIsBlank) {

        // Draw blank line
        for (Pixel i = 0; i <= LAST_PIXEL; i++) {
            bBuffer[i] = iBuffer[i] = mBuffer[i] = borderColor;
        }

    } else {

        // Draw left border
        if (!agnus.diwHFlop && agnus.diwHFlopOn != -1) {
            for (isize i = 0; i < 2 * agnus.diwHFlopOn; i++) {
                bBuffer[i] = iBuffer[i] = mBuffer[i] = borderColor;
            }
        }

        // Draw right border
        if (agnus.diwHFlopOff != -1) {
            for (isize i = 2 * agnus.diwHFlopOff; i <= LAST_PIXEL; i++) {
                bBuffer[i] = iBuffer[i] = mBuffer[i] = borderColor;
            }
        }
    }

#ifdef LINE_DEBUG
    if (LINE_DEBUG) {
        for (Pixel i = 0; i <= LAST_PIXEL / 2; i++) {
            iBuffer[i] = mBuffer[i] = 64;
        }
    }
#endif
}

template <int x> void
Denise::checkS2SCollisions(Pixel start, Pixel end)
{
    // For the odd sprites, only proceed if collision detection is enabled
    if (IS_ODD(x) && !GET_BIT(clxcon, 12 + (x/2))) return;

    // Set up the sprite comparison masks
    u16 comp01 = Z_SP0 | (GET_BIT(clxcon, 12) ? Z_SP1 : 0);
    u16 comp23 = Z_SP2 | (GET_BIT(clxcon, 13) ? Z_SP3 : 0);
    u16 comp45 = Z_SP4 | (GET_BIT(clxcon, 14) ? Z_SP5 : 0);
    u16 comp67 = Z_SP6 | (GET_BIT(clxcon, 15) ? Z_SP7 : 0);

    // Iterate over all sprite pixels
    for (Pixel pos = end; pos >= start; pos -= 2) {

        u16 z = zBuffer[pos];
        
        // Skip if there are no other sprites at this pixel coordinate
        if (!(z & (Z_SP01234567 ^ Z_SP[x]))) continue;

        // Skip if the sprite is transparent at this pixel coordinate
        if (!(z & Z_SP[x])) continue;

        // Set sprite collision bits
        if ((z & comp45) && (z & comp67)) SET_BIT(clxdat, 14);
        if ((z & comp23) && (z & comp67)) SET_BIT(clxdat, 13);
        if ((z & comp23) && (z & comp45)) SET_BIT(clxdat, 12);
        if ((z & comp01) && (z & comp67)) SET_BIT(clxdat, 11);
        if ((z & comp01) && (z & comp45)) SET_BIT(clxdat, 10);
        if ((z & comp01) && (z & comp23)) SET_BIT(clxdat, 9);
        
        if (CLX_DEBUG) {
            
            trace((z & comp45) && (z & comp67), "Collision: 45 and 67\n");
            trace((z & comp45) && (z & comp67), "Collision: 45 and 67\n");
            trace((z & comp23) && (z & comp67), "Collision: 23 and 67\n");
            trace((z & comp23) && (z & comp45), "Collision: 23 and 45\n");
            trace((z & comp01) && (z & comp67), "Collision: 01 and 67\n");
            trace((z & comp01) && (z & comp45), "Collision: 01 and 45\n");
            trace((z & comp01) && (z & comp23), "Collision: 01 and 23\n");
        }
    }
}

template <int x> void
Denise::checkS2PCollisions(Pixel start, Pixel end)
{
    // For the odd sprites, only proceed if collision detection is enabled
    if (IS_ODD(x) && !getENSP<x>()) return;
    
    u8 enabled1 = getENBP1();
    u8 enabled2 = getENBP2();
    u8 compare1 = getMVBP1() & enabled1;
    u8 compare2 = getMVBP2() & enabled2;

    // Check for sprite-playfield collisions
    for (Pixel pos = end; pos >= start; pos -= 2) {

        u16 z = zBuffer[pos];

        // Skip if the sprite is transparent at this pixel coordinate
        if (!(z & Z_SP[x])) continue;

        // debug(CLX_DEBUG, "<%d> b[%d] = %X e1 = %X e2 = %X, c1 = %X c2 = %X\n",
        //     x, pos, bBuffer[pos], enabled1, enabled2, compare1, compare2);

        // Check for a collision with playfield 2
        if ((bBuffer[pos] & enabled2) == compare2) {
            trace(CLX_DEBUG, "S%d collides with PF2\n", x);
            SET_BIT(clxdat, 5 + (x / 2));

        } else {
            // There is a hardware oddity in single-playfield mode. If PF2
            // doesn't match, PF1 doesn't match either. No matter what.
            // See http://eab.abime.net/showpost.php?p=965074&postcount=2
            if (!(zBuffer[pos] & Z_DPF)) continue;
        }

        // Check for a collision with playfield 1
        if ((bBuffer[pos] & enabled1) == compare1) {
            trace(CLX_DEBUG, "S%d collides with PF1\n", x);
            SET_BIT(clxdat, 1 + (x / 2));
        }
    }
}

void
Denise::checkP2PCollisions()
{
    // Quick-exit if the collision bit already set
    if (GET_BIT(clxdat, 0)) return;

    // Set up comparison masks
    u8 enabled1 = getENBP1();
    u8 enabled2 = getENBP2();
    u8 compare1 = getMVBP1() & enabled1;
    u8 compare2 = getMVBP2() & enabled2;

    // Check all pixels one by one
    for (isize pos = 0; pos < HPIXELS; pos++) {

        u16 b = bBuffer[pos];

        // Check if there is a hit with playfield 1
        if ((b & enabled1) != compare1) continue;

        // Check if there is a hit with playfield 2
        if ((b & enabled2) != compare2) continue;

        // Set collision bit
        SET_BIT(clxdat, 0);
        return;
    }
}

void
Denise::vsyncHandler()
{
    pixelEngine.beginOfFrame();
    
    if (amiga.inDebugMode()) {
        
        for (isize i = 0; i < 8; i++) {
            latchedSpriteInfo[i] = spriteInfo[i];
            spriteInfo[i].height = 0;
            spriteInfo[i].vstrt  = 0;
            spriteInfo[i].vstop  = 0;
            spriteInfo[i].hstrt  = 0;
            spriteInfo[i].attach = false;
        }
        memcpy(latchedSpriteData, spriteData, sizeof(spriteData));
    }
}

void
Denise::beginOfLine(int vpos)
{
    // Reset the register change recorders
    conChanges.clear();
    pixelEngine.colChanges.clear();
    
    // Save the current values of various Denise registers
    initialBplcon0 = bplcon0;
    initialBplcon1 = bplcon1;
    initialBplcon2 = bplcon2;
    wasArmed = armed;

    // Prepare the bitplane shift registers
    for (isize i = 0; i < 6; i++) shiftReg[i] = 0;

    // Clear the bBuffer
    memset(bBuffer, 0, sizeof(bBuffer));

    // Reset the sprite clipping range
    spriteClipBegin = HPIXELS;
    spriteClipEnd = HPIXELS;
}

void
Denise::endOfLine(int vpos)
{
    // debug("endOfLine pixel = %d HPIXELS = %d\n", pixel, HPIXELS);

    // Check if we are below the VBLANK area
    if (vpos >= 26) {

        // Translate bitplane data to color register indices
        translate();

        // Draw sprites
        drawSprites();

        // Perform playfield-playfield collision check (if enabled)
        if (config.clxPlfPlf) checkP2PCollisions();

        // Draw border pixels
        drawBorder();

        // Synthesize RGBA values and write the result into the frame buffer
        pixelEngine.colorize(vpos);

        // Remove certain graphics layers if requested
        if (config.hiddenLayers) {
            pixelEngine.hide(vpos, config.hiddenLayers, config.hiddenLayerAlpha);
        }
    } else {
        
        drawSprites();
        pixelEngine.endOfVBlankLine();
    }

    assert(sprChanges[0].isEmpty());
    assert(sprChanges[1].isEmpty());
    assert(sprChanges[2].isEmpty());
    assert(sprChanges[3].isEmpty());

    // Invoke the DMA debugger
    dmaDebugger.computeOverlay();
    
    // Encode a HIRES / LORES marker in the first HBLANK pixel
    *denise.pixelEngine.pixelAddr(HBLANK_MIN * 4) = hires() ? 0 : -1;
}

void
Denise::recordSpriteData(isize nr)
{
    assert(nr < 8);

    u16 line = spriteInfo[nr].height;

    // Record data registers
    spriteData[nr][line] = HI_W_LO_W(sprdatb[nr], sprdata[nr]);

    // Record additional information in sprite line 0
    if (line == 0) {
        
        spriteInfo[nr].hstrt = ((sprpos[nr] & 0xFF) << 1) | (sprctl[nr] & 0x01);
        spriteInfo[nr].vstrt = agnus.sprVStrt[nr];
        spriteInfo[nr].vstop = agnus.sprVStop[nr];
        spriteInfo[nr].attach = IS_ODD(nr) ? GET_BIT(sprctl[nr], 7) : 0;
        
        for (isize i = 0; i < 16; i++) {
            spriteInfo[nr].colors[i] = pixelEngine.getColor(i + 16);
        }
    }
    
    spriteInfo[nr].height = (line + 1) % VPOS_CNT;
}

void
Denise::dumpBuffer(const u8 *buffer, isize length) const
{
    const isize cols = 16;

    for (isize i = 0; i < (length + cols - 1) / cols; i++) {
        for (isize j = 0; j < cols; j++) msg("%2d ", buffer[i * cols + j]);
        msg("\n");
    }
}

template void Denise::drawOdd<false>(Pixel offset);
template void Denise::drawOdd<true>(Pixel offset);
template void Denise::drawEven<false>(Pixel offset);
template void Denise::drawEven<true>(Pixel offset);

template void Denise::translateDPF<true>(Pixel from, Pixel to, PFState &state);
template void Denise::translateDPF<false>(Pixel from, Pixel to, PFState &state);
