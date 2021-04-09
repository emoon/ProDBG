// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "BlitterTypes.h"
#include "AmigaComponent.h"
#include "Memory.h"

/* The Blitter supports three accuracy levels:
 *
 * Level 0: Moves data in a single chunk.
 *          Terminates immediately without using up any bus cycles.
 *
 * Level 1: Moves data in a single chunk.
 *          Uses up bus cycles like the real Blitter does.
 *
 * Level 2: Moves data word by word like the real Blitter does.
 *          Uses up bus cycles like the real Blitter does.
 *
 * Level 0 and 1 invoke the FastBlitter. Level 2 invokes the SlowBlitter.
 */

class Blitter : public AmigaComponent
{
    friend class Agnus;

    // Current configuration
    BlitterConfig config;

    // Result of the latest inspection
    BlitterInfo info;

    // The fill pattern lookup tables
    u8 fillPattern[2][2][256];     // [inclusive/exclusive][carry in][data]
    u8 nextCarryIn[2][256];        // [carry in][data]


    //
    // Blitter registers
    //
    
    // Control registers
    u16 bltcon0;
    u16 bltcon1;
    
    // DMA pointers
    u32 bltapt;
    u32 bltbpt;
    u32 bltcpt;
    u32 bltdpt;
    
    // Word masks
    u16 bltafwm;
    u16 bltalwm;
    
    // Size register
    u16 bltsizeH;
    u16 bltsizeV;

    // Modulo registers
    i16 bltamod;
    i16 bltbmod;
    i16 bltcmod;
    i16 bltdmod;
    
    // Pipeline registers
    u16 anew;
    u16 bnew;
    u16 aold;
    u16 bold;
    u16 ahold;
    u16 bhold;
    u16 chold;
    u16 dhold;
    u32 ashift;
    u32 bshift;

    
    //
    // Fast Blitter
    //

    // The Fast Blitter's blit functions
    void (Blitter::*blitfunc[32])(void);


    //
    // Slow Blitter
    //

    // Micro-programs for copy blits
    void (Blitter::*copyBlitInstr[16][2][2][6])(void);

    // Micro-program for line blits
    void (Blitter::*lineBlitInstr[6])(void);

    // The program counter indexing the micro instruction to execute
    u16 bltpc;

    // Blitter state
    int iteration;
    
    // Counters tracking the coordinate of the blit window
    u16 xCounter;
    u16 yCounter;

    // Counters tracking the DMA accesses for each channel
    i16 cntA;
    i16 cntB;
    i16 cntC;
    i16 cntD;

    // The fill carry bit
    bool fillCarry;

    // Channel A mask
    u16 mask;

    // If true, the D register won't be written to memory
    bool lockD;


    //
    // Flags
    //

    /* Indicates if the Blitter is currently running. The flag is set to true
     * when a Blitter operation starts and set to false when the operation ends.
     */
    bool running;

    /* The Blitter busy flag. This flag shows up in DMACON and has a similar
     * meaning as variable 'running'. The only difference is that the busy flag
     * is cleared a few cycles before the Blitter actually terminates.
     */
    bool bbusy;

    // The Blitter zero flag
    bool bzero;

    // Indicates whether the Blitter interrupt has been triggered
    bool birq;
    
    
    //
    // Counters
    //

private:

    // Counter for tracking the remaining words to process
    int remaining;

    // Debug counters
    int copycount;
    int linecount;

    // Debug checksums
    u32 check1;
    u32 check2;

public:
    
    // Experimental
    u8 memguard[KB(512)] = {};
    
 
    //
    // Initializing
    //
    
public:
    
    Blitter(Amiga& ref);

    const char *getDescription() const override { return "Blitter"; }
    
    void initFastBlitter();
    void initSlowBlitter();

    void _initialize() override;
    void _reset(bool hard) override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
        worker

        << config.accuracy;
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        << bltcon0
        << bltcon1

        << bltapt
        << bltbpt
        << bltcpt
        << bltdpt

        << bltafwm
        << bltalwm

        << bltsizeH
        << bltsizeV

        << bltamod
        << bltbmod
        << bltcmod
        << bltdmod

        << anew
        << bnew
        << aold
        << bold
        << ahold
        << bhold
        << chold
        << dhold
        << ashift
        << bshift

        << bltpc
        << iteration
        
        << xCounter
        << yCounter
        << cntA
        << cntB
        << cntC
        << cntD

        << fillCarry
        << mask
        << lockD

        << running
        << bbusy
        << bzero
        << birq

        << remaining;
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }

    
    //
    // Configuring
    //

public:
    
    const BlitterConfig &getConfig() const { return config; }
    
    long getConfigItem(Option option) const;
    bool setConfigItem(Option option, long value) override;
    
    
    //
    // Analyzing
    //
    
public:
    
    BlitterInfo getInfo() { return HardwareComponent::getInfo(info); }

private:
    
    // Methods from HardwareComponent
    void _inspect() override;
    void _dump(Dump::Category category, std::ostream& os) const override;


    //
    // Accessing
    //

public:
    
    // Returns true if the Blitter is processing a blit
    bool isActive() const { return running; }

    // Returns the value of the Blitter Busy Flag
    bool isBusy() const { return bbusy; }

    // Returns the value of the zero flag
    bool isZero() const { return bzero; }

    // BLTCON0
    void pokeBLTCON0(u16 value);
    void setBLTCON0(u16 value);
    void pokeBLTCON0L(u16 value);
    void setBLTCON0L(u16 value);
    void setBLTCON0ASH(u16 ash);

    u16 bltconASH()   const { return bltcon0 >> 12; }
    u16 bltconLF()    const { return bltcon0 & 0xF; }
    u16 bltconUSE()   const { return (bltcon0 >> 8) & 0xF; }
    bool bltconUSEA() const { return bltcon0 & (1 << 11); }
    bool bltconUSEB() const { return bltcon0 & (1 << 10); }
    bool bltconUSEC() const { return bltcon0 & (1 << 9); }
    bool bltconUSED() const { return bltcon0 & (1 << 8); }

    // BLTCON1
    void pokeBLTCON1(u16 value);
    void setBLTCON1(u16 value);
    void setBLTCON1BSH(u16 bsh);

    u16 bltconBSH()   const { return bltcon1 >> 12; }
    bool bltconEFE()  const { return bltcon1 & (1 << 4); }
    bool bltconIFE()  const { return bltcon1 & (1 << 3); }
    bool bltconFE()   const { return bltconEFE() || bltconIFE(); }
    bool bltconFCI()  const { return bltcon1 & (1 << 2); }
    bool bltconDESC() const { return bltcon1 & (1 << 1); }
    bool bltconLINE() const { return bltcon1 & (1 << 0); }

    // BLTAxWM
    void pokeBLTAFWM(u16 value);
    void pokeBLTALWM(u16 value);
    
    
    // BLTxPTx
    void pokeBLTAPTH(u16 value);
    void pokeBLTAPTL(u16 value);
    void pokeBLTBPTH(u16 value);
    void pokeBLTBPTL(u16 value);
    void pokeBLTCPTH(u16 value);
    void pokeBLTCPTL(u16 value);
    void pokeBLTDPTH(u16 value);
    void pokeBLTDPTL(u16 value);
    
    // BLITSIZE
    template <Accessor s> void pokeBLTSIZE(u16 value);
    void setBLTSIZE(u16 value);
    void pokeBLTSIZV(u16 value);
    void setBLTSIZV(u16 value);
    void pokeBLTSIZH(u16 value);

    // BLTxMOD
    void pokeBLTAMOD(u16 value);
    void pokeBLTBMOD(u16 value);
    void pokeBLTCMOD(u16 value);
    void pokeBLTDMOD(u16 value);
    
    // BLTxDAT
    void pokeBLTADAT(u16 value);
    void pokeBLTBDAT(u16 value);
    void pokeBLTCDAT(u16 value);
    
    
    //
    // Handling requests of other components
    //

public:
    
    // Called by Agnus when DMACON is written to
    void pokeDMACON(u16 oldValue, u16 newValue);


    //
    // Serving events
    //
    
public:
    
    // Processes a Blitter event
    void serviceEvent();


    //
    // Running the sub-units
    //

private:
    
    // Runs the barrel shifters on data paths A and B
    void doBarrelA    (u16 aNew, u16 *aOld, u16 *aHold) const;
    void doBarrelAdesc(u16 aNew, u16 *aOld, u16 *aHold) const;
    void doBarrelB    (u16 bNew, u16 *bOld, u16 *bHold) const;
    void doBarrelBdesc(u16 bNew, u16 *bOld, u16 *bHold) const;

    // Emulates the minterm logic circuit
    u16 doMintermLogic     (u16 a, u16 b, u16 c, u8 minterm) const;
    u16 doMintermLogicQuick(u16 a, u16 b, u16 c, u8 minterm) const;

    // Emulates the fill logic circuit
    void doFill(u16 &data, bool &carry);


    //
    // Executing the Blitter
    //

private:

    // Prepares for a new Blitter operation (called in state BLT_STRT1)
    void prepareBlit();

    // Starts a Blitter operation
    void beginBlit();
    void beginLineBlit(int level);
    void beginCopyBlit(int level);

    // Clears the BBUSY flag and triggers the Blitter interrupt
    void signalEnd();

    // Concludes the current Blitter operation
    void endBlit();

    
    //
    //  Executing the Fast Blitter
    //

private:
    
    // Starts a level 0 blit
    void beginFastLineBlit();
    void beginFastCopyBlit();

    // Performs a copy blit operation via the FastBlitter
    template <bool useA, bool useB, bool useC, bool useD, bool desc>
    void doFastCopyBlit();
    
    // Performs a line blit operation via the FastBlitter
    void doFastLineBlit();


    //
    //  Executing the Slow Blitter
    //

private:
    
    // Starts a level 1 blit
    void beginFakeCopyBlit();
    void beginFakeLineBlit();

    // Starts a level 2 blit
    void beginSlowLineBlit();
    void beginSlowCopyBlit();

    // Emulates a Blitter micro-instruction
    template <u16 instr> void exec();
    template <u16 instr> void fakeExec();

    // Checks iterations
    bool isFirstWord() const { return xCounter == bltsizeH; }
    bool isLastWord() const { return xCounter == 1; }

    // Sets the x or y counter to a new value
    void setXCounter(u16 value);
    void setYCounter(u16 value);
    void resetXCounter() { setXCounter(bltsizeH); }
    void resetYCounter() { setYCounter(bltsizeV); }
    void decXCounter() { setXCounter(xCounter - 1); }
    void decYCounter() { setYCounter(yCounter - 1); }

    // Emulates the barrel shifter
    void doBarrelShifterA();
    void doBarrelShifterB();
};
