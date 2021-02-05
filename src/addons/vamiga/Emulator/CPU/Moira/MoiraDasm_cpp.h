// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

template <> u32
Moira::dasmRead<Byte>(u32 &addr)
{
    addr += 2;
    return read16Dasm(addr) & 0xFF;
}

template <> u32
Moira::dasmRead<Word>(u32 &addr)
{
    addr += 2;
    return read16Dasm(addr);
}

template <> u32
Moira::dasmRead<Long>(u32 &addr)
{
    u32 result = dasmRead<Word>(addr) << 16;
    result |= dasmRead<Word>(addr);
    return result;
}

int
Moira::baseDispWords(u16 ext)
{
    u16 xx = __________xx____ (ext);

    bool base_disp      = (xx >= 2);
    bool base_disp_long = (xx == 3);

    return base_disp ? (base_disp_long ? 2 : 1) : 0;
}

int
Moira::outerDispWords(u16 ext)
{
    u16 xx = ______________xx (ext);

    bool outer_disp      = (xx >= 2) && (ext & 0x47) < 0x44;
    bool outer_disp_long = (xx == 3) && (ext & 0x47) < 0x44;

    return outer_disp ? (outer_disp_long ? 2 : 1) : 0;
}

template <Mode M, Size S> Ea<M,S>
Moira::Op(u16 reg, u32 &pc)
{
    Ea<M,S> result;
    result.reg = reg;
    result.pc = pc;

    // Read extension words
    switch (M)
    {
        case 5:  // (d,An)
        case 7:  // ABS.W
        case 9:  // (d,PC)
        {
            result.ext1 = dasmRead<Word>(pc);
            break;
        }
        case 8:  // ABS.L
        {
            result.ext1 = dasmRead<Word>(pc);
            result.ext1 = result.ext1 << 16 | dasmRead<Word>(pc);
            break;
        }
        case 6:  // (d,An,Xi)
        case 10: // (d,PC,Xi)
        {
            result.ext1 = dasmRead<Word>(pc);
            result.ext2 = 0;
            result.ext3 = 0;

            if (result.ext1 & 0x100) {

                int dw = baseDispWords((u16)result.ext1);
                if (dw == 1) result.ext2 = dasmRead<Word>(pc);
                if (dw == 2) result.ext2 = dasmRead<Long>(pc);

                int ow = outerDispWords((u16)result.ext1);
                if (ow == 1) result.ext3 = dasmRead<Word>(pc);
                if (ow == 2) result.ext3 = dasmRead<Long>(pc);
            }
            break;
        }
        case 11: // Imm
        {
            result.ext1 = dasmRead<S>(pc);
            break;
        }
        default:
        {
            break;
        }
    }

    return result;
}

void
Moira::dasmIllegal(StrWriter &str, u32 &addr, u16 op)
{
    str << "dc.w " << UInt16{op} << "; ILLEGAL";
}

void
Moira::dasmLineA(StrWriter &str, u32 &addr, u16 op)
{
    str << "dc.w " << tab << UInt16{op} << "; opcode 1010";
}

void
Moira::dasmLineF(StrWriter &str, u32 &addr, u16 op)
{
    str << "dc.w " << tab << UInt16{op} << "; opcode 1111";
}

template<Instr I, Mode M, Size S> void
Moira::dasmShiftRg(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Dn ( _____________xxx(op) );
    auto src = Dn ( ____xxx_________(op) );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmShiftIm(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Imd ( ____xxx_________(op) );
    auto dst = Dn  ( _____________xxx(op) );

    if (src.raw == 0) src.raw = 8;
    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmShiftEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAbcd(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Op <M,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddEaRg(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddRgEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn       ( ____xxx_________(op)       );
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAdda(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = An       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddiRg(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Ims ( SEXT<S>(dasmRead<S>(addr)) );
    auto dst = Dn  ( _____________xxx(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddiEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Ims      ( SEXT<S>(dasmRead<S>(addr)) );
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddqDn(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Imd ( ____xxx_________(op) );
    auto dst = Dn  ( _____________xxx(op) );

    if (src.raw == 0) src.raw = 8;
    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddqAn(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Imd ( ____xxx_________(op) );
    auto dst = An  ( _____________xxx(op) );

    if (src.raw == 0) src.raw = 8;
    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddqEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Imd      ( ____xxx_________(op)       );
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    if (src.raw == 0) src.raw = 8;
    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddxRg(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Op <M,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAddxEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Op <M,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAndEaRg(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAndRgEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn       ( ____xxx_________(op)       );
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAndiRg(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Imu ( dasmRead<S>(addr)    );
    auto dst = Dn  ( _____________xxx(op) );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAndiEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Imu      ( dasmRead<S>(addr)          );
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmAndiccr(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Imu ( dasmRead<S>(addr)         );

    str << Ins<I>{} << tab << src << ", CCR";
}

template<Instr I, Mode M, Size S> void
Moira::dasmAndisr(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Imu ( dasmRead<S>(addr) );

    str << Ins<I>{} << tab << src << ", SR";
}

template<Instr I, Mode M, Size S> void
Moira::dasmBsr(StrWriter &str, u32 &addr, u16 op)
{
    if (MIMIC_MUSASHI && S == Byte && (u8)op == 0xFF) {
        dasmIllegal(str, addr, op);
        return;
    }

    u32 dst = addr + 2;
    dst += (S == Byte) ? (i8)op : (i16)dasmRead<S>(addr);

    str << Ins<I>{} << tab << UInt(dst);
}

template<Instr I, Mode M, Size S> void
Moira::dasmChk(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmClr(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmCmp(StrWriter &str, u32 &addr, u16 op)
{
    Ea<M,S> src = Op <M,S> ( _____________xxx(op), addr );
    Dn      dst = Dn       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmCmpa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = An       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmCmpiRg(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Ims ( SEXT<S>(dasmRead<S>(addr)) );
    auto dst = Dn  ( _____________xxx(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmCmpiEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Ims      ( SEXT<S>(dasmRead<S>(addr)) );
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmCmpm(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Op <M,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmBcc(StrWriter &str, u32 &addr, u16 op)
{
    if (MIMIC_MUSASHI && S == Byte && (u8)op == 0xFF) {
        dasmIllegal(str, addr, op);
        return;
    }

    u32 dst = addr + 2;
    dst += (S == Byte) ? (i8)op : (i16)dasmRead<S>(addr);

    str << Ins<I>{} << tab << UInt(dst);
}

template<Instr I, Mode M, Size S> void
Moira::dasmBitDxEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn       ( ____xxx_________(op)       );
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmBitImEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = dasmRead<S>(addr);
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << "#" << UInt(src) << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmDbcc(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn ( _____________xxx(op) );
    auto dst = addr + 2;

    dst += (i16)dasmRead<Word>(addr);

    str << Ins<I>{} << tab << src << ", " << UInt(dst);
}

template<Instr I, Mode M, Size S> void
Moira::dasmExgDxDy(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn ( ____xxx_________(op) );
    auto dst = Dn ( _____________xxx(op) );

    str << Ins<I>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmExgAxDy(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn ( ____xxx_________(op) );
    auto dst = An ( _____________xxx(op) );

    str << Ins<I>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmExgAxAy(StrWriter &str, u32 &addr, u16 op)
{
    auto src = An ( ____xxx_________(op) );
    auto dst = An ( _____________xxx(op) );

    str << Ins<I>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmExt(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn ( _____________xxx(op) );

    str << Ins<I>{} << Sz<S>{} << tab << Dn{src};
}

template <Instr I, Mode M, Size S> void
Moira::dasmJmp(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << src;
}

template <Instr I, Mode M, Size S> void
Moira::dasmJsr(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> (_____________xxx(op), addr);

    str << Ins<I>{} << tab << src;
}

template <Instr I, Mode M, Size S> void
Moira::dasmLea(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = An       ( ____xxx_________(op)       );

    str << Ins<I>{} << tab << src << ", " << dst;
}

template <Instr I, Mode M, Size S> void
Moira::dasmLink(StrWriter &str, u32 &addr, u16 op)
{
    auto src = An  ( _____________xxx(op)          );
    auto dsp = Ims ( SEXT<S>(dasmRead<Word>(addr)) );

    str << Ins<I>{} << tab << src << ", " << dsp;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMove0(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMove2(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S>       ( _____________xxx(op), addr );
    auto dst = Op <MODE_AI,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMove3(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S>       ( _____________xxx(op), addr );
    auto dst = Op <MODE_PI,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMove4(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S>       ( _____________xxx(op), addr );
    auto dst = Op <MODE_PD,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMove5(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S>       ( _____________xxx(op), addr );
    auto dst = Op <MODE_DI,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMove6(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S>       ( _____________xxx(op), addr );
    auto dst = Op <MODE_IX,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMove7(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S>       ( _____________xxx(op), addr );
    auto dst = Op <MODE_AW,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMove8(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S>       ( _____________xxx(op), addr );
    auto dst = Op <MODE_AL,S> ( ____xxx_________(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMovea(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = An       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMovemEaRg(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = RegRegList ( (u16)dasmRead<Word>(addr)  );
    auto src = Op <M,S>   ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMovemRgEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = RegRegList ( (u16)dasmRead<Word>(addr)  );
    auto dst = Op <M,S>   ( _____________xxx(op), addr );

    if (M == 4) { src.raw = REVERSE_16(src.raw); }
    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMovepDxEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn             ( ____xxx_________(op)       );
    auto dst = Op <MODE_DI,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab;
    str << src << ", (" << UInt(dst.ext1) << "," << An{dst.reg} << ")";
}

template<Instr I, Mode M, Size S> void
Moira::dasmMovepEaDx(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <MODE_DI,S> ( _____________xxx(op), addr );
    auto dst = Dn             ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab;
    str << "(" << UInt(src.ext1) << "," << An{src.reg} << "), " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMoveq(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Ims ( (i8)op               );
    auto dst = Dn  ( ____xxx_________(op) );

    str << Ins<I>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMoveToCcr(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,Byte> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << src << ", CCR";
}

template<Instr I, Mode M, Size S> void
Moira::dasmMoveFromSrRg(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Dn ( _____________xxx(op) );

    str << Ins<I>{} << tab << "SR, " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMoveFromSrEa(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << "SR, " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMoveToSr(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << src << ", SR";
}

template<Instr I, Mode M, Size S> void
Moira::dasmMoveUspAn(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = An ( _____________xxx(op) );

    str << Ins<I>{} << tab << "USP, " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmMoveAnUsp(StrWriter &str, u32 &addr, u16 op)
{
    auto src = An ( _____________xxx(op) );

    str << Ins<I>{} << tab << src << ", USP";
}

template<Instr I, Mode M, Size S> void
Moira::dasmMul(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmDiv(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );
    auto dst = Dn       ( ____xxx_________(op)       );

    str << Ins<I>{} << Sz<S>{} << tab << src << ", " << dst;
}

template <Instr I, Mode M, Size S> void
Moira::dasmNbcd(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<NBCD>{} << tab << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmNop(StrWriter &str, u32 &addr, u16 op)
{
    str << Ins<I>{};
}

template <Instr I, Mode M, Size S> void
Moira::dasmPea(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << src;
}

template <Instr I, Mode M, Size S> void
Moira::dasmReset(StrWriter &str, u32 &addr, u16 op)
{
    str << Ins<I>{};
}

template <Instr I, Mode M, Size S> void
Moira::dasmRte(StrWriter &str, u32 &addr, u16 op)
{
    str << Ins<I>{};
}

template <Instr I, Mode M, Size S> void
Moira::dasmRtr(StrWriter &str, u32 &addr, u16 op)
{
    str << Ins<I>{};
}

template <Instr I, Mode M, Size S> void
Moira::dasmRts(StrWriter &str, u32 &addr, u16 op)
{
    str << Ins<I>{};
}

template<Instr I, Mode M, Size S> void
Moira::dasmSccRg(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Dn ( _____________xxx(op) );

    str << Ins<I>{} << tab << src;
}

template<Instr I, Mode M, Size S> void
Moira::dasmSccEa(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << src;
}

template<Instr I, Mode M, Size S> void
Moira::dasmStop(StrWriter &str, u32 &addr, u16 op)
{
    auto src = Ims ( SEXT<S>(dasmRead<S>(addr)) );

    str << Ins<I>{} << tab << src;
}

template<Instr I, Mode M, Size S> void
Moira::dasmNegRg(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Dn ( _____________xxx(op) );

    str << Ins<I>{} << Sz<S>{} << tab << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmNegEa(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmSwap(StrWriter &str, u32 &addr, u16 op)
{
    Dn reg = Dn ( _____________xxx(op) );

    str << Ins<I>{} << tab << reg;
}

template<Instr I, Mode M, Size S> void
Moira::dasmTasRg(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Dn ( _____________xxx(op) );

    str << Ins<I>{} << tab << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmTasEa(StrWriter &str, u32 &addr, u16 op)
{
    auto dst = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << tab << dst;
}

template<Instr I, Mode M, Size S> void
Moira::dasmTrap(StrWriter &str, u32 &addr, u16 op)
{
    auto nr = Imu ( ____________xxxx(op) );

    str << Ins<I>{} << tab << nr;
}

template<Instr I, Mode M, Size S> void
Moira::dasmTrapv(StrWriter &str, u32 &addr, u16 op)
{
    str << Ins<I>{};
}

template<Instr I, Mode M, Size S> void
Moira::dasmTst(StrWriter &str, u32 &addr, u16 op)
{
    auto ea = Op <M,S> ( _____________xxx(op), addr );

    str << Ins<I>{} << Sz<S>{} << tab << ea;
}

template <Instr I, Mode M, Size S> void
Moira::dasmUnlk(StrWriter &str, u32 &addr, u16 op)
{
    auto reg = An ( _____________xxx(op) );

    str << Ins<I>{} << tab << reg;
}
