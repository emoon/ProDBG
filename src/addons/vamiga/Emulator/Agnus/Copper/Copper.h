// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _COPPER_H
#define _COPPER_H

#include "Beam.h"

class Copper : public AmigaComponent
{
    friend class Agnus;
    
    // Result of the latest inspection
    CopperInfo info;

    // The currently executed Copper list (1 or 2)
    u8 copList = 1;

    /* Indicates if the next instruction should be skipped. This flag is
     * usually false. It is set to true by the SKIP instruction if the skip
     * condition holds.
     */
    bool skip = false;
     
    // The Copper list location pointers
    u32 cop1lc;
    u32 cop2lc;

    /* Address of the first and last executed instruction in each Copper list
     * These values are needed by the debugger to determine the end of the
     * Copper lists. Note that these values cannot be computed directly. They
     * are computed by observing the program counter.
     */
    u32 cop1end;
    u32 cop2end;

    // The Copper Danger bit (CDANG)
    bool cdang;
        
    // The Copper instruction registers
    u16 cop1ins = 0;
    u16 cop2ins = 0;

    // The Copper program counter
    u32 coppc = 0;

    /* Indicates whether the Copper has been active since the last vertical
     * sync. The value of this variable is used to determine if a write to the
     * location registers will be pushed through the Copper's program counter.
     */
    bool activeInThisFrame;
   
    // Storage for disassembled instruction
    char disassembly[128];

public:

    // Indicates if Copper is currently servicing an event (for debugging only)
    bool servicing = false;
    
    // Temporary debug flag
    bool verbose = false;


    //
    // Debugging
    //

private:

    u64 checkcnt = 0;
    u32 checksum = fnv_1a_init32();


    //
    // Initializing
    //
    
public:
    
    Copper(Amiga& ref);

    void _reset(bool hard) override;

    
    //
    // Analyzing
    //

public:
    
    // Returns the result of the latest inspection
    CopperInfo getInfo() { return HardwareComponent::getInfo(info); }

private:

    void _inspect() override;
    void _dump() override;

    
    //
    // Serialization
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
        worker

        & copList
        & skip
        & cop1lc
        & cop2lc
        & cop1end
        & cop2end
        & cdang
        & cop1ins
        & cop2ins
        & coppc
        & activeInThisFrame;
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }


    //
    // Accessing
    //

public:
    
    u32 getCopPC() const { return coppc; }
    
    void pokeCOPCON(u16 value);
    template <Accessor s> void pokeCOPJMP1();
    template <Accessor s> void pokeCOPJMP2();
    void pokeCOPINS(u16 value);
    void pokeCOP1LCH(u16 value);
    void pokeCOP1LCL(u16 value);
    void pokeCOP2LCH(u16 value);
    void pokeCOP2LCL(u16 value);
    void pokeNOOP(u16 value);

    
    //
    // Running the device
    //
    
private:
 
    // Advances the program counter
    void advancePC();

    // Switches the Copper list
    void switchToCopperList(int nr);

    /* Searches for the next matching beam position. This function is called
     * when a WAIT statement is processed. It is uses to compute where the
     * Copper wakes up.
     *
     * Return values:
     *
     * true:  The Copper wakes up in the current frame.
     *        The wake-up position is returned in variable 'result'.
     * false: The Copper does not wake up the current frame.
     *        Variable 'result' remains untouched.
     */
    bool findMatch(Beam &result);
    bool findMatchNew(Beam &result);

    // Called by findMatch() to determine the vertical trigger position
    bool findVerticalMatch(i16 vStrt, i16 vComp, i16 vMask, i16 &result);

    // Called by findMatch() to determine the horizontal trigger position
    bool findHorizontalMatch(i16 hStrt, i16 hComp, i16 hMask, i16 &result);
    bool findHorizontalMatchNew(u32 &beam, u32 comp, u32 mask);

    // Emulates the Copper writing a value into one of the custom registers
    void move(u32 addr, u16 value);

    // Runs the comparator circuit
    bool comparator(Beam beam, u16 waitpos, u16 mask);
    bool comparator(Beam beam);
    bool comparator();

    // Emulates a WAIT command
    void scheduleWaitWakeup(bool bfd);


    //
    // Analyzing Copper instructions
    //
    
private:
    
    /*             MOVE              WAIT              SKIP
     * Bit   cop1ins cop2ins   cop1ins cop2ins   cop1ins cop2ins
     *  15      x     DW15       VP7     BFD       VP7     BFD
     *  14      x     DW14       VP6     VM6       VP6     VM6
     *  13      x     DW13       VP5     VM5       VP5     VM5
     *  12      x     DW12       VP4     VM4       VP4     VM4
     *  11      x     DW11       VP3     VM3       VP3     VM3
     *  10      x     DW10       VP2     VM2       VP2     VM2
     *   9      x     DW9        VP1     VM1       VP1     VM1
     *   8     RA8    DW8        VP0     VM0       VP0     VM0
     *   7     RA7    DW7        HP8     HM8       HP8     HM8
     *   6     RA6    DW6        HP7     HM7       HP7     HM7
     *   5     RA5    DW5        HP6     HM6       HP6     HM6
     *   4     RA4    DW4        HP5     HM5       HP5     HM5
     *   3     RA3    DW3        HP4     HM4       HP4     HM4
     *   2     RA2    DW2        HP3     HM3       HP3     HM3
     *   1     RA1    DW1        HP2     HM2       HP2     HM2
     *   0      0     DW0         1       0         1       1
     *
     * Each of the following functions exists in two variants. The first
     * variant analyzes the instruction in the instructions register. The
     * second variant analyzes the instruction at a certain location in memory.
     */
 
    bool isMoveCmd();
    bool isMoveCmd(u32 addr);
    
    bool isWaitCmd();
    bool isWaitCmd(u32 addr);

    bool isSkipCmd();
    bool isSkipCmd(u32 addr);
    
    u16 getRA();
    u16 getRA(u32 addr);

    u16 getDW();
    u16 getDW(u32 addr);

    bool getBFD();
    bool getBFD(u32 addr);

    u16 getVPHP();
    u16 getVPHP(u32 addr);
    u16 getVP() { return HI_BYTE(getVPHP()); }
    u16 getVP(u32 addr) { return HI_BYTE(getVPHP(addr)); }
    u16 getHP() { return LO_BYTE(getVPHP()); }
    u16 getHP(u32 addr) { return LO_BYTE(getVPHP(addr)); }
    
    u16 getVMHM();
    u16 getVMHM(u32 addr);
    u16 getVM() { return HI_BYTE(getVMHM()); }
    u16 getVM(u32 addr) { return HI_BYTE(getVMHM(addr)); }
    u16 getHM() { return LO_BYTE(getVMHM()); }
    u16 getHM(u32 addr) { return LO_BYTE(getVMHM(addr)); }
    
public:
    
    // Returns true if the Copper has no access to this custom register
    bool isIllegalAddress(u32 addr);
    
    // Returns true if the Copper instruction at addr is illegal
    bool isIllegalInstr(u32 addr);
    
 
    //
    // Managing events
    //
    
public:
    
    // Processes a Copper event
    void serviceEvent(EventID id);

    // Schedules the next Copper event
    void schedule(EventID next, int delay = 2);

    // Reschedules the current Copper event
    void reschedule(int delay = 1);

private:
    
    // Executed after each frame
    void vsyncHandler();
    

    //
    // Handling delegation calls
    //

public:

    void blitterDidTerminate();


    //
    // Debugging
    //
    
public:

    // Returns the number of instructions in Copper list 1 or 2
    int instrCount(int nr);

    // Manually lengthens or shortens the value returned by instrCount()
    void adjustInstrCount(int nr, int offset);

    // Disassembles a single Copper command
    char *disassemble(u32 addr);
    char *disassemble(unsigned list, u32 offset);

    // Dumps a Copper list
    void dumpCopperList(unsigned list, unsigned length); 
};

#endif 
