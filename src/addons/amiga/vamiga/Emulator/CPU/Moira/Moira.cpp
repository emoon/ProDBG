// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include <stdio.h>
#include <assert.h>
#include <algorithm>

#include "Moira.h"
#include "MoiraConfig.h"

namespace moira {

#include "MoiraInit_cpp.h"
#include "MoiraALU_cpp.h"
#include "MoiraDataflow_cpp.h"
#include "MoiraExceptions_cpp.h"
#include "MoiraExec_cpp.h"
#include "StrWriter_cpp.h"
#include "MoiraDasm_cpp.h"

Moira::Moira()
{
    createJumpTables();
}

void
Moira::reset()
{
    flags = CPU_CHECK_IRQ;

    for(int i = 0; i < 8; i++) reg.d[i] = reg.a[i] = 0xFFFFFFFF;
    reg.usp = 0;
    reg.ipl = 0;
    ipl = 0;
    fcl = 0;
    
    reg.sr.t = 0;
    reg.sr.s = 1;
    reg.sr.x = 0;
    reg.sr.n = 0;
    reg.sr.z = 0;
    reg.sr.v = 0;
    reg.sr.c = 0;
    reg.sr.ipl = 7;

    sync(16);

    // Read the initial (supervisor) stack pointer from memory
    sync(2);
    reg.sp = read16OnReset(0);
    sync(4);
    reg.ssp = reg.sp = read16OnReset(2) | reg.sp << 16;
    sync(4);
    reg.pc = read16OnReset(4);
    sync(4);
    reg.pc = read16OnReset(6) | reg.pc << 16;

    // Fill the prefetch queue
    sync(4);
    queue.irc = read16OnReset(reg.pc & 0xFFFFFF);
    sync(2);
    prefetch();
    
    debugger.reset();
}

void
Moira::execute()
{
    // Check the integrity of the CPU flags
    if (reg.ipl > reg.sr.ipl || reg.ipl == 7) assert(flags & CPU_CHECK_IRQ);
    assert(!!(flags & CPU_TRACE_FLAG) == reg.sr.t);

    // Check the integrity of the program counter
    assert(reg.pc0 == reg.pc);
    
    //
    // The quick execution path: Call the instruction handler and return
    //

    if (!flags) {

        reg.pc += 2;
        (this->*exec[queue.ird])(queue.ird);
        assert(reg.pc0 == reg.pc);
        return;
    }

    //
    // The slow execution path: Process flags one by one
    //

    // Only continue if the CPU is not halted
    if (flags & CPU_IS_HALTED) {
        sync(2);
        return;
    }
        
    // Process pending trace exception (if any)
    if (flags & CPU_TRACE_EXCEPTION) {
        execTraceException();
        goto done;
    }

    // Check if the T flag is set inside the status register
    if (flags & CPU_TRACE_FLAG) {
        flags |= CPU_TRACE_EXCEPTION;
    }

    // Process pending interrupt (if any)
    if (flags & CPU_CHECK_IRQ) {
        if (checkForIrq()) goto done;
    }

    // If the CPU is stopped, poll the IPL lines and return
    if (flags & CPU_IS_STOPPED) {
        
        // Initiate a privilege exception if the supervisor bit is cleared
        if (!reg.sr.s) {
            sync(4);
            reg.pc -= 2;
            flags &= ~CPU_IS_STOPPED;
            execPrivilegeException();
            return;
        }
        
        pollIrq();
        sync(MIMIC_MUSASHI ? 1 : 2);
        return;
    }

    // If logging is enabled, record the executed instruction
    if (flags & CPU_LOG_INSTRUCTION) {
        debugger.logInstruction();
    }

    // Execute the instruction
    reg.pc += 2;
    (this->*exec[queue.ird])(queue.ird);
    assert(reg.pc0 == reg.pc);

done:
    
    // Check if a breakpoint has been reached
    if (flags & CPU_CHECK_BP) {
        
        // Don't break if the instruction won't be executed due to tracing
        if (flags & CPU_TRACE_EXCEPTION) return;
        
        // Compare breakpoint addresses with instruction address
        if (debugger.breakpointMatches(reg.pc0)) breakpointReached(reg.pc0);
    }
}

bool
Moira::checkForIrq()
{
    // pollIrq();
    
    if (reg.ipl > reg.sr.ipl || reg.ipl == 7) {

        // Notify delegate
        assert(reg.ipl < 7);

        // Trigger interrupt
        execIrqException(reg.ipl);
        return true;

    } else {

        // If the polled IPL is up to date, we disable interrupt checking for
        // the time being, because no interrupt can occur as long as the
        // external IPL or the IPL mask inside the status register keep the
        // same. If one of these variables changes, we reenable interrupt
        // checking.
        if (reg.ipl == ipl) flags &= ~CPU_CHECK_IRQ;
        return false;
    }
}

void
Moira::halt()
{
    printf("HALTING CPU\n");
    
    // Halt the CPU
    flags |= CPU_IS_HALTED;
    reg.pc = reg.pc0;

    // Inform the delegate
    signalHalt();
}

template<Size S> u32
Moira::readD(int n)
{
    return CLIP<S>(reg.d[n]);
}

template<Size S> u32
Moira::readA(int n)
{
    return CLIP<S>(reg.a[n]);
}

template<Size S> u32
Moira::readR(int n)
{
    return CLIP<S>(reg.r[n]);
}

template<Size S> void
Moira::writeD(int n, u32 v)
{
    reg.d[n] = WRITE<S>(reg.d[n], v);
}

template<Size S> void
Moira::writeA(int n, u32 v)
{
    reg.a[n] = WRITE<S>(reg.a[n], v);
}

template<Size S> void
Moira::writeR(int n, u32 v)
{
    reg.r[n] = WRITE<S>(reg.r[n], v);
}

u8
Moira::getCCR(const StatusRegister &sr)
{
    return
    sr.c << 0 |
    sr.v << 1 |
    sr.z << 2 |
    sr.n << 3 |
    sr.x << 4;
}

void
Moira::setCCR(u8 val)
{
    reg.sr.c = (val >> 0) & 1;
    reg.sr.v = (val >> 1) & 1;
    reg.sr.z = (val >> 2) & 1;
    reg.sr.n = (val >> 3) & 1;
    reg.sr.x = (val >> 4) & 1;
}

u16
Moira::getSR(const StatusRegister &sr)
{
    return
    sr.t << 15 | sr.s << 13 | sr.ipl << 8 | getCCR();
}

void
Moira::setSR(u16 val)
{
    bool t = (val >> 15) & 1;
    bool s = (val >> 13) & 1;
    u8 ipl = (val >>  8) & 7;

    reg.sr.ipl = ipl;
    flags |= CPU_CHECK_IRQ;
    t ? setTraceFlag() : clearTraceFlag();

    setCCR((u8)val);
    setSupervisorMode(s);
}

void
Moira::setSupervisorMode(bool enable)
{
    if (reg.sr.s == enable) return;

    if (enable) {
        reg.sr.s = 1;
        reg.usp = reg.a[7];
        reg.a[7] = reg.ssp;
    } else {
        reg.sr.s = 0;
        reg.ssp = reg.a[7];
        reg.a[7] = reg.usp;
    }
}

void
Moira::setFC(FunctionCode value)
{
    if (!EMULATE_FC) return;
    fcl = value;
}

template<Mode M> void
Moira::setFC()
{
    if (!EMULATE_FC) return;
    fcl = (M == MODE_DIPC || M == MODE_IXPC) ? FC_USER_PROG : FC_USER_DATA;
}

void
Moira::setIPL(u8 val)
{
    if (ipl != val) {
        ipl = val;
        flags |= CPU_CHECK_IRQ;
    }
}

int
Moira::getIrqVector(int level) {

    assert(level < 8);

    sync(4);

    switch (irqMode) {

        case IRQ_AUTO:          return 24 + level;
        case IRQ_USER:          return readIrqUserVector(level) & 0xFF;
        case IRQ_SPURIOUS:      return 24;
        case IRQ_UNINITIALIZED: return 15;
    }

    assert(false);
    return 0;
}

int
Moira::disassemble(u32 addr, char *str)
{
    u32 pc     = addr;
    u16 opcode = read16Dasm(pc);

    StrWriter writer(str, hex, upper);

    (this->*dasm[opcode])(writer, pc, opcode);
    writer << Finish{};

    return pc - addr + 2;
}

void
Moira::disassembleWord(u32 value, char *str)
{
    sprintx(str, value, true, 0, 4); // Upper case, no '$' prefix, 4 digits
}

void
Moira::disassembleMemory(u32 addr, int cnt, char *str)
{
    addr -= 2; // because dasmRead increases addr first
    for (int i = 0; i < cnt; i++) {
        u32 value = dasmRead<Word>(addr);
        sprintx(str, value, true, 0, 4);
        *str++ = (i == cnt - 1) ? 0 : ' ';
    }
}

void
Moira::disassemblePC(u32 pc, char *str)
{
    sprintx(str, pc, true, 0, 6); // Upper case, no '$' prefix, 6 digits
}

void
Moira::disassembleSR(const StatusRegister &sr, char *str)
{
    str[0]  = sr.t ? 'T' : 't';
    str[1]  = '-';
    str[2]  = sr.s ? 'S' : 's';
    str[3]  = '-';
    str[4]  = '-';
    str[5]  = (sr.ipl & 0b100) ? '1' : '0';
    str[6]  = (sr.ipl & 0b010) ? '1' : '0';
    str[7]  = (sr.ipl & 0b001) ? '1' : '0';
    str[8]  = '-';
    str[9]  = '-';
    str[10] = '-';
    str[11] = sr.x ? 'X' : 'x';
    str[12] = sr.n ? 'N' : 'n';
    str[13] = sr.z ? 'Z' : 'z';
    str[14] = sr.v ? 'V' : 'v';
    str[15] = sr.c ? 'C' : 'c';
    str[16] = 0;
}

void
Moira::disassembleSR(u16 sr, char *str)
{
    str[0]  = (sr & 0b1000000000000000) ? 'T' : 't';
    str[1]  = '-';
    str[2]  = (sr & 0b0010000000000000) ? 'S' : 's';
    str[3]  = '-';
    str[4]  = '-';
    str[5]  = (sr & 0b0000010000000000) ? '1' : '0';
    str[6]  = (sr & 0b0000001000000000) ? '1' : '0';
    str[7]  = (sr & 0b0000000100000000) ? '1' : '0';
    str[8]  = '-';
    str[9]  = '-';
    str[10] = '-';
    str[11] = (sr & 0b0000000000010000) ? 'X' : 'x';
    str[12] = (sr & 0b0000000000001000) ? 'N' : 'n';
    str[13] = (sr & 0b0000000000000100) ? 'Z' : 'z';
    str[14] = (sr & 0b0000000000000010) ? 'V' : 'v';
    str[15] = (sr & 0b0000000000000001) ? 'C' : 'c';
    str[16] = 0;
}

// Make sure the compiler generates certain instances of template functions
template u32 Moira::readD <Long> (int n);
template u32 Moira::readA <Long> (int n);
template void Moira::writeD <Long> (int n, u32 v);
template void Moira::writeA <Long> (int n, u32 v);

}
