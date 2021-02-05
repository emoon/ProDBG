// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// Sanitizer friendly macros for adding signed offsets to u32 values
#define U32_ADD(x,y) (u32)((i64)(x) + (i64)(y))
#define U32_SUB(x,y) (u32)((i64)(x) - (i64)(y))
#define U32_ADD3(x,y,z) (u32)((i64)(x) + (i64)(y) + (i64)(z))
#define U32_SUB3(x,y,z) (u32)((i64)(x) - (i64)(y) - (i64)(z))

// Sanitizer friendly macros for adding signed offsets to u64 values
#define U64_ADD(x,y) (u64)((i64)(x) + (i64)(y))
#define U64_SUB(x,y) (u64)((i64)(x) - (i64)(y))
#define U64_ADD3(x,y,z) (u64)((i64)(x) + (i64)(y) + (i64)(z))
#define U64_SUB3(x,y,z) (u64)((i64)(x) - (i64)(y) - (i64)(z))


template<Size S> u32 MSBIT() {
    if (S == Byte) return 0x00000080;
    if (S == Word) return 0x00008000;
    if (S == Long) return 0x80000000;
}

template<Size S> u32 CLIP(u64 data) {
    if (S == Byte) return data & 0x000000FF;
    if (S == Word) return data & 0x0000FFFF;
    if (S == Long) return data & 0xFFFFFFFF;
}

template<Size S> u32 CLEAR(u64 data) {
    if (S == Byte) return data & 0xFFFFFF00;
    if (S == Word) return data & 0xFFFF0000;
    if (S == Long) return data & 0x00000000;
}

template<Size S> i32 SEXT(u64 data) {
    if (S == Byte) return (i8)data;
    if (S == Word) return (i16)data;
    if (S == Long) return (i32)data;
}

template<Size S> bool NBIT(u64 data) {
    if (S == Byte) return (data & 0x00000080) != 0;
    if (S == Word) return (data & 0x00008000) != 0;
    if (S == Long) return (data & 0x80000000) != 0;
}

template<Size S> bool CARRY(u64 data) {
    if (S == Byte) return data & 0x000000100;
    if (S == Word) return data & 0x000010000;
    if (S == Long) return data & 0x100000000;
}

template<Size S> bool ZERO(u64 data) {
    if (S == Byte) return !(data & 0x000000FF);
    if (S == Word) return !(data & 0x0000FFFF);
    if (S == Long) return !(data & 0xFFFFFFFF);
}

template<Size S> u32 WRITE(u32 d1, u32 d2) {
    if (S == Byte) return (d1 & 0xFFFFFF00) | (d2 & 0x000000FF);
    if (S == Word) return (d1 & 0xFFFF0000) | (d2 & 0x0000FFFF);
    if (S == Long) return d2;
}

template<Instr I, Size S> u32
Moira::shift(int cnt, u64 data) {

    switch(I) {

        case ASL:
        {
            bool carry = false;
            u32 changed = 0;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                u64 shifted = data << 1;
                changed |= (u32)(data ^ shifted);
                data = shifted;
            }
            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = NBIT<S>(changed);
            break;
        }
        case ASR:
        {
            bool carry = false;
            u32 changed = 0;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                u64 shifted = SEXT<S>(data) >> 1;
                changed |= (u32)(data ^ shifted);
                data = shifted;
            }
            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = NBIT<S>(changed);
            break;
        }
        case LSL:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                data = data << 1;
            }
            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case LSR:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                data = data >> 1;
            }
            if (cnt) reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case ROL:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = NBIT<S>(data);
                data = data << 1 | (carry ? 1 : 0);
            }
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case ROR:
        {
            bool carry = false;
            for (int i = 0; i < cnt; i++) {
                carry = data & 1;
                data >>= 1;
                if (carry) data |= MSBIT<S>();
            }
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case ROXL:
        {
            bool carry = reg.sr.x;
            for (int i = 0; i < cnt; i++) {
                bool extend = carry;
                carry = NBIT<S>(data);
                data = data << 1 | (extend ? 1 : 0);
            }

            reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        case ROXR:
        {
            bool carry = reg.sr.x;
            for (int i = 0; i < cnt; i++) {
                bool extend = carry;
                carry = data & 1;
                data >>= 1;
                if (extend) data |= MSBIT<S>();
            }

            reg.sr.x = carry;
            reg.sr.c = carry;
            reg.sr.v = 0;
            break;
        }
        default:
        {
            assert(false);
        }
    }
    reg.sr.n = NBIT<S>(data);
    reg.sr.z = ZERO<S>(data);
    return CLIP<S>(data);
}

template<Instr I, Size S> u32
Moira::addsub(u32 op1, u32 op2)
{
    u64 result;

    switch(I) {

        case ADD:
        case ADDI:
        case ADDQ:
        {
            result = U64_ADD(op1, op2);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            reg.sr.z = ZERO<S>(result);
            break;
        }
        case ADDX:
        {
            result = U64_ADD3(op1, op2, reg.sr.x);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ result) & (op2 ^ result));
            if (CLIP<S>(result)) reg.sr.z = 0;
            break;
        }
        case SUB:
        case SUBI:
        case SUBQ:
        {
            result = U64_SUB(op2, op1);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            reg.sr.z = ZERO<S>(result);
            break;
        }
        case SUBX:
        {
            result = U64_SUB3(op2, op1, reg.sr.x);

            reg.sr.x = reg.sr.c = CARRY<S>(result);
            reg.sr.v = NBIT<S>((op1 ^ op2) & (op2 ^ result));
            if (CLIP<S>(result)) reg.sr.z = 0;
            break;
        }
        default:
        {
            assert(false);
        }
    }
    reg.sr.n = NBIT<S>(result);
    return (u32)result;
}

template <Instr I> u32
Moira::mul(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

         case MULS:
         {
             result = (i16)op1 * (i16)op2;
             break;
         }
         case MULU:
         {
             result = op1 * op2;
             break;
         }
        default: assert(false);
     }

    reg.sr.n = NBIT<Long>(result);
    reg.sr.z = ZERO<Long>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;

    sync(cyclesMul<I>((u16)op1));
    return result;
}

template <Instr I> u32
Moira::div(u32 op1, u32 op2)
{
    u32 result;
    bool overflow;

    reg.sr.n = reg.sr.z = reg.sr.v = reg.sr.c = 0;

    switch (I) {

        case DIVS: // Signed division
        {
            i64 quotient  = (i64)(i32)op1 / (i16)op2;
            i16 remainder = (i64)(i32)op1 % (i16)op2;

            result = (u32)((quotient & 0xffff) | remainder << 16);
            overflow = ((quotient & 0xffff8000) != 0 &&
                        (quotient & 0xffff8000) != 0xffff8000);
            overflow |= op1 == 0x80000000 && (i16)op2 == -1;
            break;
        }
        case DIVU: // Unsigned division
        {
            i64 quotient  = op1 / op2;
            u16 remainder = (u16)(op1 % op2);

            result = (u32)((quotient & 0xffff) | remainder << 16);
            overflow = quotient > 0xFFFF;
            break;
        }
    }
    reg.sr.v = overflow ? 1        : reg.sr.v;
    reg.sr.n = overflow ? 1        : NBIT<Word>(result);
    reg.sr.z = overflow ? reg.sr.z : ZERO<Word>(result);

    sync(cyclesDiv<I>(op1, (u16)op2) - 4);
    return overflow ? op1 : result;
}

template<Instr I, Size S> u32
Moira::bcd(u32 op1, u32 op2)
{
    u64 result;

    switch(I) {

        case ABCD:
        {
            // Extract nibbles
            u16 op1_hi = op1 & 0xF0, op1_lo = op1 & 0x0F;
            u16 op2_hi = op2 & 0xF0, op2_lo = op2 & 0x0F;

            // From portable68000
            u16 resLo = op1_lo + op2_lo + reg.sr.x;
            u16 resHi = op1_hi + op2_hi;
            u64 tmp_result;
            result = tmp_result = resHi + resLo;
            if (resLo > 9) result += 6;
            reg.sr.x = reg.sr.c = (result & 0x3F0) > 0x90;
            if (reg.sr.c) result += 0x60;
            if (CLIP<Byte>(result)) reg.sr.z = 0;
            reg.sr.n = NBIT<Byte>(result);
            reg.sr.v = ((tmp_result & 0x80) == 0) && ((result & 0x80) == 0x80);
            break;
        }
        case SBCD:
        {
            // Extract nibbles
            u16 op1_hi = op1 & 0xF0, op1_lo = op1 & 0x0F;
            u16 op2_hi = op2 & 0xF0, op2_lo = op2 & 0x0F;

            // From portable68000
            u16 resLo = op2_lo - op1_lo - reg.sr.x;
            u16 resHi = op2_hi - op1_hi;
            u64 tmp_result;
            result = tmp_result = resHi + resLo;
            int bcd = 0;
            if (resLo & 0xf0) {
                bcd = 6;
                result -= 6;
            }
            if (((op2 - op1 - reg.sr.x) & 0x100) > 0xff) result -= 0x60;
            reg.sr.c = reg.sr.x = ((op2 - op1 - bcd - reg.sr.x) & 0x300) > 0xff;

            if (CLIP<Byte>(result)) reg.sr.z = 0;
            reg.sr.n = NBIT<Byte>(result);
            reg.sr.v = ((tmp_result & 0x80) == 0x80) && ((result & 0x80) == 0);
            break;
        }
        default:
        {
            assert(false);
        }
    }
    reg.sr.n = NBIT<S>(result);
    return (u32)result;
}

template <Size S> void
Moira::cmp(u32 op1, u32 op2)
{
    u64 result = U64_SUB(op2, op1);
    
    reg.sr.c = NBIT<S>(result >> 1);
    reg.sr.v = NBIT<S>((op2 ^ op1) & (op2 ^ result));
    reg.sr.z = ZERO<S>(result);
    reg.sr.n = NBIT<S>(result);
}

template<Instr I, Size S> u32
Moira::logic(u32 op)
{
    u32 result;

    switch(I) {

        case NOT:
        {
            result = ~op;
            reg.sr.n = NBIT<S>(result);
            reg.sr.z = ZERO<S>(result);
            reg.sr.v = 0;
            reg.sr.c = 0;
            break;
        }
        case NEG:
        {
            result = addsub<SUB,S>(op, 0);
            break;
        }
        case NEGX:
        {
            result = addsub<SUBX,S>(op, 0);
            break;
        }
        default:
        {
            assert(false);
        }
    }
    return result;
}

template<Instr I, Size S> u32
Moira::logic(u32 op1, u32 op2)
{
    u32 result;

    switch(I) {

        case AND: case ANDI: case ANDICCR: case ANDISR:
        {
            result = op1 & op2;
            break;
        }
        case OR: case ORI: case ORICCR: case ORISR:
        {
            result = op1 | op2;
            break;
        }
        case EOR: case EORI: case EORICCR: case EORISR:
        {
            result = op1 ^ op2;
            break;
        }
        default:
        {
            assert(false);
        }
    }

    reg.sr.n = NBIT<S>(result);
    reg.sr.z = ZERO<S>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;
    return result;
}

template <Instr I> u32
Moira::bit(u32 op, u8 bit)
{
    switch (I) {
        case BCHG:
        {
            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op ^= (1 << bit);
            break;
        }
        case BSET:
        {
            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op |= (1 << bit);
            break;
        }
        case BCLR:
        {
            reg.sr.z = 1 ^ ((op >> bit) & 1);
            op &= ~(1 << bit);
            break;
        }
        case BTST:
        {
            reg.sr.z = 1 ^ ((op >> bit) & 1);
            break;
        }
        default:
        {
            assert(false);
        }
    }
    return op;
}

template <Instr I> bool
Moira::cond() {

    switch(I) {

        case BRA: case DBT:  case ST:  return true;
        case DBF: case SF:             return false;
        case BHI: case DBHI: case SHI: return !reg.sr.c && !reg.sr.z;
        case BLS: case DBLS: case SLS: return reg.sr.c || reg.sr.z;
        case BCC: case DBCC: case SCC: return !reg.sr.c;
        case BCS: case DBCS: case SCS: return reg.sr.c;
        case BNE: case DBNE: case SNE: return !reg.sr.z;
        case BEQ: case DBEQ: case SEQ: return reg.sr.z;
        case BVC: case DBVC: case SVC: return !reg.sr.v;
        case BVS: case DBVS: case SVS: return reg.sr.v;
        case BPL: case DBPL: case SPL: return !reg.sr.n;
        case BMI: case DBMI: case SMI: return reg.sr.n;
        case BGE: case DBGE: case SGE: return reg.sr.n == reg.sr.v;
        case BLT: case DBLT: case SLT: return reg.sr.n != reg.sr.v;
        case BGT: case DBGT: case SGT: return reg.sr.n == reg.sr.v && !reg.sr.z;
        case BLE: case DBLE: case SLE: return reg.sr.n != reg.sr.v || reg.sr.z;
    }

    assert(false);
    return 0;
}

template <Instr I> int
Moira::cyclesBit(u8 bit)
{
    switch (I)
    {
        case BTST: return 2;
        case BCLR: return MIMIC_MUSASHI ? 6 : (bit > 15 ? 6 : 4);
        case BSET:
        case BCHG: return MIMIC_MUSASHI ? 4 : (bit > 15 ? 4 : 2);
    }

    assert(false);
    return 0;
}

template <Instr I> int
Moira::cyclesMul(u16 data)
{
    int mcycles = 17;

    switch (I)
    {
        case MULU:
        {
            for (; data; data >>= 1) if (data & 1) mcycles++;
            return 2 * mcycles;
        }
        case MULS:
        {
            data = ((data << 1) ^ data) & 0xFFFF;
            for (; data; data >>= 1) if (data & 1) mcycles++;
            return 2 * mcycles;
        }
    }

    assert(false);
    return 0;
}

template <Instr I> int
Moira::cyclesDiv(u32 op1, u16 op2)
{
    switch (I)
    {
        case DIVU:
        {
            u32 dividend = op1;
            u16 divisor  = op2;
            int mcycles  = 38;

            // Check if quotient is larger than 16 bit
            if ((dividend >> 16) >= divisor) return 10;
            u32 hdivisor = divisor << 16;

            for (int i = 0; i < 15; i++) {
                if ((i32)dividend < 0) {
                    dividend <<= 1;
                    dividend = U32_SUB(dividend, hdivisor);
                } else {
                    dividend <<= 1;
                    if (dividend >= hdivisor) {
                        dividend = U32_SUB(dividend, hdivisor);
                        mcycles += 1;
                    } else {
                        mcycles += 2;
                    }
                }
            }
            return 2 * mcycles;
        }
        case DIVS:
        {
            i32 dividend = (i32)op1;
            i16 divisor  = (i16)op2;
            int mcycles  = (dividend < 0) ? 7 : 6;

            // Check if quotient is larger than 16 bit
            if ((abs(dividend) >> 16) >= abs(divisor))
                return (mcycles + 2) * 2;

            mcycles += 55;

            if (divisor >= 0) {
                mcycles += (dividend < 0) ? 1 : -1;
            }

            u32 aquot = abs(dividend) / abs(divisor);
            for (int i = 0; i < 15; i++) {
                if ( (i16)aquot >= 0) mcycles++;
                aquot <<= 1;
            }
            return 2 * mcycles;
        }
    }

    assert(false);
    return 0;
}

template <Instr I> u32
Moira::mulMusashi(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

        case MULS:
        {
            result = (i16)op1 * (i16)op2;
            break;
        }
        case MULU:
        {
            result = op1 * op2;
            break;
        }
        default: assert(false);
    }

    reg.sr.n = NBIT<Long>(result);
    reg.sr.z = ZERO<Long>(result);
    reg.sr.v = 0;
    reg.sr.c = 0;

    return result;
}

template <Instr I> u32
Moira::divMusashi(u32 op1, u32 op2)
{
    u32 result;

    switch (I) {

        case DIVS:
        {
            sync(154);

            if (op1 == 0x80000000 && (i32)op2 == -1) {

                reg.sr.z = 0;
                reg.sr.n = 0;
                reg.sr.v = 0;
                reg.sr.c = 0;
                result = 0;
                break;
            }

            i64 quotient  = (i64)(i32)op1 / (i16)op2;
            i16 remainder = (i64)(i32)op1 % (i16)op2;

            if (quotient == (i16)quotient) {

                reg.sr.z = quotient;
                reg.sr.n = NBIT<Word>(quotient);
                reg.sr.v = 0;
                reg.sr.c = 0;
                result = (quotient & 0xffff) | (u16)remainder << 16;

            } else {

                result = op1;
                reg.sr.v = 1;
            }
            break;
        }
        case DIVU:
        {
            sync(136);

            i64 quotient  = op1 / op2;
            u16 remainder = (u16)(op1 % op2);

            if(quotient < 0x10000) {

                reg.sr.z = quotient;
                reg.sr.n = NBIT<Word>(quotient);
                reg.sr.v = 0;
                reg.sr.c = 0;

                result = (quotient & 0xffff) | remainder << 16;

            } else {

                result = op1;
                reg.sr.v = 1;
            }
            break;
        }
    }

    return result;
}
