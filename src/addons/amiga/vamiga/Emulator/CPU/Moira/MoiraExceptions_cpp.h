// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

void
Moira::saveToStack(AEStackFrame &frame)
{
    // Push PC
    push <Word> ((u16)frame.pc);
    push <Word> (frame.pc >> 16);
    
    // Push SR and IRD
    push <Word> (frame.sr);
    push <Word> (frame.ird);
    
    // Push address
    push <Word> ((u16)frame.addr);
    push <Word> (frame.addr >> 16);
    
    // Push memory access type and function code
    push <Word> (frame.code);
}

void
Moira::saveToStackBrief(u16 sr, u32 pc)
{
    if (MIMIC_MUSASHI) {

        push <Long> (pc);
        push <Word> (sr);

    } else {

        reg.sp -= 6;
        writeM <MEM_DATA, Word> ((reg.sp + 4) & ~1, pc & 0xFFFF);
        writeM <MEM_DATA, Word> ((reg.sp + 0) & ~1, sr);
        writeM <MEM_DATA, Word> ((reg.sp + 2) & ~1, pc >> 16);
    }
}

void
Moira::execAddressError(AEStackFrame frame, int delay)
{
    assert(frame.addr & 1);
    
    // Emulate additional delay
    sync(delay);
    
    // Enter supervisor mode
    setSupervisorMode(true);
    
    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;
    sync(8);

    // Write stack frame
    bool doubleFault;
    if (!(doubleFault = misaligned<Word>(reg.sp))) {
        
        saveToStack(frame);
        sync(2);
        jumpToVector(3);
    }
    
    // Inform the delegate
    signalAddressError(frame);
    
    // Halt the CPU if a double fault occurred
    if (doubleFault) halt();
}

void
Moira::execUnimplemented(int nr)
{
    u16 status = getSR();
    
    // Enter supervisor mode
    setSupervisorMode(true);
    
    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;

    // Write exception information to stack
    sync(4);
    saveToStackBrief(status, reg.pc - 2);

    jumpToVector<AE_SET_CB3>(nr);
}

void
Moira::execLineA(u16 opcode)
{
    signalLineAException(opcode);
    execUnimplemented(10);
}

void
Moira::execLineF(u16 opcode)
{
    signalLineFException(opcode);
    execUnimplemented(11);
}

void
Moira::execIllegal(u16 opcode)
{
    signalIllegalOpcodeException(opcode);
    execUnimplemented(4);
}

void
Moira::execTraceException()
{
    signalTraceException();
    
    u16 status = getSR();

    // Recover from stop state
    flags &= ~CPU_IS_STOPPED;

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;

    // Write exception information to stack
    sync(4);
    saveToStackBrief(status, reg.pc);

    jumpToVector(9);
}

void
Moira::execTrapException(int nr)
{
    signalTrapException();
    
    u16 status = getSR();

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing, but keep the CPU_TRACE_EXCEPTION flag
    clearTraceFlag();

    // Write exception information to stack
    saveToStackBrief(status);

    jumpToVector(nr);
}

void
Moira::execPrivilegeException()
{
    signalPrivilegeViolation();
    
    u16 status = getSR();

    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;

    // Write exception information to stack
    sync(4);
    saveToStackBrief(status, reg.pc - 2);

    jumpToVector<AE_SET_CB3>(8);
}

void
Moira::execIrqException(u8 level)
{
    assert(level < 8);
    signalInterrupt(level);
    
    // Remember the current value of the status register
    u16 status = getSR();

    // Recover from stop state
    flags &= ~CPU_IS_STOPPED;

    // Clear the polled IPL value
    reg.ipl = 0;

    // Temporarily raise the interrupt threshold
    reg.sr.ipl = level;
    
    // Enter supervisor mode
    setSupervisorMode(true);

    // Disable tracing
    clearTraceFlag();
    flags &= ~CPU_TRACE_EXCEPTION;
        
    sync(6);
    reg.sp -= 6;
    writeM <MEM_DATA, Word> (reg.sp + 4, reg.pc & 0xFFFF);

    sync(4);
    queue.ird = getIrqVector(level);
    
    sync(4);
    writeM <MEM_DATA, Word> (reg.sp + 0, status);
    writeM <MEM_DATA, Word> (reg.sp + 2, reg.pc >> 16);

    jumpToVector<AE_SET_CB3>(queue.ird);
}
