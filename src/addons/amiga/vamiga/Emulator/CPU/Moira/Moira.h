// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "AmigaComponent.h"
#include "MoiraConfig.h"
#include "MoiraTypes.h"
#include "MoiraDebugger.h"
#include "StrWriter.h"

#include <assert.h>

namespace moira {

// Execution control flags


class Moira : public AmigaComponent {

    friend class Debugger;
    friend class Breakpoints;
    friend class Watchpoints;

protected:

    // Emulated CPU model (68000 is the only supported model yet)
    CPUModel model = M68000;

    // Interrupt mode of this CPU
    IrqMode irqMode = IRQ_AUTO;

    // Number format used by the disassembler (hex or decimal)
    bool hex = true;

    // Text formatting style used by the disassembler (upper case or lower case)
    bool upper = false;

    // Tab spacing used by the disassembler
    Align tab{8};


    //
    // Internals
    //

public:

    // Breakpoints, watchpoints, instruction tracing
    Debugger debugger = Debugger(*this);

protected:

    /* State flags
     *
     * CPU_IS_HALTED:
     *     Set when the CPU is in "halted" state.
     *
     * CPU_IS_STOPPED:
     *     Set when the CPU is in "stopped" state. This state is entered when
     *     the STOP instruction has been executed. The state is left when the
     *     next interrupt occurs.
     *
     * CPU_LOG_INSTRUCTION:
     *     This flag is set if instruction logging is enabled. If set, the
     *     CPU records the current register contents in a log buffer.
     *
     * CPU_CHECK_INTERRUPTS:
     *     The CPU only checks for pending interrupts if this flag is set.
     *     To accelerate emulation, the CPU deletes this flag if it can assure
     *     that no interrupt can trigger.
     *
     * CPU_TRACE_EXCEPTION:
     *    If this flag is set, the CPU initiates the trace exception.
     *
     * CPU_TRACE_FLAG:
     *    This flag is a copy of the T flag from the status register. The
     *    copy is held to accelerate emulation.
     *
     * CPU_CHECK_BP:
     *    This flag indicates whether the CPU should check for breakpoints.
     *
     * CPU_CHECK_WP:
     *    This flag indicates whether the CPU should check fo watchpoints.
     */
    int flags;
    static const int CPU_IS_HALTED         = (1 << 8);
    static const int CPU_IS_STOPPED        = (1 << 9);
    static const int CPU_LOG_INSTRUCTION   = (1 << 10);
    static const int CPU_CHECK_IRQ         = (1 << 11);
    static const int CPU_TRACE_EXCEPTION   = (1 << 12);
    static const int CPU_TRACE_FLAG        = (1 << 13);
    static const int CPU_CHECK_BP          = (1 << 14);
    static const int CPU_CHECK_WP          = (1 << 15);

    // Number of elapsed cycles since powerup
    i64 clock;

    // The data and address registers
    Registers reg;

    // The prefetch queue
    PrefetchQueue queue;

    // Current value on the IPL pins (Interrupt Priority Level)
    u8 ipl;

    // Value on the lower two function code pins (FC1|FC0)
    u8 fcl;
            
    // Remembers the number of the last processed exception
    int exception;

    // Jump table holding the instruction handlers
    typedef void (Moira::*ExecPtr)(u16);
    ExecPtr exec[65536];

    // Jump table holding the disassebler handlers
    typedef void (Moira::*DasmPtr)(StrWriter&, u32&, u16);
    DasmPtr *dasm = nullptr;
    
private:
    
    // Table holding instruction infos
    InstrInfo *info = nullptr;


    //
    // Constructing
    //

public:

    Moira(Amiga &ref);
    virtual ~Moira();

    void createJumpTables();

    // Configures the output format of the disassembler
    void configDasm(bool h, bool u) { hex = h; upper = u; }


    //
    // Running the CPU
    //

public:

    // Performs a hard reset (power up)
    void reset();

    // Executes the next instruction
    void execute();
    
    // Returns true if the CPU is in HALT state
    bool isHalted() const { return flags & CPU_IS_HALTED; }
    
private:

    // Invoked inside execute() to check for a pending interrupt
    bool checkForIrq();

    // Puts the CPU into HALT state
    void halt();
    

    //
    // Running the Disassembler
    //

public:

    // Disassembles a single instruction and returns the instruction size
    int disassemble(u32 addr, char *str);

    // Returns a textual representation for a single word
    void disassembleWord(u32 value, char *str);

    // Returns a textual representation for one or more words from memory
    void disassembleMemory(u32 addr, int cnt, char *str);

    // Returns a textual representation for the program counter
    void disassemblePC(char *str) { disassemblePC(reg.pc, str); }
    void disassemblePC(u32 pc, char *str);

    // Returns a textual representation for the status register
    void disassembleSR(char *str) { disassembleSR(reg.sr, str); }
    void disassembleSR(const StatusRegister &sr, char *str);

    // Return an info struct for a certain opcode
    InstrInfo getInfo(u16 op); 

    
    //
    // Interfacing with other components
    //

protected:

#if 0
    
    // Reads a byte or a word from memory
    virtual u8 read8(u32 addr) = 0;
    virtual u16 read16(u32 addr) = 0;

    // Special variants used by the reset routine and the disassembler
    virtual u16 read16OnReset(u32 addr) { return read16(addr); }
    virtual u16 read16Dasm(u32 addr) { return read16(addr); }

    // Writes a byte or word into memory
    virtual void write8  (u32 addr, u8  val) = 0;
    virtual void write16 (u32 addr, u16 val) = 0;

    // Provides the interrupt level in IRQ_USER mode
    virtual u16 readIrqUserVector(u8 level) const { return 0; }

    // Instrution delegates
    virtual void signalReset() { };
    virtual void signalStop(u16 op) { };
    virtual void signalTAS() { };

    // State delegates
    virtual void signalHalt() { };

    // Exception delegates
    virtual void signalAddressError(AEStackFrame &frame) { };
    virtual void signalLineAException(u16 opcode) { };
    virtual void signalLineFException(u16 opcode) { };
    virtual void signalIllegalOpcodeException(u16 opcode) { };
    virtual void signalTraceException() { };
    virtual void signalTrapException() { };
    virtual void signalPrivilegeViolation() { };
    virtual void signalInterrupt(u8 level) { };
    virtual void signalJumpToVector(int nr, u32 addr) { };

    // Exception delegates
    virtual void addressErrorHandler() { };
    
    // Called when a breakpoint is reached
    virtual void breakpointReached(u32 addr) { };

    // Called when a breakpoint is reached
    virtual void watchpointReached(u32 addr) { };

#endif
    
    // Reads a byte or a word from memory
    u8 read8(u32 addr);
    u16 read16(u32 addr);

    // Special variants used by the reset routine and the disassembler
    u16 read16OnReset(u32 addr);
    u16 read16Dasm(u32 addr);

    // Writes a byte or word into memory
    void write8  (u32 addr, u8  val);
    void write16 (u32 addr, u16 val);

    // Provides the interrupt level in IRQ_USER mode
    u16 readIrqUserVector(u8 level) const;

    // Instrution delegates
    void signalReset();
    void signalStop(u16 op);
    void signalTAS();

    // State delegates
    void signalHalt();

    // Exception delegates
    void signalAddressError(AEStackFrame &frame);
    void signalLineAException(u16 opcode);
    void signalLineFException(u16 opcode);
    void signalIllegalOpcodeException(u16 opcode);
    void signalTraceException();
    void signalTrapException();
    void signalPrivilegeViolation();
    void signalInterrupt(u8 level);
    void signalJumpToVector(int nr, u32 addr);

    // Exception delegates
    void addressErrorHandler();
    
    // Called when a breakpoint is reached
    void breakpointReached(u32 addr);

    // Called when a breakpoint is reached
    void watchpointReached(u32 addr);
    

    //
    // Accessing the clock
    //

public:

    i64 getClock() const { return clock; }
    void setClock(i64 val) { clock = val; }

protected:

    // Advances the clock (called before each memory access)
    void sync(int cycles); 
    // virtual void sync(int cycles) { clock += cycles; }


    //
    // Accessing registers
    //

public:

    u32 getD(int n) const { return readD(n); }
    void setD(int n, u32 v) { writeD(n,v); }

    u32 getA(int n) const { return readA(n); }
    void setA(int n, u32 v) { writeA(n,v); }

    u32 getPC() const { return reg.pc; }
    void setPC(u32 val) { reg.pc = val; }

    u32 getPC0() const { return reg.pc0; }
    void setPC0(u32 val) { reg.pc0 = val; }

    u16 getIRC() const { return queue.irc; }
    void setIRC(u16 val) { queue.irc = val; }

    u16 getIRD() const { return queue.ird; }
    void setIRD(u16 val) { queue.ird = val; }

    u8 getCCR() const { return getCCR(reg.sr); }
    void setCCR(u8 val);

    u16 getSR() const { return getSR(reg.sr); }
    void setSR(u16 val);

    u32 getSP() const { return reg.sp; }
    void setSP(u32 val) { reg.sp = val; }

    u32 getSSP() const { return reg.sr.s ? reg.sp : reg.ssp; }
    void setSSP(u32 val) { if (reg.sr.s) reg.sp = val; else reg.ssp = val; }

    u32 getUSP() const { return reg.sr.s ? reg.usp : reg.sp; }
    void setUSP(u32 val) { if (reg.sr.s) reg.usp = val; else reg.sp = val; }

    void setSupervisorMode(bool enable);

    u8 getCCR(const StatusRegister &sr) const;
    u16 getSR(const StatusRegister &sr) const;

private:

    void setTraceFlag() { reg.sr.t = true; flags |= CPU_TRACE_FLAG; }
    void clearTraceFlag() { reg.sr.t = false; flags &= ~CPU_TRACE_FLAG; }

protected:

    template<Size S = Long> u32 readD(int n) const;
    template<Size S = Long> u32 readA(int n) const;
    template<Size S = Long> u32 readR(int n) const;
    template<Size S = Long> void writeD(int n, u32 v);
    template<Size S = Long> void writeA(int n, u32 v);
    template<Size S = Long> void writeR(int n, u32 v);

    //
    // Managing the function code pins
    //
    
public:
    
    // Returns the current value on the function code pins
    FunctionCode readFC() { return (FunctionCode)((reg.sr.s ? 4 : 0) | fcl); }

    private:
    
    // Sets the function code pins to a specific value
    void setFC(FunctionCode value);

    // Sets the function code pins according the the provided addressing mode
    template<Mode M> void setFC();


    //
    // Handling interrupts
    //

public:

    u8 getIPL() const { return ipl; }
    void setIPL(u8 val);
    
private:
    
    // Polls the IPL pins
    void pollIrq() { reg.ipl = ipl; }
    
    // Selects the IRQ vector to branch to
    u16 getIrqVector(u8 level) const;
    
#include "MoiraInit.h"
#include "MoiraALU.h"
#include "MoiraDataflow.h"
#include "MoiraExceptions.h"
#include "MoiraDasm.h"
};

}
