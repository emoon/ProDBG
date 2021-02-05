// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

void
CPU::sync(int cycles)
{
    // Advance the CPU clock
    clock += cycles;

    // Emulate Agnus up to the same cycle
    agnus.executeUntil(CPU_CYCLES(clock));
}

u8
CPU::read8(u32 addr)
{
    return mem.peek8 <ACCESSOR_CPU> (addr);
}

u16
CPU::read16(u32 addr)
{
    u16 result = mem.peek16 <ACCESSOR_CPU> (addr);
 
    /*
    static int counter = 0;
    if (addr == 0xc001b0) {
        if (counter == 928) {
            amiga.signalStop();
            // COPREG_DEBUG = 1;
        }
        debug("%d: exec::allocMem(%x,%x)\n", counter++, reg.d[0], reg.d[1]);
    }
    */
    /*
    if (addr >= 0xE80000 && addr <= 0xE8FFFF) {
        debug("get_word: Zorro(%x)  = %x\n", addr, result);
    }
    */
    /*
    if (addr >= 0xDC0000 && addr <= 0xDEFFFF) {
        debug("read16(%x) = %x\n", addr, result);
    }
    
    if (addr >= 0xD80000 && addr <= 0xD8FFFF) {
        debug("read16(%x) = %x (%x,%x %x,%x %x,%x)\n", addr, result,
              agnus.busOwner[agnus.pos.h-2], agnus.busValue[agnus.pos.h-2],
              agnus.busOwner[agnus.pos.h-1], agnus.busValue[agnus.pos.h-1],
              agnus.busOwner[agnus.pos.h-0], agnus.busValue[agnus.pos.h-0]);
    }
    */
    
    return result;
}

u16
CPU::read16Dasm(u32 addr)
{
    return mem.spypeek16 <ACCESSOR_CPU> (addr);
}

u16
CPU::read16OnReset(u32 addr)
{
    return mem.chip ? read16(addr) : 0;
}

void
CPU::write8(u32 addr, u8 val)
{
    trace(XFILES && addr - reg.pc < 5, "XFILES: write8 close to PC %x\n", reg.pc);

    mem.poke8 <ACCESSOR_CPU> (addr, val);
}

void
CPU::write16 (u32 addr, u16 val)
{
    trace(XFILES && addr - reg.pc < 5, "XFILES: write16 close to PC %x\n", reg.pc);

    mem.poke16 <ACCESSOR_CPU> (addr, val);
}

void
CPU::signalReset()
{
    trace(XFILES, "XFILES: RESET instruction\n");
    amiga.softReset();
}

void
CPU::signalStop(u16 op)
{
    if (!(op & 0x2000)) {
        trace(XFILES, "XFILES: STOP instruction (%x)\n", op);
    }
}

void
CPU::signalTAS()
{
    trace(XFILES, "XFILES: TAS instruction\n");
}

void
CPU::signalHalt()
{
    messageQueue.put(MSG_CPU_HALT);
}

void
CPU::signalAddressError(moira::AEStackFrame &frame)
{
    trace(XFILES, "XFILES: Address error exception %x %x %x %x %x\n",
          frame.code, frame.addr, frame.ird, frame.sr, frame.pc);
    // amiga.signalStop();
}

void
CPU::signalLineAException(u16 opcode)
{
    trace(XFILES, "XFILES: lineAException(%x)\n", opcode);
}

void
CPU::signalLineFException(u16 opcode)
{
    trace(XFILES, "XFILES: lineFException(%x)\n", opcode);
}

void
CPU::signalIllegalOpcodeException(u16 opcode)
{
    trace(XFILES, "XFILES: illegalOpcodeException(%x)\n", opcode);
}

void
CPU::signalTraceException()
{
    // debug(XFILES, "XFILES: traceException\n");
}

void
CPU::signalTrapException()
{
    trace(XFILES, "XFILES: trapException\n");
}

void
CPU::signalPrivilegeViolation()
{
}

void
CPU::signalInterrupt(u8 level)
{
    debug(INT_DEBUG, "Executing level %d IRQ\n", level);
}

void
CPU::signalJumpToVector(int nr, u32 addr)
{
    bool isIrqException = nr >= 24 && nr <= 31;

    if (isIrqException) {
        trace(INT_DEBUG, "Exception %d: Changing PC to %x\n", nr, addr);
    }
}

void
CPU::breakpointReached(u32 addr)
{
    amiga.setControlFlags(RL_BREAKPOINT_REACHED);
}

void
CPU::watchpointReached(u32 addr)
{
    amiga.setControlFlags(RL_WATCHPOINT_REACHED);
}

CPU::CPU(Amiga& ref) : AmigaComponent(ref)
{
}

void
CPU::_reset(bool hard)
{    
    RESET_SNAPSHOT_ITEMS(hard)

    if (hard) {
                
        // Reset the Moira core
        Moira::reset();
        
        // Remove all previously recorded instructions
        debugger.clearLog();
        
    } else {
        
        /* "The RESET instruction causes the processor to assert RESET for 124
         *  clock periods toreset the external devices of the system. The
         *  internal state of the processor is notaffected. Neither the status
         *  register nor any of the internal registers is affected by an
         *  internal reset operation. All external devices in the system should
         *  be reset at the completion of the RESET instruction."
         *      [Motorola M68000 User Manual]
         */            
    }
}

void
CPU::_inspect()
{
    _inspect(getPC0());
}

void
CPU::_inspect(u32 dasmStart)
{
    synchronized {
        
        info.pc0 = getPC0() & 0xFFFFFF;
        
        for (isize i = 0; i < 8; i++) {
            info.d[i] = getD((int)i);
            info.a[i] = getA((int)i);
        }
        info.usp = getUSP();
        info.ssp = getSSP();
        info.sr = getSR();
    }
}

void
CPU::_dump() const
{
    msg("     PC0: %8X\n", reg.pc0);
    msg(" D0 - D3: ");
    for (isize i = 0; i < 4; i++) msg("%8X ", reg.d[i]);
    msg("\n");
    msg(" D4 - D7: ");
    for (isize i = 4; i < 8; i++) msg("%8X ", reg.d[i]);
    msg("\n");
    msg(" A0 - A3: ");
    for (isize i = 0; i < 4; i++) msg("%8X ", reg.a[i]);
    msg("\n");
    msg(" A4 - A7: ");
    for (isize i = 4; i < 8; i++) msg("%8X ", reg.a[i]);
    msg("\n");
    msg("     SSP: %X\n", reg.ssp);
    msg("   Flags: %X\n", getSR());
}

void
CPU::_setDebug(bool enable)
{
    if (enable) {
         
         msg("Enabling debug mode\n");
         debugger.enableLogging();

     } else {

         msg("Disabling debug mode\n");
         debugger.disableLogging();
     }
}

isize
CPU::didLoadFromBuffer(const u8 *buffer)
{
    /* Because we don't save breakpoints and watchpoints in a snapshot, the
     * CPU flags for checking breakpoints and watchpoints can be in a corrupt
     * state after loading. These flags need to be updated according to the
     * current breakpoint and watchpoint list.
     */
    debugger.breakpoints.setNeedsCheck(debugger.breakpoints.elements() != 0);
    debugger.watchpoints.setNeedsCheck(debugger.watchpoints.elements() != 0);
    return 0;
}

const char *
CPU::disassembleRecordedInstr(isize i, isize *len)
{
    return disassembleInstr(debugger.logEntryAbs((int)i).pc0, len);
}
const char *
CPU::disassembleRecordedWords(isize i, isize len)
{
    return disassembleWords(debugger.logEntryAbs((int)i).pc0, len);
}

const char *
CPU::disassembleRecordedFlags(isize i)
{
    static char result[18];
    
    disassembleSR(debugger.logEntryAbs((int)i).sr, result);
    return result;
}

const char *
CPU::disassembleRecordedPC(isize i)
{
    static char result[16];
    
    Moira::disassemblePC(debugger.logEntryAbs((int)i).pc0, result);
    return result;
}

const char *
CPU::disassembleInstr(u32 addr, isize *len)
{
    static char result[128];

    int l = disassemble(addr, result);

    if (len) *len = (isize)l;
    return result;
}

const char *
CPU::disassembleWords(u32 addr, isize len)
{
    static char result[64];

    disassembleMemory(addr, (int)len, result);
    return result;
}

const char *
CPU::disassembleAddr(u32 addr)
{
    static char result[16];

    disassemblePC(addr, result);
    return result;
}

const char *
CPU::disassembleInstr(isize *len)
{
    return disassembleInstr(reg.pc0, len);
}
const char *
CPU::disassembleWords(isize len)
{
    return disassembleWords(reg.pc0, len);
    return "";
}
