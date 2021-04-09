// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

template<Mode M, Size S, Flags F> bool
Moira::readOp(int n, u32 &ea, u32 &result)
{
    // Handle non-memory modes
    if (M == MODE_DN) { result = readD<S>(n); return true; }
    if (M == MODE_AN) { result = readA<S>(n); return true; }
    if (M == MODE_IM) { result = readI<S>();  return true; }

    // Compute effective address
    ea = computeEA<M,S>(n);

    // Read from effective address
    bool error; result = readM<M,S,F>(ea, error);

    // Emulate -(An) register modification
    updateAnPD<M,S>(n);

    // Exit if an address error has occurred
    if (error) return false;

    // Emulate (An)+ register modification
    updateAnPI<M,S>(n);

    return !error;
}

template<Mode M, Size S, Flags F> bool
Moira::writeOp(int n, u32 val)
{
    // Handle non-memory modes
    if (M == MODE_DN) { writeD<S>(n, val); return true;  }
    if (M == MODE_AN) { writeA<S>(n, val); return true;  }
    if (M == MODE_IM) { assert(false);     return false; }

    // Compute effective address
    u32 ea = computeEA<M,S>(n);

    // Write to effective address
    bool error; writeM <M,S,F> (ea, val, error);

    // Emulate -(An) register modification
    updateAnPD<M,S>(n);

    // Early exit in case of an address error
    if (error) return false;
    
    // Emulate (An)+ register modification
    updateAnPI<M,S>(n);
    
    return !error;
}

template<Mode M, Size S, Flags F> void
Moira::writeOp(int n, u32 ea, u32 val)
{
    // Handle non-memory modes
    if (M == MODE_DN) { writeD <S> (n, val); return; }
    if (M == MODE_AN) { writeA <S> (n, val); return; }
    if (M == MODE_IM) { assert(false);       return; }

    writeM <M,S,F> (ea, val);
}

template<Mode M, Size S, Flags F> u32
Moira::computeEA(u32 n) {

    assert(n < 8);

    u32 result;

    switch (M) {

        case 0:  // Dn
        case 1:  // An
        {
            result = n;
            break;
        }
        case 2:  // (An)
        {
            result = readA(n);
            break;
        }
        case 3:  // (An)+
        {
            result = readA(n);
            break;
        }
        case 4:  // -(An)
        {
            sync(2);
            result = readA(n) - ((n == 7 && S == Byte) ? 2 : S);
            break;
        }
        case 5: // (d,An)
        {
            u32 an = readA(n);
            i16  d = (i16)queue.irc;
            
            result = U32_ADD(an, d);
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 6: // (d,An,Xi)
        {
            i8   d = (i8)queue.irc;
            u32 an = readA(n);
            u32 xi = readR((queue.irc >> 12) & 0b1111);

            result = U32_ADD3(an, d, ((queue.irc & 0x800) ? xi : SEXT<Word>(xi)));

            sync(2);
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 7: // ABS.W
        {
            result = (i16)queue.irc;
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 8: // ABS.L
        {
            result = queue.irc << 16;
            readExt();
            result |= queue.irc;
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 9: // (d,PC)
        {
            i16  d = (i16)queue.irc;

            result = U32_ADD(reg.pc, d);
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 10: // (d,PC,Xi)
        {
            i8   d = (i8)queue.irc;
            u32 xi = readR((queue.irc >> 12) & 0b1111);
            
            result = U32_ADD3(reg.pc, d, ((queue.irc & 0x800) ? xi : SEXT<Word>(xi)));
            sync(2);
            if ((F & SKIP_LAST_READ) == 0) readExt();
            break;
        }
        case 11: // Im
        {
            result = readI<S>();
            break;
        }
        default:
        {
            assert(false);
        }
    }
    return result;
}

template<Mode M, Size S> void
Moira::updateAnPD(int n)
{
    // -(An)
    if (M == 4) reg.a[n] -= (n == 7 && S == Byte) ? 2 : S;
}

template<Mode M, Size S> void
Moira::undoAnPD(int n)
{
    // -(An)
    if (M == 4) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;
}

template<Mode M, Size S> void
Moira::updateAnPI(int n)
{
    // (An)+
    if (M == 3) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;

}

template<Mode M, Size S> void
Moira::updateAn(int n)
{
    // (An)+
    if (M == 3) reg.a[n] += (n == 7 && S == Byte) ? 2 : S;

    // -(An)
    if (M == 4) reg.a[n] -= (n == 7 && S == Byte) ? 2 : S;
}

template<MemSpace M, Size S, Flags F> u32
Moira::readM(u32 addr, bool &error)
{
    // Check for address errors
    if ((error = misaligned<S>(addr))) {
        setFC(M == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
        execAddressError(makeFrame<F>(addr), 2);
        return 0;
    }
    
    return readM<M,S,F>(addr);
}

template<MemSpace M, Size S, Flags F> u32
Moira::readM(u32 addr)
{
    u32 result;
        
    // Break down long word accesses into two word accesses
    if (S == Long) {
        result = readM<M, Word>(addr) << 16;
        result |= readM<M, Word, F>(addr + 2);
        return result;
    }
    
    // Update function code pins
    setFC(M == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);

    // Check if a watchpoint is being accessed
    if ((flags & CPU_CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        watchpointReached(addr);
    }
    
    // Perform the read operation
    sync(2);
    if (F & POLLIPL) pollIrq();
    result = (S == Byte) ? read8(addr & 0xFFFFFF) : read16(addr & 0xFFFFFF);
    sync(2);
    
    return result;
}

template<Mode M, Size S, Flags F> u32
Moira::readM(u32 addr, bool &error)
{
    if (isPrgMode(M)) {
        return readM <MEM_PROG, S, F> (addr, error);
    } else {
        return readM <MEM_DATA, S, F> (addr, error);
    }
}

template<Mode M, Size S, Flags F> u32
Moira::readM(u32 addr)
{
    if (isPrgMode(M)) {
        return readM <MEM_PROG, S, F> (addr);
    } else {
        return readM <MEM_DATA, S, F> (addr);
    }
}

template<MemSpace M, Size S, Flags F> void
Moira::writeM(u32 addr, u32 val, bool &error)
{
    // Check for address errors
    if ((error = misaligned<S>(addr))) {
        setFC(M == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
        execAddressError(makeFrame <F|AE_WRITE> (addr), 2);
        return;
    }
    
    writeM <M,S,F> (addr, val);
}

template<MemSpace M, Size S, Flags F> void
Moira::writeM(u32 addr, u32 val)
{
    // Break down long word accesses into two word accesses
    if (S == Long) {
        if (F & REVERSE) {
            writeM <M, Word>    (addr + 2, val & 0xFFFF);
            writeM <M, Word, F> (addr,     val >> 16   );
        } else {
            writeM <M, Word>    (addr,     val >> 16   );
            writeM <M, Word, F> (addr + 2, val & 0xFFFF);
        }
        return;
    }
    
    // Update function code pins
    setFC(M == MEM_DATA ? FC_USER_DATA : FC_USER_PROG);
    
    // Check if a watchpoint is being accessed
    if ((flags & CPU_CHECK_WP) && debugger.watchpointMatches(addr, S)) {
        watchpointReached(addr);
    }

    // Perform the write operation
    sync(2);
    if (F & POLLIPL) pollIrq();
    S == Byte ? write8(addr & 0xFFFFFF, (u8)val) : write16(addr & 0xFFFFFF, (u16)val);
    sync(2);
}

template<Mode M, Size S, Flags F> void
Moira::writeM(u32 addr, u32 val, bool &error)
{
    if (isPrgMode(M)) {
        writeM <MEM_PROG, S, F> (addr, val, error);
    } else {
        writeM <MEM_DATA, S, F> (addr, val, error);
    }
}

template<Mode M, Size S, Flags F> void
Moira::writeM(u32 addr, u32 val)
{
    if (isPrgMode(M)) {
        writeM <MEM_PROG, S, F> (addr, val);
    } else {
        writeM <MEM_DATA, S, F> (addr, val);
    }
}

template<Size S> u32
Moira::readI()
{
    u32 result;

    switch (S) {
        case Byte:
            result = (u8)queue.irc;
            readExt();
            break;
        case Word:
            result = queue.irc;
            readExt();
            break;
        case Long:
            result = queue.irc << 16;
            readExt();
            result |= queue.irc;
            readExt();
            break;
    }

    return result;
}

template<Size S, Flags F> void
Moira::push(u32 val)
{
    reg.sp -= S;
    writeM <MEM_DATA,S,F> (reg.sp, val);
}

template<Size S, Flags F> void
Moira::push(u32 val, bool &error)
{
    reg.sp -= S;
    writeM <MEM_DATA,S,F> (reg.sp, val, error);
}

template<Size S> bool
Moira::misaligned(u32 addr)
{
    return EMULATE_ADDRESS_ERROR ? ((addr & 1) && S != Byte) : false;
}

template <Flags F> AEStackFrame
Moira::makeFrame(u32 addr, u32 pc, u16 sr, u16 ird)
{
    AEStackFrame frame;
    u16 read = 0x10;
    
    // Prepare
    if (F & AE_WRITE) read = 0;
    if (F & AE_PROG) setFC(FC_USER_PROG);
    if (F & AE_DATA) setFC(FC_USER_DATA);

    // Create
    frame.code = (ird & 0xFFE0) | readFC() | read;
    frame.addr = addr;
    frame.ird = ird;
    frame.sr = sr;
    frame.pc = pc;

    // Adjust
    if (F & AE_INC_PC) frame.pc += 2;
    if (F & AE_DEC_PC) frame.pc -= 2;
    if (F & AE_INC_ADDR) frame.addr += 2;
    if (F & AE_DEC_ADDR) frame.addr -= 2;
    if (F & AE_SET_CB3) frame.code |= (1 << 3);
        
    return frame;
}

template <Flags F> AEStackFrame
Moira::makeFrame(u32 addr, u32 pc)
{
    return makeFrame <F> (addr, pc, getSR(), getIRD());
}

template <Flags F> AEStackFrame
Moira::makeFrame(u32 addr)
{
    return makeFrame <F> (addr, getPC(), getSR(), getIRD());
}

template<Flags F> void
Moira::prefetch()
{
    /* Whereas pc is a moving target (it moves forward while an instruction is
     * being processed, pc0 stays stable throughout the entire execution of
     * an instruction. It always points to the start address of the currently
     * executed instruction.
     */
    reg.pc0 = reg.pc;
    
    queue.ird = queue.irc;
    queue.irc = (u16)readM<MEM_PROG, Word, F>(reg.pc + 2);
}

template<Flags F, int delay> void
Moira::fullPrefetch()
{    
    // Check for address error
    if (misaligned(reg.pc)) {
        execAddressError(makeFrame(reg.pc), 2);
        return;
    }

    queue.irc = (u16)readM<MEM_PROG, Word>(reg.pc);
    if (delay) sync(delay);
    prefetch<F>();
}

void
Moira::readExt()
{
    reg.pc += 2;
    
    // Check for address error
    if (misaligned<Word>(reg.pc)) {
        execAddressError(makeFrame(reg.pc));
        return;
    }
    
    queue.irc = (u16)readM<MEM_PROG, Word>(reg.pc);
}

template<Flags F> void
Moira::jumpToVector(int nr)
{
    exception = nr;
    
    u32 vectorAddr = 4 * nr;
    
    // Update the program counter
    reg.pc = readM<MEM_DATA, Long>(vectorAddr);
    
    // Check for address error
    if (misaligned(reg.pc)) {
        if (nr != 3) {
            execAddressError(makeFrame<F|AE_PROG>(reg.pc, vectorAddr));
        } else {
            halt(); // Double fault
        }
        return;
    }
    
    // Update the prefetch queue
    queue.irc = (u16)readM<MEM_PROG, Word>(reg.pc);
    sync(2);
    prefetch<POLLIPL>();
    
    signalJumpToVector(nr, reg.pc);
}
