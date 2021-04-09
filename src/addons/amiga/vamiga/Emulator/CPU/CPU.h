// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "CPUTypes.h"
#include "AmigaComponent.h"
#include "Moira.h"

class CPU : public moira::Moira {

    // Result of the latest inspection
    CPUInfo info;

    
    //
    // Initializing
    //

public:

    CPU(Amiga& ref);

    const char *getDescription() const override { return "CPU"; }
    
    void _reset(bool hard) override;
    
    
    //
    // Analyzing
    //
    
public:
    
    CPUInfo getInfo() { return HardwareComponent::getInfo(info); }
        
private:
    
    void _inspect() override;
    void _inspect(u32 dasmStart);
    void _dump(Dump::Category category, std::ostream& os) const override;

    
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

        << flags
        << clock

        << reg.pc
        << reg.pc0
        << reg.sr.t
        << reg.sr.s
        << reg.sr.x
        << reg.sr.n
        << reg.sr.z
        << reg.sr.v
        << reg.sr.c
        << reg.sr.ipl
        << reg.r
        << reg.usp
        << reg.ssp
        << reg.ipl

        << queue.irc
        << queue.ird

        << ipl
        << fcl
        << exception;
    }

    template <class T>
    void applyToResetItems(T& worker)
    {
    }

    isize _size() override { COMPUTE_SNAPSHOT_SIZE }
    isize _load(const u8 *buffer) override { LOAD_SNAPSHOT_ITEMS }
    isize _save(u8 *buffer) override { SAVE_SNAPSHOT_ITEMS }
    isize didLoadFromBuffer(const u8 *buffer) override;


    //
    // Controlling
    //
    
private:

    void _debugOn() override;
    void _debugOff() override;

        
    //
    // Talking to Moira
    //

private:

    /*
    void sync(int cycles) override;
    u8 read8(u32 addr) override;
    u16 read16(u32 addr) override;
    u16 read16OnReset(u32 addr) override;
    u16 read16Dasm(u32 addr) override;
    void write8 (u32 addr, u8  val) override;
    void write16 (u32 addr, u16 val) override;
    u16 readIrqUserVector(u8 level) const override { return 0; }
 
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
    */
    
    //
    // Working with the clock
    //

public:

    // Returns the clock in CPU cycles
    CPUCycle getCpuClock() const { return getClock(); }

    // Returns the clock in master cycles
    Cycle getMasterClock() const { return CPU_CYCLES(getClock()); }

    // Delays the CPU by a certain amout of master cycles
    void addWaitStates(Cycle cycles) { clock += AS_CPU_CYCLES(cycles); }
    
    
    //
    // Running the disassembler
    //
    
    // Disassembles a recorded instruction from the log buffer
    const char *disassembleRecordedInstr(isize i, isize *len);
    const char *disassembleRecordedWords(isize i, isize len);
    const char *disassembleRecordedFlags(isize i);
    const char *disassembleRecordedPC(isize i);

    // Disassembles the instruction at the specified address
    const char *disassembleInstr(u32 addr, isize *len);
    const char *disassembleWords(u32 addr, isize len);
    const char *disassembleAddr(u32 addr);

    // Disassembles the currently executed instruction
    const char *disassembleInstr(isize *len);
    const char *disassembleWords(isize len);
};
