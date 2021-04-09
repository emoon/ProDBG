// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "config.h"
#include "Blitter.h"
#include "Agnus.h"
#include "Checksum.h"
#include "Memory.h"
#include "Paula.h"

void
Blitter::initFastBlitter()
{
    void (Blitter::*blitfunc[32])(void) = {
        &Blitter::doFastCopyBlit<0,0,0,0,0>, &Blitter::doFastCopyBlit<0,0,0,0,1>,
        &Blitter::doFastCopyBlit<0,0,0,1,0>, &Blitter::doFastCopyBlit<0,0,0,1,1>,
        &Blitter::doFastCopyBlit<0,0,1,0,0>, &Blitter::doFastCopyBlit<0,0,1,0,1>,
        &Blitter::doFastCopyBlit<0,0,1,1,0>, &Blitter::doFastCopyBlit<0,0,1,1,1>,
        &Blitter::doFastCopyBlit<0,1,0,0,0>, &Blitter::doFastCopyBlit<0,1,0,0,1>,
        &Blitter::doFastCopyBlit<0,1,0,1,0>, &Blitter::doFastCopyBlit<0,1,0,1,1>,
        &Blitter::doFastCopyBlit<0,1,1,0,0>, &Blitter::doFastCopyBlit<0,1,1,0,1>,
        &Blitter::doFastCopyBlit<0,1,1,1,0>, &Blitter::doFastCopyBlit<0,1,1,1,1>,
        &Blitter::doFastCopyBlit<1,0,0,0,0>, &Blitter::doFastCopyBlit<1,0,0,0,1>,
        &Blitter::doFastCopyBlit<1,0,0,1,0>, &Blitter::doFastCopyBlit<1,0,0,1,1>,
        &Blitter::doFastCopyBlit<1,0,1,0,0>, &Blitter::doFastCopyBlit<1,0,1,0,1>,
        &Blitter::doFastCopyBlit<1,0,1,1,0>, &Blitter::doFastCopyBlit<1,0,1,1,1>,
        &Blitter::doFastCopyBlit<1,1,0,0,0>, &Blitter::doFastCopyBlit<1,1,0,0,1>,
        &Blitter::doFastCopyBlit<1,1,0,1,0>, &Blitter::doFastCopyBlit<1,1,0,1,1>,
        &Blitter::doFastCopyBlit<1,1,1,0,0>, &Blitter::doFastCopyBlit<1,1,1,0,1>,
        &Blitter::doFastCopyBlit<1,1,1,1,0>, &Blitter::doFastCopyBlit<1,1,1,1,1>
    };

    assert(sizeof(this->blitfunc) == sizeof(blitfunc));
    memcpy(this->blitfunc, blitfunc, sizeof(blitfunc));
}

void
Blitter::beginFastLineBlit()
{
    // Only call this function in line blit mode
    assert(bltconLINE());

    // Run the fast line Blitter
    doFastLineBlit();

    // Terminate immediately
    signalEnd();
    paula.raiseIrq(INT_BLIT);
    endBlit();
}

void
Blitter::beginFastCopyBlit()
{
    // Only call this function in copy blit mode
    assert(!bltconLINE());

    // Run the fast copy Bliter
    int nr = ((bltcon0 >> 7) & 0b11110) | bltconDESC();
    (this->*blitfunc[nr])();

    // Terminate immediately
    signalEnd();
    paula.raiseIrq(INT_BLIT);
    endBlit();
}

template <bool useA, bool useB, bool useC, bool useD, bool desc>
void Blitter::doFastCopyBlit()
{
    u32 apt = bltapt;
    u32 bpt = bltbpt;
    u32 cpt = bltcpt;
    u32 dpt = bltdpt;

    bool fill = bltconFE();
    bool fillCarry;

    int incr = desc ? -2 : 2;
    [[maybe_unused]] int ash  = desc ? 16 - bltconASH() : bltconASH();
    [[maybe_unused]] int bsh  = desc ? 16 - bltconBSH() : bltconBSH();
    i32 amod = desc ? -bltamod : bltamod;
    i32 bmod = desc ? -bltbmod : bltbmod;
    i32 cmod = desc ? -bltcmod : bltcmod;
    i32 dmod = desc ? -bltdmod : bltdmod;

    aold = 0;
    bold = 0;

    for (isize y = 0; y < bltsizeV; y++) {

        // Reset the fill carry bit
        fillCarry = !!bltconFCI();

        // Apply the "first word mask" in the first iteration
        u16 mask = bltafwm;

        for (isize x = 0; x < bltsizeH; x++) {

            // Apply the "last word mask" in the last iteration
            if (x == bltsizeH - 1) mask &= bltalwm;

            // Fetch A
            if (useA) {
                anew = mem.peek16 <ACCESSOR_AGNUS> (apt);
                trace(BLT_DEBUG, "    A = peek(%X) = %X\n", apt, anew);
                apt = U32_ADD(apt, incr);
            }

            // Fetch B
            if (useB) {
                bnew = mem.peek16 <ACCESSOR_AGNUS> (bpt);
                trace(BLT_DEBUG, "    B = peek(%X) = %X\n", bpt, bnew);
                bpt = U32_ADD(bpt, incr);
            }

            // Fetch C
            if (useC) {
                chold = mem.peek16 <ACCESSOR_AGNUS> (cpt);
                trace(BLT_DEBUG, "    C = peek(%X) = %X\n", cpt, chold);
                cpt = U32_ADD(cpt, incr);
            }
            
            trace(BLT_DEBUG, "    After fetch: A = %x B = %x C = %x\n",
                  anew, bnew, chold);
            trace(BLT_DEBUG, "    After masking with %x (%x,%x) %x\n",
                  mask, bltafwm, bltalwm, anew & mask);
            trace(BLT_DEBUG, "    ash = %d bsh = %d mask = %X\n",
                  bltconASH(), bltconBSH(), mask);

            // Run the barrel shifter on path A (even if A channel is disabled)
            if (desc) {
                doBarrelAdesc(anew & mask, &aold, &ahold);
            } else {
                doBarrelA(anew & mask, &aold, &ahold);
            }
            
            // Run the barrel shifter on path B (if B channel enabled)
            if (useB) {
                if (desc) {
                    doBarrelBdesc(bnew, &bold, &bhold);
                } else {
                    doBarrelB(bnew, &bold, &bhold);
                }
            }

            trace(BLT_DEBUG, "    After shifting (%d,%d) A = %x B = %x\n", ash, bsh, ahold, bhold);
            
            // Run the minterm logic circuit
            trace(BLT_DEBUG, "    Minterms: ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
            dhold = doMintermLogicQuick(ahold, bhold, chold, bltcon0 & 0xFF);
            if (BLT_DEBUG) {
                assert(dhold == doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF));
            }

            // Run the fill logic circuit
            if (fill) doFill(dhold, fillCarry);

            // Update the zero flag
            if (dhold) bzero = false;

            // Write D
            if (useD) {
                mem.poke16 <ACCESSOR_AGNUS> (dpt, dhold);

                if (BLT_CHECKSUM) {
                    check1 = util::fnv_1a_it32(check1, dhold);
                    check2 = util::fnv_1a_it32(check2, dpt & agnus.ptrMask);
                }
                trace(BLT_DEBUG, "D: poke(%X), %X  (check: %X %X)\n", dpt, dhold, check1, check2);
                dpt = U32_ADD(dpt, incr);
            }

            // Clear the word mask
            mask = 0xFFFF;
        }

        // Add modulo values
        if (useA) apt = U32_ADD(apt, amod);
        if (useB) bpt = U32_ADD(bpt, bmod);
        if (useC) cpt = U32_ADD(cpt, cmod);
        if (useD) dpt = U32_ADD(dpt, dmod);
    }

    // Write back pointer registers
    bltapt = apt;
    bltbpt = bpt;
    bltcpt = cpt;
    bltdpt = dpt;
}

#define blitterLineIncreaseX(a_shift, cpt) \
if (a_shift < 15) a_shift++; \
else \
{ \
a_shift = 0; \
cpt += 2; \
}

#define blitterLineDecreaseX(a_shift, cpt) \
{ \
if (a_shift == 0) \
{ \
a_shift = 16; \
cpt -= 2; \
} \
a_shift--; \
}

#define blitterLineIncreaseY(cpt, cmod) cpt += cmod;
#define blitterLineDecreaseY(cpt, cmod) cpt -= cmod;

void
Blitter::doFastLineBlit()
{
    bltapt &= agnus.ptrMask;
    bltcpt &= agnus.ptrMask;
    bltdpt &= agnus.ptrMask;

    //
    // Adapted from WinFellow
    //

    u32 bltcon = HI_W_LO_W(bltcon0, bltcon1);
    
    int height = bltsizeV;
    
    u16 bltadat_local = 0;
    u16 bltbdat_local = 0;
    u16 bltcdat_local = chold;
    u16 bltddat_local = 0;
    u16 mask = (u16)((bnew >> bltconBSH()) | (bnew << (16 - bltconBSH())));
    bool a_enabled = bltcon & 0x08000000;
    bool c_enabled = bltcon & 0x02000000;
    
    bool decision_is_signed = (((bltcon >> 6) & 1) == 1);
    u32 decision_variable = bltapt;
    
    // Quirk: Set decision increases to 0 if a is disabled, ensures bltapt remains unchanged
    i16 decision_inc_signed = a_enabled ? bltbmod : 0;
    i16 decision_inc_unsigned = a_enabled ? bltamod : 0;
    
    u32 bltcpt_local = bltcpt;
    u32 bltdpt_local = bltdpt;
    u32 blit_a_shift_local = bltconASH();
    u32 bzero_local = 0;
    
    u32 sulsudaul = (bltcon >> 2) & 0x7;
    bool x_independent = (sulsudaul & 4);
    bool x_inc = ((!x_independent) && !(sulsudaul & 2)) || (x_independent && !(sulsudaul & 1));
    bool y_inc = ((!x_independent) && !(sulsudaul & 1)) || (x_independent && !(sulsudaul & 2));
    bool single_dot = false;
    u8 minterm = (u8)(bltcon >> 16);
    
    for (isize i = 0; i < height; ++i)
    {
        // Read C-data from memory if the C-channel is enabled
        if (c_enabled) {
            bltcdat_local = mem.peek16 <ACCESSOR_AGNUS> (bltcpt_local);
        }
        
        // Calculate data for the A-channel
        bltadat_local = (anew & bltafwm) >> blit_a_shift_local;
        
        // Check for single dot
        if (x_independent) {
            if (bltcon & 0x00000002) {
                if (single_dot) {
                    bltadat_local = 0;
                } else {
                    single_dot = true;
                }
            }
        }
        
        // Calculate data for the B-channel
        bltbdat_local = (mask & 1) ? 0xFFFF : 0;
        
        // Calculate result
        bltddat_local = doMintermLogicQuick(bltadat_local, bltbdat_local, bltcdat_local, minterm);
        
        // Save result to D-channel, same as the C ptr after first pixel.
        if (c_enabled) { // C-channel must be enabled
            mem.poke16 <ACCESSOR_AGNUS> (bltdpt_local, bltddat_local);

            if (BLT_CHECKSUM) {
                check1 = util::fnv_1a_it32(check1, bltddat_local);
                check2 = util::fnv_1a_it32(check2, bltdpt_local & agnus.ptrMask);
            }
        }
        
        // Remember zero result status
        bzero_local = bzero_local | bltddat_local;
        
        // Rotate mask
        mask = (u16)(mask << 1 | mask >> 15);
        
        // Test movement in the X direction
        // When the decision variable gets positive,
        // the line moves one pixel to the right
        
        // decrease/increase x
        if (decision_is_signed) {
            // Do not yet increase, D has sign
            // D = D + (2*sdelta = bltbmod)
            decision_variable = (u32)((i64)decision_variable + decision_inc_signed);
            // decision_variable += decision_inc_signed;
        } else {
            // increase, D reached a positive value
            // D = D + (2*sdelta - 2*ldelta = bltamod)
            decision_variable = (u32)((i64)decision_variable + decision_inc_unsigned);
            // decision_variable += decision_inc_unsigned;
            
            if (!x_independent) {
                if (x_inc) {
                    blitterLineIncreaseX(blit_a_shift_local, bltcpt_local);
                } else {
                    blitterLineDecreaseX(blit_a_shift_local, bltcpt_local);
                }
            } else {
                if (y_inc) {
                    blitterLineIncreaseY(bltcpt_local, bltcmod);
                } else {
                    blitterLineDecreaseY(bltcpt_local, bltcmod);
                }
                single_dot = false;
            }
        }
        decision_is_signed = ((i16)decision_variable < 0);
        
        if (!x_independent)
        {
            // decrease/increase y
            if (y_inc) {
                blitterLineIncreaseY(bltcpt_local, bltcmod);
            } else {
                blitterLineDecreaseY(bltcpt_local, bltcmod);
            }
        }
        else
        {
            if (x_inc) {
                blitterLineIncreaseX(blit_a_shift_local, bltcpt_local);
            } else {
                blitterLineDecreaseX(blit_a_shift_local, bltcpt_local);
            }
        }
        bltdpt_local = bltcpt_local;
    }
    bltcon = bltcon & 0x0FFFFFFBF;
    if (decision_is_signed) bltcon |= 0x00000040;
    
    setBLTCON0ASH(blit_a_shift_local);
    bnew   = bltbdat_local;
    
    bltapt = decision_variable & agnus.ptrMask;
    bltcpt = bltcpt_local & agnus.ptrMask;
    bltdpt = bltdpt_local & agnus.ptrMask;
    bzero  = bzero_local == 0;
}
