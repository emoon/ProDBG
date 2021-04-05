// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#define SUPERVISOR_MODE_ONLY if (!reg.sr.s) { execPrivilegeException(); return; }

#define REVERSE_8(x) (((x) * 0x0202020202ULL & 0x010884422010ULL) % 1023)
#define REVERSE_16(x) ((REVERSE_8((x) & 0xFF) << 8) | REVERSE_8(((x) >> 8) & 0xFF))

#define ______________xx(opcode) (u16)((opcode >> 0)  & 0b11)
#define _____________xxx(opcode) (u16)((opcode >> 0)  & 0b111)
#define ____________xxxx(opcode) (u16)((opcode >> 0)  & 0b1111)
#define ________xxxxxxxx(opcode) (u16)((opcode >> 0)  & 0b11111111)
#define __________xxx___(opcode) (u16)((opcode >> 3)  & 0b111)
#define __________xx____(opcode) (u16)((opcode >> 4)  & 0b11)
#define _______xxx______(opcode) (u16)((opcode >> 6)  & 0b111)
#define _________x______(opcode) (u16)((opcode >> 6)  & 0b1)
#define ________x_______(opcode) (u16)((opcode >> 7)  & 0b1)
#define _______x________(opcode) (u16)((opcode >> 8)  & 0b1)
#define _____xx_________(opcode) (u16)((opcode >> 9)  & 0b11)
#define ____xxx_________(opcode) (u16)((opcode >> 9)  & 0b111)
#define ____x___________(opcode) (u16)((opcode >> 11) & 0b1)
#define xxxx____________(opcode) (u16)((opcode >> 12) & 0b1111)

#define STD_AE_FRAME \
(M == MODE_PD && S != Long) ? AE_INC_PC : \
(M == MODE_DI)              ? AE_DEC_PC : \
(M == MODE_IX)              ? AE_DEC_PC : \
(M == MODE_DIPC)            ? AE_DEC_PC : \
(M == MODE_IXPC)            ? AE_DEC_PC : 0

template<Instr I, Mode M, Size S> void
Moira::execShiftRg(u16 opcode)
{
    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);
    int cnt = readD(src) & 0x3F;

    prefetch<POLLIPL>();
    sync((S == Long ? 4 : 2) + 2 * cnt);

    writeD<S>(dst, shift<I,S>(cnt, readD<S>(dst)));
}

template<Instr I, Mode M, Size S> void
Moira::execShiftIm(u16 opcode)
{
    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);
    int cnt = src ? src : 8;

    prefetch<POLLIPL>();
    sync((S == Long ? 4 : 2) + 2 * cnt);

    writeD<S>(dst, shift<I,S>(cnt, readD<S>(dst)));
}

template<Instr I, Mode M, Size S> void
Moira::execShiftEa(u16 op)
{
    int src = _____________xxx(op);

    u32 ea, data;
    if (!readOp<M,S, STD_AE_FRAME>(src, ea, data)) return;

    prefetch();

    writeM<M,S, POLLIPL>(ea, shift<I,S>(1, data));
}

template<Instr I, Mode M, Size S> void
Moira::execAbcd(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    switch (M) {

        case 0: // Dn
        {
            u32 result = bcd<I,Byte>(readD<Byte>(src), readD<Byte>(dst));
            prefetch<POLLIPL>();

            sync(S == Long ? 6 : 2);
            writeD<Byte>(dst, result);
            break;
        }
        default: // Ea
        {
            u32 ea1, ea2, data1, data2;
            if (!readOp<M,S>(src, ea1, data1)) return;
            sync(-2);
            if (!readOp<M,S>(dst, ea2, data2)) return;

            u32 result = bcd<I, Byte>(data1, data2);
            prefetch();

            writeM<M, Byte, POLLIPL>(ea2, result);
            break;
        }
    }
}

template<Instr I, Mode M, Size S> void
Moira::execAddEaRg(u16 opcode)
{
    u32 ea, data, result;
    
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);
    
    if (!readOp<M,S, STD_AE_FRAME>(src, ea, data)) return;
    
    result = addsub<I,S>(data, readD<S>(dst));
    prefetch<POLLIPL>();
    
    if (S == Long) sync(2 + (isMemMode(M) ? 0 : 2));
    writeD<S>(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAddRgEa(u16 opcode)
{
    u32 ea, data, result;

    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    if (!readOp<M,S, STD_AE_FRAME>(dst, ea, data)) return;
    result = addsub<I,S>(readD<S>(src), data);

    prefetch();
    writeM <M, S, POLLIPL> (ea, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAdda(u16 opcode)
{
    u32 ea, data, result;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    if (!readOp<M,S, STD_AE_FRAME>(src, ea, data)) return;
    data = SEXT<S>(data);

    result = (I == ADDA) ? readA(dst) + data : readA(dst) - data;
    prefetch<POLLIPL>();

    sync(2);
    if (S == Word || isRegMode(M) || isImmMode(M)) sync(2);
    writeA(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAddiRg(u16 opcode)
{
    u32 src = readI<S>();
    int dst = _____________xxx(opcode);

    u32 ea, data, result;
    if (!readOp<M,S>(dst, ea, data)) return;

    result = addsub<I,S>(src, data);
    prefetch<POLLIPL>();

    if (S == Long) sync(4);
    writeD<S>(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAddiEa(u16 opcode)
{
    u32 src = readI<S>();
    int dst = _____________xxx(opcode);

    u32 ea, data, result;
    if (!readOp<M,S, STD_AE_FRAME>(dst, ea, data)) return;

    result = addsub<I,S>(src, data);
    prefetch();

    writeOp<M,S, POLLIPL>(dst, ea, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAddqDn(u16 opcode)
{
    i8  src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    if (src == 0) src = 8;
    u32 result = addsub<I,S>(src, readD<S>(dst));
    prefetch<POLLIPL>();

    if (S == Long) sync(4);
    writeD<S>(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAddqAn(u16 opcode)
{
    i8  src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    if (src == 0) src = 8;
    u32 result = (I == ADDQ) ? readA(dst) + src : readA(dst) - src;
    prefetch<POLLIPL>();

    sync(4);
    writeA(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAddqEa(u16 opcode)
{
    i8  src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    u32 ea, data, result;
    if (!readOp<M,S, STD_AE_FRAME>(dst, ea, data)) return;

    if (src == 0) src = 8;
    result = addsub<I,S>(src, data);
    prefetch();

    writeOp<M,S, POLLIPL>(dst, ea, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAddxRg(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 result = addsub<I,S>(readD<S>(src), readD<S>(dst));
    prefetch<POLLIPL>();

    if (S == Long) sync(4);
    writeD<S>(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAddxEa(u16 opcode)
{
    const u64 flags =
    (S == Word) ? AE_INC_PC : (S == Long) ? AE_INC_PC | AE_INC_ADDR : 0;
    
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea1, ea2, data1, data2;
 
    if (!readOp<M,S, flags>(src, ea1, data1)) {
        if (S == Long) undoAnPD<M,S>(src);
        return;
    }
    
    sync(-2);
    
    if (!readOp<M,S, flags>(dst, ea2, data2)) {
        if (S == Long) undoAnPD<M,S>(dst);
        return;
    }

    u32 result = addsub<I,S>(data1, data2);

    if (S == Long && !MIMIC_MUSASHI) {
        writeM <M, Word> (ea2 + 2, result & 0xFFFF);
        prefetch();
        writeM<M, Word, POLLIPL>(ea2, result >> 16);
        return;
    }

    prefetch();
    writeM<M, S, POLLIPL>(ea2, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAndEaRg(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea, data;
    if (!readOp<M,S, STD_AE_FRAME>(src, ea, data)) return;

    u32 result = logic<I,S>(data, readD<S>(dst));
    prefetch<POLLIPL>();

    if (S == Long) sync(isRegMode(M) || isImmMode(M) ? 4 : 2);
    writeD<S>(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAndRgEa(u16 opcode)
{
    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    u32 ea, data;
    if (!readOp<M,S, STD_AE_FRAME>(dst, ea, data)) return;

    u32 result = logic<I,S>(readD<S>(src), data);
    isMemMode(M) ? prefetch() : prefetch<POLLIPL>();

    if (S == Long && isRegMode(M)) sync(4);
    writeOp <M,S, POLLIPL | REVERSE> (dst, ea, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAndiRg(u16 opcode)
{
    u32 src = readI<S>();
    int dst = _____________xxx(opcode);

    u32 result = logic<I,S>(src, readD<S>(dst));
    prefetch<POLLIPL>();

    if (S == Long) sync(4);
    writeD<S>(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAndiEa(u16 opcode)
{
    u32 ea, data, result;

    u32 src = readI<S>();
    int dst = _____________xxx(opcode);

    if (!readOp<M,S, STD_AE_FRAME>(dst, ea, data)) return;

    result = logic<I,S>(src, data);
    prefetch();

    writeOp <M,S, POLLIPL | REVERSE> (dst, ea, result);
}

template<Instr I, Mode M, Size S> void
Moira::execAndiccr(u16 opcode)
{
    u32 src = readI<S>();
    u8  dst = getCCR();

    sync(8);

    u32 result = logic<I,S>(src, dst);
    setCCR(result);

    (void)readM<MEM_DATA, Word>(reg.pc+2);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execAndisr(u16 opcode)
{
    SUPERVISOR_MODE_ONLY

    u32 src = readI<S>();
    u16 dst = getSR();

    sync(8);

    u32 result = logic<I,S>(src, dst);
    setSR(result);

    (void)readM<MEM_DATA, Word>(reg.pc+2);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execBcc(u16 opcode)
{
    sync(2);
    if (cond<I>()) {

        u32 newpc = reg.pc + (S == Word ? (i16)queue.irc : (i8)opcode);
        
        // Check for address error
        if (misaligned<Word>(newpc)) {
            execAddressError(makeFrame(newpc, reg.pc));
            return;
        }
                
        // Take branch
        reg.pc = newpc;
        fullPrefetch<POLLIPL>();

    } else {

        // Fall through to next instruction
        sync(2);
        if (S == Word) readExt();
        prefetch<POLLIPL>();
    }
}

template<Instr I, Mode M, Size S> void
Moira::execBitDxEa(u16 opcode)
{
    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    switch (M) {

        case 0:
        {
            u8 b = readD(src) & 0b11111;
            u32 data = readD(dst);
            data = bit<I>(data, b);

            prefetch<POLLIPL>();

            sync(cyclesBit<I>(b));
            if (I != BTST) writeD(dst, data);
            break;
        }
        default:
        {
            u8 b = readD(src) & 0b111;

            u32 ea, data;
            if (!readOp<M, Byte>(dst, ea, data)) return;

            data = bit<I>(data, b);

            if (I != BTST) {
                prefetch();
                writeM<M, Byte, POLLIPL>(ea, data);
            } else {
                prefetch<POLLIPL>();
            }
        }
    }
}

template<Instr I, Mode M, Size S> void
Moira::execBitImEa(u16 opcode)
{
    u8  src = readI<S>();
    int dst = _____________xxx(opcode);

    switch (M)
    {
        case 0:
        {
            src &= 0b11111;
            u32 data = readD(dst);
            data = bit<I>(data, src);

            prefetch<POLLIPL>();

            sync(cyclesBit<I>(src));
            if (I != BTST) writeD(dst, data);
            break;
        }
        default:
        {
            src &= 0b111;
            u32 ea, data;
            if (!readOp<M,S>(dst, ea, data)) return;

            data = bit<I>(data, src);

            if (I != BTST) {
                prefetch();
                writeM <M, S, POLLIPL> (ea, data);
            } else {
                prefetch<POLLIPL>();
            }
        }
    }
}

template<Instr I, Mode M, Size S> void
Moira::execBsr(u16 opcode)
{
    i16 offset = S == Word ? (i16)queue.irc : (i8)opcode;
    u32 newpc = reg.pc + offset;
    u32 retpc = reg.pc + (S == Word ? 2 : 0);

    // Check for address error
    if (misaligned<Word>(newpc)) {
        execAddressError(makeFrame(newpc));
        return;
    }
    
    // Save return address on stack
    sync(2);
    bool error;
    push <Long> (retpc, error);
    if (error) return;
    
    // Jump to new address
    reg.pc = newpc;

    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execChk(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    i64 c = clock;
    u32 ea, data, dy;
    if (!readOp<M,S, STD_AE_FRAME>(src, ea, data)) return;
    dy = readD<S>(dst);

    sync(6);

    reg.sr.z = ZERO<S>(dy);
    reg.sr.v = 0;
    reg.sr.c = 0;
    reg.sr.n = MIMIC_MUSASHI ? reg.sr.n : 0;

    if ((i16)dy > (i16)data) {

        sync(MIMIC_MUSASHI ? 10 - (int)(clock - c) : 2);
        reg.sr.n = NBIT<S>(dy);
        execTrapException(6);
        return;
    }

    if ((i16)dy < 0) {

        sync(MIMIC_MUSASHI ? 10 - (int)(clock - c) : 4);
        reg.sr.n = MIMIC_MUSASHI ? NBIT<S>(dy) : 1;
        execTrapException(6);
        return;
    }
    
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execClr(u16 opcode)
{
    int dst = _____________xxx(opcode);

    u32 ea, data;
    if (!readOp<M,S, STD_AE_FRAME>(dst, ea, data)) return;

    isMemMode(M) ? prefetch() : prefetch<POLLIPL>();

    if (S == Long && isRegMode(M)) sync(2);
    writeOp <M,S, REVERSE | POLLIPL> (dst, ea, 0);

    reg.sr.n = 0;
    reg.sr.z = 1;
    reg.sr.v = 0;
    reg.sr.c = 0;
}

template<Instr I, Mode M, Size S> void
Moira::execCmp(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea, data;
    if (!readOp<M,S, STD_AE_FRAME>(src, ea, data)) return;

    cmp<S>(data, readD<S>(dst));
    prefetch<POLLIPL>();

    if (S == Long) sync(2);
}

template<Instr I, Mode M, Size S> void
Moira::execCmpa(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea, data;
    if (!readOp<M,S, STD_AE_FRAME>(src, ea, data)) return;

    data = SEXT<S>(data);
    cmp<Long>(data, readA(dst));
    prefetch<POLLIPL>();

    sync(2);
}

template<Instr I, Mode M, Size S> void
Moira::execCmpiRg(u16 opcode)
{
    u32 src = readI<S>();
    int dst = _____________xxx(opcode);

    prefetch<POLLIPL>();

    if (S == Long) sync(2);
    cmp<S>(src, readD<S>(dst));
}

template<Instr I, Mode M, Size S> void
Moira::execCmpiEa(u16 opcode)
{
    u32 src = readI<S>();
    int dst = _____________xxx(opcode);

    u32 ea, data;
    if (!readOp<M,S, STD_AE_FRAME>(dst, ea, data)) return;
    prefetch();

    cmp<S>(src, data);
}

template<Instr I, Mode M, Size S> void
Moira::execCmpm(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea1, ea2, data1, data2;

    if (!readOp<M,S, AE_INC_PC>(src, ea1, data1)) return;
    if (!readOp<M,S, AE_INC_PC>(dst, ea2, data2)) return;

    cmp<S>(data1, data2);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execDbcc(u16 opcode)
{
    sync(2);
    if (!cond<I>()) {

        int dn = _____________xxx(opcode);
        u32 newpc = reg.pc + (i16)queue.irc;

        bool takeBranch = readD<Word>(dn) != 0;
        
        // Check for address error
        if (misaligned<S>(newpc)) {
            execAddressError(makeFrame(newpc, newpc + 2));
            return;
        }
        
        // Decrement loop counter
        writeD<Word>(dn, readD<Word>(dn) - 1);

        // Branch
        if (takeBranch) {
            reg.pc = newpc;
            fullPrefetch<POLLIPL>();
            return;
        } else {
            (void)readM<MEM_PROG, Word>(reg.pc + 2);
        }
    } else {
        sync(2);
    }

    // Fall through to next instruction
    reg.pc += 2;
    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execExgDxDy(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    std::swap(reg.d[src], reg.d[dst]);
    prefetch<POLLIPL>();

    sync(2);
}

template<Instr I, Mode M, Size S> void
Moira::execExgAxDy(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    std::swap(reg.a[src], reg.d[dst]);

    prefetch<POLLIPL>();
    sync(2);
}

template<Instr I, Mode M, Size S> void
Moira::execExgAxAy(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    std::swap(reg.a[src], reg.a[dst]);

    prefetch<POLLIPL>();
    sync(2);
}

template<Instr I, Mode M, Size S> void
Moira::execExt(u16 opcode)
{
    int n = _____________xxx(opcode);

    u32 dn = readD(n);
    dn = (S == Long) ? SEXT<Word>(dn) : SEXT<Byte>(dn);

    writeD<S>(n, dn);
    reg.sr.n = NBIT<S>(dn);
    reg.sr.z = ZERO<S>(dn);
    reg.sr.v = 0;
    reg.sr.c = 0;

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execJmp(u16 opcode)
{
    u32 oldpc = reg.pc;
    
    int src = _____________xxx(opcode);
    u32 ea  = computeEA <M,Long, SKIP_LAST_READ> (src);
    
    const int delay[] = { 0,0,0,0,0,2,4,2,0,2,4,0 };
    sync(delay[M]);
    
    // Check for address error
    if (misaligned<Word>(ea)) {
        execAddressError(makeFrame(ea, oldpc));
        return;
    }
    
    // Jump to new address
    reg.pc = ea;

    // Fill the prefetch queue
    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execJsr(u16 opcode)
{
    int src = _____________xxx(opcode);
    u32 ea  = computeEA<M, Long, SKIP_LAST_READ>(src);
    
    const int delay[] = { 0,0,0,0,0,2,4,2,0,2,4,0 };
    sync(delay[M]);

    /*
    if (M == 5) {
        int x = (i16)queue.irc;
        printf("JSR $%s%x(a%d) [a%d = %x] ea = %x\n", x<0?"-":"", x<0?-(unsigned)x:x, src, src, reg.a[src], ea);
    }
    */
    
    // Check for address error in displacement modes
    if (isDspMode(M) && misaligned<Word>(ea)) {
        execAddressError(makeFrame(ea));
        return;
    }

    // Update program counter
    if (isAbsMode(M) || isDspMode(M)) reg.pc += 2;

    // Check for address error in all other modes
    if (misaligned<Word>(ea)) {
        execAddressError(makeFrame(ea));
        return;
    }

    // Save return address on stack
    bool error;
    push <Long> (reg.pc, error);
    if (error) return;

    // Jump to new address
    reg.pc = ea;

    queue.irc = readM<MEM_PROG, Word>(ea);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execLea(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    reg.a[dst] = computeEA<M,S>(src);
    if (isIdxMode(M)) sync(2);

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execLink(u16 opcode)
{
    u16 ird  = getIRD();
    u32 sp   = getSP() - 4;
    
    int ax   = _____________xxx(opcode);
    i16 disp = (i16)readI<S>();

    // Check for address error
    if (misaligned<Long>(sp)) {
        writeA(ax, sp);
        execAddressError(makeFrame<AE_DATA|AE_WRITE>(sp, getPC() + 2, getSR(), ird));
        return;
    }
    
    // Write to stack
    push <Long> (readA(ax) - ((MIMIC_MUSASHI && ax == 7) ? 4 : 0));

    // Modify address register and stack pointer
    writeA(ax, sp);
    reg.sp += (i32)disp;

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMove0(u16 opcode)
{
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    if (!readOp <M, S, STD_AE_FRAME> (src, ea, data)) return;

    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;

    if (!writeOp <MODE_DN, S> (dst, data)) return;

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMove2(u16 opcode)
{
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    if (!readOp <M,S,STD_AE_FRAME> (src, ea, data)) return;
    
    if (S == Word || (M != MODE_DN && M != MODE_AN && M != MODE_IM)) {
        reg.sr.n = NBIT<Word>(data);
        reg.sr.z = ZERO<Word>(data);
        reg.sr.v = 0;
        reg.sr.c = 0;
    }
        
    if (!writeOp <MODE_AI,S,AE_INC_PC> (dst, data)) return;
    
    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMove3(u16 opcode)
{
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    if (!readOp <M, S, STD_AE_FRAME> (src, ea, data)) return;

    if (S == Word || (M != MODE_DN && M != MODE_AN && M != MODE_IM)) {
        reg.sr.n = NBIT<Word>(data);
        reg.sr.z = ZERO<Word>(data);
        reg.sr.v = 0;
        reg.sr.c = 0;
    }

    if (!writeOp <MODE_PI, S, AE_INC_PC> (dst, data)) return;
    prefetch<POLLIPL>();

    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;
}

template<Instr I, Mode M, Size S> void
Moira::execMove4(u16 opcode)
{
    u16 ird = getIRD();
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    /* Source: http://pasti.fxatari.com/68kdocs/68kPrefetch.html
     *
     * "When the destination addressing mode is pre-decrement, steps 4 and 5
     *  above are inverted. So it behaves like a read modify instruction and it
     *  is a class 0 instruction. Note: The behavior is the same disregarding
     *  transfer size (byte, word or long), and disregarding the source
     *  addressing mode."
     */
    if (!readOp <M,S,STD_AE_FRAME> (src, ea, data)) return;

    // Determine next address error stack frame format
    const u64 flags0 = AE_WRITE | AE_DATA;
    const u64 flags1 = AE_WRITE | AE_DATA | AE_SET_CB3;
    const u64 flags2 = AE_WRITE | AE_DATA;
    int format = (S == Long) ? 0 : reg.sr.c ? 1 : 2;

    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;

    prefetch();
    sync(-2);

    ea = computeEA<MODE_PD, S>(dst);
    
    // Check for address error
    if (misaligned<S>(ea)) {
        if (format == 0) execAddressError(makeFrame<flags0>(ea + 2, reg.pc + 2, getSR(), ird));
        if (format == 1) execAddressError(makeFrame<flags1>(ea, reg.pc + 2), 2);
        if (format == 2) execAddressError(makeFrame<flags2>(ea, reg.pc + 2), 2);
        if (S != Long) updateAn <MODE_PD, S> (dst);
        return;
    }
    
    writeM<MODE_PD, S, REVERSE | POLLIPL>(ea, data);
    updateAn<MODE_PD, S>(dst);
}

template<Instr I, Mode M, Size S> void
Moira::execMove5(u16 opcode)
{
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    if (!readOp <M,S, STD_AE_FRAME> (src, ea, data)) return;
    
    if (S == Long && (M == MODE_DN || M == MODE_AN || M == MODE_IM)) {
        reg.sr.n = NBIT<Word>(data >> 16);
        reg.sr.z = ZERO<Word>(data >> 16) && reg.sr.z;
    } else {
        reg.sr.n = NBIT<S>(data);
        reg.sr.z = ZERO<S>(data);
        reg.sr.v = 0;
        reg.sr.c = 0;
    }
        
    if (!writeOp <MODE_DI,S> (dst, data)) return;
    prefetch<POLLIPL>();
    
    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;
}

template<Instr I, Mode M, Size S> void
Moira::execMove6(u16 opcode)
{
    u32 ea, data;
    
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);
    
    if (!readOp <M,S, STD_AE_FRAME> (src, ea, data)) return;
    
    if (S == Long && (M == MODE_DN || M == MODE_AN || M == MODE_IM)) {
        reg.sr.n = NBIT<Word>(data >> 16);
        reg.sr.z = ZERO<Word>(data >> 16) && reg.sr.z;
    } else {
        reg.sr.n = NBIT<S>(data);
        reg.sr.z = ZERO<S>(data);
        reg.sr.v = 0;
        reg.sr.c = 0;
    }
    
    if (!writeOp <MODE_IX,S> (dst, data)) return;
    prefetch<POLLIPL>();

    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;
}

template<Instr I, Mode M, Size S> void
Moira::execMove7(u16 opcode)
{
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    if (!readOp <M,S, STD_AE_FRAME> (src, ea, data)) return;
    
    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;
    
    if (!writeOp <MODE_AW,S> (dst, data)) return;
    
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMove8(u16 opcode)
{
    u32 ea, data;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    /* Source: http://pasti.fxatari.com/68kdocs/68kPrefetch.html
     *
     * "When the destination addressing mode is long absolute and the source
     *  operand is any memory addr.mode, step 4 is interleaved in the middle of
     *  step 3. Step 3 only performs a single prefetch in this case. The other
     *  prefetch cycle that is normally performed at that step is deferred
     *  after the write cycles. So, two prefetch cycles are performed after the
     *  write ones. It is a class 2 instruction. Note: The behavior is the same
     *  disregarding transfer size (byte, word or long). But if the source
     *  operand is a data or address register, or immediate, then the behavior
     *  is the same as other MOVE variants (class 1 instruction)."
     */
    if (isMemMode(M)) {
        
        if (!readOp <M,S, STD_AE_FRAME> (src, ea, data)) return;
        
        reg.sr.n = NBIT<Word>(data);
        reg.sr.z = ZERO<Word>(data);
        reg.sr.v = 0;
        reg.sr.c = 0;

        u32 ea2 = queue.irc << 16;
        readExt();
        ea2 |= queue.irc;

        if (misaligned<S>(ea2)) {
            execAddressError(makeFrame<AE_WRITE|AE_DATA>(ea2));
            return;
        }

        reg.sr.n = NBIT<S>(data);
        reg.sr.z = ZERO<S>(data);
        reg.sr.v = 0;
        reg.sr.c = 0;

        writeM <MODE_AL,S> (ea2, data);
        readExt();
        
    } else {

        if (!readOp <M,S> (src, ea, data)) return;

        reg.sr.n = NBIT<S>(data);
        reg.sr.z = ZERO<S>(data);
        reg.sr.v = 0;
        reg.sr.c = 0;

        if (!writeOp <MODE_AL,S> (dst, data)) return;
    }

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMovea(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea, data;
    if (!readOp <M,S, STD_AE_FRAME> (src, ea, data)) return;

    prefetch<POLLIPL>();
    writeA(dst, SEXT<S>(data));
}

template<Instr I, Mode M, Size S> void
Moira::execMovemEaRg(u16 opcode)
{
    int src  = _____________xxx(opcode);
    u16 mask = readI<Word>();
    u32 ea   = computeEA<M,S>(src);
    
    // Check for address error
    if (misaligned<S>(ea)) {
        setFC<M>();
        if (M == MODE_IX || M == MODE_IXPC) {
            execAddressError(makeFrame <AE_DEC_PC> (ea));
        } else {
            execAddressError(makeFrame <AE_INC_PC> (ea));
        }
        return;
    }
    
    if (S == Long) (void)readM<MEM_DATA, Word>(ea);

    switch (M) {

        case 3: // (An)+
        {
            for(int i = 0; i <= 15; i++) {

                if (mask & (1 << i)) {
                    writeR(i, SEXT<S>(readM<M,S>(ea)));
                    ea += S;
                }
            }
            writeA(src, ea);
            break;
        }
        default:
        {
            for(int i = 0; i <= 15; i++) {

                if (mask & (1 << i)) {
                    writeR(i, SEXT<S>(readM<M,S>(ea)));
                    ea += S;
                }
            }
            break;
        }
    }
    if (S == Word) (void)readM<MEM_DATA, Word>(ea);
    
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMovemRgEa(u16 opcode)
{
    int dst  = _____________xxx(opcode);
    u16 mask = readI<Word>();

    switch (M) {

        case 4: // -(An)
        {
            u32 ea = readA(dst);
            
            // Check for address error
            if (mask && misaligned<S>(ea)) {
                setFC<M>();
                execAddressError(makeFrame <AE_INC_PC|AE_WRITE> (ea - S));
                return;
            }

            for(int i = 15; i >= 0; i--) {

                if (mask & (0x8000 >> i)) {
                    ea -= S;
                    writeM <M, S, MIMIC_MUSASHI ? REVERSE : 0> (ea, reg.r[i]);
                }
            }
            writeA(dst, ea);
            break;
        }
        default:
        {
            u32 ea = computeEA<M,S>(dst);
            
            // Check for address error
            if (mask && misaligned<S>(ea)) {
                setFC<M>();
                execAddressError(makeFrame <AE_INC_PC|AE_WRITE> (ea));
                return;
            }
  
            for(int i = 0; i < 16; i++) {
                
                if (mask & (1 << i)) {
                    writeM <M, S> (ea, reg.r[i]);
                    ea += S;
                }
            }
            break;
        }
    }
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMovepDxEa(u16 opcode)
{
    int src = ____xxx_________(opcode);
    int dst = _____________xxx(opcode);

    u32 ea = computeEA<M,S>(dst);
    u32 dx = readD(src);

    switch (S) {

        case Long:
        {
            writeM <M,Byte> (ea, (dx >> 24) & 0xFF); ea += 2;
            writeM <M,Byte> (ea, (dx >> 16) & 0xFF); ea += 2;
        }
        case Word:
        {
            writeM <M,Byte> (ea, (dx >>  8) & 0xFF); ea += 2;
            writeM <M,Byte> (ea, (dx >>  0) & 0xFF);
        }
    }
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMovepEaDx(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea = computeEA<M,S>(src);
    u32 dx = 0;

    switch (S) {

        case Long:
        {
            dx |= readM <MEM_DATA, Byte> (ea) << 24; ea += 2;
            dx |= readM <MEM_DATA, Byte> (ea) << 16; ea += 2;
            // fallthrough
        }
        case Word:
        {
            dx |= readM <MEM_DATA, Byte> (ea) << 8; ea += 2;
            dx |= readM <MEM_DATA, Byte> (ea) << 0;
        }

    }
    writeD <S> (dst, dx);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMoveq(u16 opcode)
{
    i8  src = (i8)(opcode & 0xFF);
    int dst = ____xxx_________(opcode);

    writeD <Long> (dst, (i32)src);

    reg.sr.n = NBIT<Byte>(src);
    reg.sr.z = ZERO<Byte>(src);
    reg.sr.v = 0;
    reg.sr.c = 0;

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMoveToCcr(u16 opcode)
{
    int src = _____________xxx(opcode);
    u32 ea, data;
    
    if (!readOp <M,S, STD_AE_FRAME> (src, ea, data)) return;

    sync(4);
    setCCR(data);

    (void)readM <MEM_PROG, Word> (reg.pc + 2);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMoveFromSrRg(u16 opcode)
{
    int dst = _____________xxx(opcode);

    u32 ea, data;
    if (!readOp <M,S> (dst, ea, data)) return;
    prefetch<POLLIPL>();

    sync(2);
    writeD <S> (dst, getSR());
}

template<Instr I, Mode M, Size S> void
Moira::execMoveFromSrEa(u16 opcode)
{
    int dst = _____________xxx(opcode);
    u32 ea, data;
    
    if (!readOp <M,S, STD_AE_FRAME> (dst, ea, data)) return;
    prefetch();

    writeOp <M,S, POLLIPL> (dst, ea, getSR());
}

template<Instr I, Mode M, Size S> void
Moira::execMoveToSr(u16 opcode)
{
    SUPERVISOR_MODE_ONLY

    int src = _____________xxx(opcode);

    u32 ea, data;
    if (!readOp <M,S, STD_AE_FRAME> (src, ea, data)) return;

    sync(4);
    setSR(data);

    (void)readM <MEM_PROG, Word> (reg.pc + 2);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execMoveUspAn(u16 opcode)
{
    SUPERVISOR_MODE_ONLY

    int an = _____________xxx(opcode);
    prefetch<POLLIPL>();
    writeA(an, getUSP());
}

template<Instr I, Mode M, Size S> void
Moira::execMoveAnUsp(u16 opcode)
{
    SUPERVISOR_MODE_ONLY

    int an = _____________xxx(opcode);
    prefetch<POLLIPL>();
    setUSP(readA(an));
}

template<Instr I, Mode M, Size S> void
Moira::execMul(u16 opcode)
{
    if (MIMIC_MUSASHI) {
        execMulMusashi<I, M, S>(opcode);
        return;
    }

    u32 ea, data, result;

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    if (!readOp <M,Word, STD_AE_FRAME> (src, ea, data)) return;

    prefetch<POLLIPL>();
    result = mul<I>(data, readD<Word>(dst));
    
    writeD(dst, result);
}

template<Instr I, Mode M, Size S> void
Moira::execMulMusashi(u16 op)
{
    u32 ea, data, result;

    int src = _____________xxx(op);
    int dst = ____xxx_________(op);

    if (!readOp <M,Word> (src, ea, data)) return;

    prefetch<POLLIPL>();
    result = mulMusashi<I>(data, readD<Word>(dst));

    sync(50);
    writeD(dst, result);
}


template<Instr I, Mode M, Size S> void
Moira::execDiv(u16 opcode)
{
    if (MIMIC_MUSASHI) {
        execDivMusashi<I, M, S>(opcode);
        return;
    }

    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    u32 ea, divisor, result;
    if (!readOp <M,Word, STD_AE_FRAME> (src, ea, divisor)) return;
    u32 dividend = readD(dst);
    
    // Check for division by zero
    if (divisor == 0) {

        if (I == DIVU) {
            reg.sr.n = NBIT<Long>(dividend);
            reg.sr.z = (dividend & 0xFFFF0000) == 0;
            reg.sr.v = 0;
            reg.sr.c = 0;
        } else {
            reg.sr.n = 0;
            reg.sr.z = 1;
            reg.sr.v = 0;
            reg.sr.c = 0;
        }

        sync(8);
        execTrapException(5);
        return;
    }

    result = div<I>(dividend, divisor);

    writeD(dst, result);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execDivMusashi(u16 opcode)
{
    int src = _____________xxx(opcode);
    int dst = ____xxx_________(opcode);

    i64 c = clock;
    u32 ea, divisor, result;
    if (!readOp<M, Word>(src, ea, divisor)) return;

    // Check for division by zero
    if (divisor == 0) {
        sync(8 - (int)(clock - c));
        execTrapException(5);
        return;
    }

    u32 dividend = readD(dst);
    result = divMusashi<I>(dividend, divisor);

    writeD(dst, result);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execNbcd(u16 opcode)
{
    int reg = _____________xxx(opcode);

    switch (M) {

        case 0: // Dn
        {
            prefetch<POLLIPL>();

            sync(2);
            writeD<Byte>(reg, bcd<SBCD, Byte>(readD<Byte>(reg), 0));
            break;
        }
        default: // Ea
        {
            u32 ea, data;
            if (!readOp<M, Byte>(reg, ea, data)) return;
            prefetch();
            writeM<M, Byte, POLLIPL>(ea, bcd <SBCD,Byte> (data, 0));
            break;
        }
    }
}

template<Instr I, Mode M, Size S> void
Moira::execNegRg(u16 opcode)
{
    int dst = ( _____________xxx(opcode) );
    u32 ea, data;

    if (!readOp<M,S>(dst, ea, data)) return;

    data = logic<I,S>(data);
    prefetch<POLLIPL>();

    if (S == Long) sync(2);
    writeD<S>(dst, data);
}

template<Instr I, Mode M, Size S> void
Moira::execNegEa(u16 opcode)
{
    int dst = ( _____________xxx(opcode) );
    u32 ea, data;

    if (!readOp<M,S,STD_AE_FRAME>(dst, ea, data)) return;
    
    data = logic<I,S>(data);
    prefetch();

    writeOp<M,S,POLLIPL>(dst, ea, data);
}

template<Instr I, Mode M, Size S> void
Moira::execNop(u16 opcode)
{
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execPea(u16 opcode)
{
    int src = _____________xxx(opcode);

    u32 ea = computeEA<M,Long>(src);

    if (isIdxMode(M)) sync(2);
    
    if (misaligned(reg.sp)) {
        reg.sp -= S;
        if (isAbsMode(M)) {
            execAddressError(makeFrame<AE_WRITE|AE_DATA>(reg.sp));
        } else {
            execAddressError(makeFrame<AE_WRITE|AE_DATA|AE_INC_PC>(reg.sp));
        }
        return;
    }
    
    if (isAbsMode(M)) {
        push<Long>(ea);
        prefetch<POLLIPL>();
    } else {
        prefetch();
        push<Long, POLLIPL>(ea);
    }
}

template<Instr I, Mode M, Size S> void
Moira::execReset(u16 opcode)
{
    SUPERVISOR_MODE_ONLY
    signalReset();
    
    sync(128);
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execRte(u16 opcode)
{
    SUPERVISOR_MODE_ONLY

    u16 newsr = readM<MEM_DATA, Word>(reg.sp);
    reg.sp += 2;

    u32 newpc = readM<MEM_DATA, Long>(reg.sp);
    reg.sp += 4;

    setSR(newsr);

    if (misaligned(newpc)) {
        execAddressError(makeFrame<AE_PROG>(newpc, reg.pc));
        return;
    }

    setPC(newpc);

    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execRtr(u16 opcode)
{    
    bool error;
    u16 newccr = readM<M, Word>(reg.sp, error);
    if (error) return;
    
    reg.sp += 2;

    u32 newpc = readM<MEM_DATA, Long>(reg.sp);
    reg.sp += 4;
    
    setCCR((u8)newccr);
    
    if (misaligned(newpc)) {
        execAddressError(makeFrame<AE_PROG>(newpc, reg.pc));
        return;
    }
    
    setPC(newpc);

    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execRts(u16 opcode)
{
    bool error;
    u32 newpc = readM<M, Long>(reg.sp, error);
    if (error) return;
 
    reg.sp += 4;

    if (misaligned(newpc)) {
        execAddressError(makeFrame<AE_PROG>(newpc, reg.pc));
        return;
    }
    
    setPC(newpc);
    fullPrefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execSccRg(u16 opcode)
{
    int dst = ( _____________xxx(opcode) );
    u32 ea, data;

    if (!readOp<M,Byte>(dst, ea, data)) return;

    data = cond<I>() ? 0xFF : 0;
    prefetch<POLLIPL>();

    if (data) sync(2);
    writeD<Byte>(dst, data);
}

template<Instr I, Mode M, Size S> void
Moira::execSccEa(u16 opcode)
{
    int dst = ( _____________xxx(opcode) );
    u32 ea, data;

    if (!readOp<M,Byte>(dst, ea, data)) return;

    data = cond<I>() ? 0xFF : 0;
    prefetch();

    writeOp <M,Byte, POLLIPL> (dst, ea, data);
}

template<Instr I, Mode M, Size S> void
Moira::execStop(u16 opcode)
{
    SUPERVISOR_MODE_ONLY

    u16 src = readI<Word>();

    setSR(src);
    flags |= CPU_IS_STOPPED;

    prefetch<POLLIPL>();
    
    signalStop(src);
}

template<Instr I, Mode M, Size S> void
Moira::execSwap(u16 opcode)
{
    int rg  = ( _____________xxx(opcode) );
    u32 dat = readD(rg);

    prefetch<POLLIPL>();

    dat = (dat >> 16) | (dat & 0xFFFF) << 16;
    writeD(rg, dat);

    reg.sr.n = NBIT<Long>(dat);
    reg.sr.z = ZERO<Long>(dat);
    reg.sr.v = 0;
    reg.sr.c = 0;
}

template<Instr I, Mode M, Size S> void
Moira::execTasRg(u16 opcode)
{
    signalTAS();
    
    int dst = ( _____________xxx(opcode) );

    u32 ea, data;
    readOp<M,Byte>(dst, ea, data);

    reg.sr.n = NBIT<Byte>(data);
    reg.sr.z = ZERO<Byte>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;

    data |= 0x80;
    writeD<S>(dst, data);

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execTasEa(u16 opcode)
{
    signalTAS();

    int dst = ( _____________xxx(opcode) );

    u32 ea, data;
    readOp<M,Byte>(dst, ea, data);

    reg.sr.n = NBIT<Byte>(data);
    reg.sr.z = ZERO<Byte>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;
    data |= 0x80;

    if (!isRegMode(M)) sync(2);
    writeOp <M,S> (dst, ea, data);

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execTrap(u16 opcode)
{
    int nr = ____________xxxx(opcode);

    sync(4);
    execTrapException(32 + nr);
}

template<Instr I, Mode M, Size S> void
Moira::execTrapv(u16 opcode)
{
    if (reg.sr.v) {
        execTrapException(7);
        return;
    }
    
    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execTst(u16 opcode)
{
    int rg = _____________xxx(opcode);

    u32 ea, data;
    if (!readOp<M,S, STD_AE_FRAME>(rg, ea, data)) return;

    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    reg.sr.v = 0;
    reg.sr.c = 0;

    prefetch<POLLIPL>();
}

template<Instr I, Mode M, Size S> void
Moira::execUnlk(u16 opcode)
{
    int an = _____________xxx(opcode);

    // Move address register to stack pointer
    if (misaligned(readA(an))) {
        execAddressError(makeFrame<AE_DATA|AE_INC_PC>(readA(an)));
        return;
    }
    reg.sp = readA(an);

    // Update address register
    u32 ea, data;
    if (!readOp<MODE_AI, Long, AE_DATA|AE_INC_PC>(7, ea, data)) return;
    writeA(an, data);

    if (an != 7) reg.sp += 4;
    prefetch<POLLIPL>();
}
