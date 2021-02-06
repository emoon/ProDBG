// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _CPU_H
#define _CPU_H

#include "AmigaComponent.h"
#include "Moira.h"

class CPU : public AmigaComponent, public moira::Moira {

    // Result of the latest inspection
    CPUInfo info;

    
    //
    // Initializing
    //

public:

    CPU(Amiga& ref);

    void _reset(bool hard) override;
    
    
    //
    // Analyzing
    //
    
public:
    
    CPUInfo getInfo() { return HardwareComponent::getInfo(info); }
    
private:
    
    void _inspect() override;
    void _inspect(u32 dasmStart);
    void _dump() override;

    
    //
    // Serializing
    //
    
private:
    
    template <class T>
    void applyToPersistentItems(T& worker)
    {
    }

    template <class T>
    void applyToHardResetItems(T& worker)
    {
        worker

        & flags
        & clock

        & reg.pc
        & reg.pc0
        & reg.sr.t
        & reg.sr.s
        & reg.sr.x
        & reg.sr.n
        & reg.sr.z
        & reg.sr.v
        & reg.sr.c
        & reg.sr.ipl
        & reg.r
        & reg.usp
        & reg.ssp
        & reg.ipl

        & queue.irc
        & queue.ird

        & ipl
        & fcl
        & exception;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    size_t _size() override { COMPUTE_SNAPSHOT_SIZE }
    size_t _load(u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    size_t _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    size_t didLoadFromBuffer(u8 *buffer) override;


    //
    // Controlling
    //
    
private:

    void _setDebug(bool enable) override;

        
    //
    // Talking to Moira
    //

private:

    void sync(int cycles) override;
    u8 read8(u32 addr) override;
    u16 read16(u32 addr) override;
    u16 read16OnReset(u32 addr) override;
    u16 read16Dasm(u32 addr) override;
    void write8 (u32 addr, u8  val) override;
    void write16 (u32 addr, u16 val) override;
    int readIrqUserVector(u8 level) override { return 0; }
 
    void signalReset() override;
    void signalStop(u16 op) override;
    void signalTAS() override;
    
    void signalHalt() override;
    
    void signalAddressError(moira::AEStackFrame &frame) override;
    void signalLineAException(u16 opcode) override;
    void signalLineFException(u16 opcode) override;
    void signalIllegalOpcodeException(u16 opcode) override;
    void signalTraceException() override;
    void signalTrapException() override;
    void signalPrivilegeViolation() override;
    void signalInterrupt(u8 level) override;
    
    void signalJumpToVector(int nr, u32 addr) override;
    
    void breakpointReached(u32 addr) override;
    void watchpointReached(u32 addr) override;

    
    //
    // Working with the clock
    //

public:

    // Returns the clock in CPU cycles
    CPUCycle getCpuClock() { return getClock(); }

    // Returns the clock in master cycles
    Cycle getMasterClock() { return CPU_CYCLES(getClock()); }

    // Delays the CPU by a certain amout of master cycles
    void addWaitStates(Cycle cycles) { clock += AS_CPU_CYCLES(cycles); }
    
    
    //
    // Running the disassembler
    //
    
    // Disassembles a recorded instruction from the log buffer
    const char *disassembleRecordedInstr(int i, long *len);
    const char *disassembleRecordedWords(int i, int len);
    const char *disassembleRecordedFlags(int i);
    const char *disassembleRecordedPC(int i);

    // Disassembles the instruction at the specified address
    const char *disassembleInstr(u32 addr, long *len);
    const char *disassembleWords(u32 addr, int len);
    const char *disassembleAddr(u32 addr);

    // Disassembles the currently executed instruction
    const char *disassembleInstr(long *len);
    const char *disassembleWords(int len);
};

#endif
