// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

// Adds a single entry to the instruction jump table

#define TPARAM(x,y,z) <x,y,z>
#define bind(id, name, I, M, S) { \
assert(exec[id] == &Moira::execIllegal); \
if (dasm) assert(dasm[id] == &Moira::dasmIllegal); \
exec[id] = &Moira::exec##name TPARAM(I, M, S); \
if (dasm) dasm[id] = &Moira::dasm##name TPARAM(I, M, S); \
if (info) info[id] = InstrInfo { I, M, S }; \
}

// Registers an instruction in one of the standard instruction formats:
//
//     ____ ____ ____ _XXX      XXX : Operand parameter (Register number etc.)
//     ____ ____ ____ XXXX        S : Size information  (Word or Long)
//     ____ ____ XXXX XXXX       SS : Size information  (Byte or Word or Long)
//     ____ XXX_ ____ _XXX
//     ____ XXX_ XXXX XXXX
//     ____ ____ __MM MXXX
//     ____ XXX_ __MM MXXX
//     ____ XXX_ SS__ _XXX
//     ____ ____ SSMM MXXX
//     ____ XXX_ SSMM MXXX
//     ____ ___S __MM MXXX
//     ____ XXXS __MM MXXX
//     __SS ____ __MM MXXX
//     __SS XXX_ __MM MXXX

#define _____________XXX(op,I,M,S,f) { \
for (int j = 0; j < 8; j++) bind((op) | j, f, I, M, S); }

#define ____________XXXX(op,I,M,S,f) { \
for (int j = 0; j < 16; j++) bind((op) | j, f, I, M, S); }

#define ________XXXXXXXX(op,I,M,S,f) { \
for (int j = 0; j < 256; j++) bind((op) | j, f, I, M, S); }

#define ____XXX______XXX(op,I,M,S,f) { \
for (int i = 0; i < 8; i++) _____________XXX((op) | i << 9, I, M, S, f); }

#define ____XXX_XXXXXXXX(op,I,M,S,f) { \
for (int i = 0; i < 8; i++) ________XXXXXXXX((op) | i << 9, I, M, S, f); }

#define __________MMMXXX(op,I,m,S,f) { \
for (int j = 0; j < 8; j++) { \
if ((m) & 0b100000000000) bind((op) | 0 << 3 | j, f, I,  (Mode)0, S); \
if ((m) & 0b010000000000) bind((op) | 1 << 3 | j, f, I,  (Mode)1, S); \
if ((m) & 0b001000000000) bind((op) | 2 << 3 | j, f, I,  (Mode)2, S); \
if ((m) & 0b000100000000) bind((op) | 3 << 3 | j, f, I,  (Mode)3, S); \
if ((m) & 0b000010000000) bind((op) | 4 << 3 | j, f, I,  (Mode)4, S); \
if ((m) & 0b000001000000) bind((op) | 5 << 3 | j, f, I,  (Mode)5, S); \
if ((m) & 0b000000100000) bind((op) | 6 << 3 | j, f, I,  (Mode)6, S); \
} \
if ((m) & 0b000000010000) bind((op) | 7 << 3 | 0, f, I,  (Mode)7, S); \
if ((m) & 0b000000001000) bind((op) | 7 << 3 | 1, f, I,  (Mode)8, S); \
if ((m) & 0b000000000100) bind((op) | 7 << 3 | 2, f, I,  (Mode)9, S); \
if ((m) & 0b000000000010) bind((op) | 7 << 3 | 3, f, I, (Mode)10, S); \
if ((m) & 0b000000000001) bind((op) | 7 << 3 | 4, f, I, (Mode)11, S); }

#define ____XXX___MMMXXX(op,I,m,S,f) { \
for (int i = 0; i < 8; i++) __________MMMXXX((op) | i << 9, I, m, S, f) }

#define ____XXX_SS___XXX(op,I,M,s,f) { \
if ((s) & 0b100) ____XXX______XXX((op) | 2 << 6, I, M, Long, f); \
if ((s) & 0b010) ____XXX______XXX((op) | 1 << 6, I, M, Word, f); \
if ((s) & 0b001) ____XXX______XXX((op) | 0 << 6, I, M, Byte, f); }

#define ________SSMMMXXX(op,I,m,s,f) { \
if ((s) & 0b100) __________MMMXXX((op) | 2 << 6, I, m, Long, f); \
if ((s) & 0b010) __________MMMXXX((op) | 1 << 6, I, m, Word, f); \
if ((s) & 0b001) __________MMMXXX((op) | 0 << 6, I, m, Byte, f); }

#define ____XXX_SSMMMXXX(op,I,m,s,f) { \
if ((s) & 0b100) ____XXX___MMMXXX((op) | 2 << 6, I, m, Long, f); \
if ((s) & 0b010) ____XXX___MMMXXX((op) | 1 << 6, I, m, Word, f); \
if ((s) & 0b001) ____XXX___MMMXXX((op) | 0 << 6, I, m, Byte, f); }

#define _______S__MMMXXX(op,I,m,s,f) { \
if ((s) & 0b100) __________MMMXXX((op) | 1 << 8, I, m, Long, f); \
if ((s) & 0b010) __________MMMXXX((op) | 0 << 8, I, m, Word, f); \
if ((s) & 0b001) assert(false); }

#define ____XXXS__MMMXXX(op,I,m,s,f) { \
if ((s) & 0b100) ____XXX___MMMXXX((op) | 1 << 8, I, m, Long, f); \
if ((s) & 0b010) ____XXX___MMMXXX((op) | 0 << 8, I, m, Word, f); \
if ((s) & 0b001) assert(false); }

#define __SS______MMMXXX(op,I,m,s,f) { \
if ((s) & 0b100) __________MMMXXX((op) | 2 << 12, I, m, Long, f); \
if ((s) & 0b010) __________MMMXXX((op) | 3 << 12, I, m, Word, f); \
if ((s) & 0b001) __________MMMXXX((op) | 1 << 12, I, m, Byte, f); }

#define __SSXXX___MMMXXX(op,I,m,s,f) { \
if ((s) & 0b100) ____XXX___MMMXXX((op) | 2 << 12, I, m, Long, f); \
if ((s) & 0b010) ____XXX___MMMXXX((op) | 3 << 12, I, m, Word, f); \
if ((s) & 0b001) ____XXX___MMMXXX((op) | 1 << 12, I, m, Byte, f); }


static u16
parse(const char *s, int sum = 0)
{
    return
    *s == ' ' ? parse(s + 1, sum) :
    *s == '-' ? parse(s + 1, sum << 1) :
    *s == '0' ? parse(s + 1, sum << 1) :
    *s == '1' ? parse(s + 1, (sum << 1) + 1) : (u16)sum;
}

void
Moira::createJumpTables()
{
    u16 opcode;

    //
    // Start with clean tables
    //

    for (int i = 0; i < 0x10000; i++) {
        exec[i] = &Moira::execIllegal;
        if (dasm) dasm[i] = &Moira::dasmIllegal;
        if (info) info[i] = InstrInfo { ILLEGAL, MODE_IP, (Size)0 };
    }


    // Unimplemented instructions
    //
    //       Format: 1010 ---- ---- ---- (Line A instructions)
    //               1111 ---- ---- ---- (Line F instructions)

    for (int i = 0; i < 0x1000; i++) {

        exec[0b1010 << 12 | i] = &Moira::execLineA;
        if (dasm) dasm[0b1010 << 12 | i] = &Moira::dasmLineA;
        if (info) info[0b1010 << 12 | i] = InstrInfo { LINE_A, MODE_IP, (Size)0 };

        exec[0b1111 << 12 | i] = &Moira::execLineF;
        if (dasm) dasm[0b1111 << 12 | i] = &Moira::dasmLineF;
        if (info) info[0b1111 << 12 | i] = InstrInfo { LINE_F, MODE_IP, (Size)0 };
    }


    // ABCD
    //
    //       Syntax: (1) ABCD Dx,Dy
    //               (2) ABCD -(Ax),-(Ay)
    //         Size: Byte

    // Dx,Dy
    opcode = parse("1100 ---1 0000 0---");
    ____XXX______XXX(opcode, ABCD, MODE_DN, Byte, Abcd);

    // -(Ax),-(Ay)
    opcode = parse("1100 ---1 0000 1---");
    ____XXX______XXX(opcode, ABCD, MODE_PD, Byte, Abcd);


    // ADD
    //
    //       Syntax: (1) ADD <ea>,Dy
    //               (2) ADD Dx,<ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X   X   X   X

    opcode = parse("1101 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, ADD, 0b101111111111, Byte,        AddEaRg);
    ____XXX_SSMMMXXX(opcode, ADD, 0b111111111111, Word | Long, AddEaRg);

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

     opcode = parse("1101 ---1 ---- ----");
     ____XXX_SSMMMXXX(opcode, ADD, 0b001111111000, Byte | Word | Long, AddRgEa);


    // ADDA
    //
    //       Syntax: ADDA <ea>,Ay
    //         Size: Word, Longword
    //
    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("1101 ---- 11-- ----");
    ____XXXS__MMMXXX(opcode, ADDA, 0b111111111111, Word | Long, Adda)


    // ADDI
    //
    //       Syntax: ADDI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 0110 ---- ----");
    ________SSMMMXXX(opcode, ADDI, 0b100000000000, Byte | Word | Long, AddiRg);
    ________SSMMMXXX(opcode, ADDI, 0b001111111000, Byte | Word | Long, AddiEa);


    // ADDQ
    //
    //       Syntax: ADDQ #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X

    opcode = parse("0101 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, ADDQ, 0b100000000000, Byte | Word | Long, AddqDn);
    ____XXX_SSMMMXXX(opcode, ADDQ, 0b010000000000, Word | Long,        AddqAn);
    ____XXX_SSMMMXXX(opcode, ADDQ, 0b001111111000, Byte | Word | Long, AddqEa);


    // ADDX
    //
    //       Syntax: (1) ADDX Dx,Dy
    //               (2) ADDX -(Ax),-(Ay)
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1101 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, ADDX, MODE_DN, Byte | Word | Long, AddxRg);

    // -(Ax),-(Ay)
    opcode = parse("1101 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, ADDX, MODE_PD, Byte | Word | Long, AddxEa);


    // AND
    //
    //       Syntax: (1) AND <ea>,Dy
    //               (2) AND Dx,<ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1100 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, AND, 0b101111111111, Byte | Word | Long, AndEaRg);

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1100 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, AND, 0b001111111000, Byte | Word | Long, AndRgEa);


    // ANDI
    //
    //       Syntax: ANDI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 0010 ---- ----");
    ________SSMMMXXX(opcode, ANDI, 0b100000000000, Byte | Word | Long, AndiRg);
    ________SSMMMXXX(opcode, ANDI, 0b001111111000, Byte | Word | Long, AndiEa);


    // ANDI to CCR
    //
    //       Syntax: ANDI #<data>,CCR
    //         Size: Byte
    //

    bind(parse("0000 0010 0011 1100"), Andiccr, ANDICCR, MODE_IM, Byte);


    // ANDI to SR
    //
    //       Syntax: ANDI #<data>,SR
    //         Size: Byte
    //

    bind(parse("0000 0010 0111 1100"), Andisr, ANDISR, MODE_IM, Word);


    // ASL, ASR
    //
    //       Syntax: (1) ASx Dx,Dy
    //               (2) ASx #<data>,Dy
    //               (3) ASx <ea>
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --10 0---");
    ____XXX_SS___XXX(opcode, ASL, MODE_DN,  Byte | Word | Long, ShiftRg);

    opcode = parse("1110 ---0 --10 0---");
    ____XXX_SS___XXX(opcode, ASR, MODE_DN,  Byte | Word | Long, ShiftRg);

    // #<data>,Dy
    opcode = parse("1110 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, ASL, MODE_IM, Byte | Word | Long, ShiftIm);

    opcode = parse("1110 ---0 --00 0---");
    ____XXX_SS___XXX(opcode, ASR, MODE_IM, Byte | Word | Long, ShiftIm);

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0001 11-- ----");
    __________MMMXXX(opcode, ASL, 0b001111111000, Word, ShiftEa);

    opcode = parse("1110 0000 11-- ----");
    __________MMMXXX(opcode, ASR, 0b001111111000, Word, ShiftEa);


    // Bcc
    //
    //       Syntax: Bcc <label>
    //         Size: Word

    // Dn,<label>
    opcode = parse("0110 ---- ---- ----");

    bind(opcode | 0x000, Bcc, BRA, MODE_IP, Word);
    bind(opcode | 0x200, Bcc, BHI, MODE_IP, Word);
    bind(opcode | 0x300, Bcc, BLS, MODE_IP, Word);
    bind(opcode | 0x400, Bcc, BCC, MODE_IP, Word);
    bind(opcode | 0x500, Bcc, BCS, MODE_IP, Word);
    bind(opcode | 0x600, Bcc, BNE, MODE_IP, Word);
    bind(opcode | 0x700, Bcc, BEQ, MODE_IP, Word);
    bind(opcode | 0x800, Bcc, BVC, MODE_IP, Word);
    bind(opcode | 0x900, Bcc, BVS, MODE_IP, Word);
    bind(opcode | 0xA00, Bcc, BPL, MODE_IP, Word);
    bind(opcode | 0xB00, Bcc, BMI, MODE_IP, Word);
    bind(opcode | 0xC00, Bcc, BGE, MODE_IP, Word);
    bind(opcode | 0xD00, Bcc, BLT, MODE_IP, Word);
    bind(opcode | 0xE00, Bcc, BGT, MODE_IP, Word);
    bind(opcode | 0xF00, Bcc, BLE, MODE_IP, Word);

    for (int i = 1; i <= 0xFF; i++) {
        bind(opcode | 0x000 | i, Bcc, BRA, MODE_IP, Byte);
        bind(opcode | 0x200 | i, Bcc, BHI, MODE_IP, Byte);
        bind(opcode | 0x300 | i, Bcc, BLS, MODE_IP, Byte);
        bind(opcode | 0x400 | i, Bcc, BCC, MODE_IP, Byte);
        bind(opcode | 0x500 | i, Bcc, BCS, MODE_IP, Byte);
        bind(opcode | 0x600 | i, Bcc, BNE, MODE_IP, Byte);
        bind(opcode | 0x700 | i, Bcc, BEQ, MODE_IP, Byte);
        bind(opcode | 0x800 | i, Bcc, BVC, MODE_IP, Byte);
        bind(opcode | 0x900 | i, Bcc, BVS, MODE_IP, Byte);
        bind(opcode | 0xA00 | i, Bcc, BPL, MODE_IP, Byte);
        bind(opcode | 0xB00 | i, Bcc, BMI, MODE_IP, Byte);
        bind(opcode | 0xC00 | i, Bcc, BGE, MODE_IP, Byte);
        bind(opcode | 0xD00 | i, Bcc, BLT, MODE_IP, Byte);
        bind(opcode | 0xE00 | i, Bcc, BGT, MODE_IP, Byte);
        bind(opcode | 0xF00 | i, Bcc, BLE, MODE_IP, Byte);
    }


    // BCHG, BCLR
    //
    //       Syntax: (1) BCxx Dn,<ea>
    //               (2) BCxx #<data>,<ea>
    //         Size: Byte, Longword

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 ---1 01-- ----");
    ____XXX___MMMXXX(opcode, BCHG, 0b101111111000, Long, BitDxEa);
    opcode = parse("0000 ---1 10-- ----");
    ____XXX___MMMXXX(opcode, BCLR, 0b101111111000, Long, BitDxEa);

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1000 01-- ----");
    __________MMMXXX(opcode, BCHG, 0b101111111000, Byte, BitImEa);
    opcode = parse("0000 1000 10-- ----");
    __________MMMXXX(opcode, BCLR, 0b101111111000, Byte, BitImEa);


    // BSET
    //
    //       Syntax: (1) BSET Dx,<ea>
    //               (2) BSET #<data>,<ea>
    //         Size: Byte, Longword

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, BSET, 0b101111111000, Long, BitDxEa);

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1000 11-- ----");
    __________MMMXXX(opcode, BSET, 0b101111111000, Byte, BitImEa);


    // BSR
    //
    //       Syntax: (1) BSR <label>
    //         Size: Byte, Word

    opcode = parse("0110 0001 ---- ----");
    bind(opcode, Bsr, BSR, MODE_IP, Word);
    for (int i = 1; i <= 0xFF; i++) {
        bind(opcode | i, Bsr, BSR, MODE_IP, Byte);
    }

    // BTST
    //
    //       Syntax: (1) BTST Dx,<ea>
    //               (2) BTST #<data>,<ea>
    //         Size: Byte, Longword

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X?

    opcode = parse("0000 ---1 00-- ----");
    ____XXX___MMMXXX(opcode, BTST, 0b101111111111, Byte, BitDxEa);

    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X

    opcode = parse("0000 1000 00-- ----");
    __________MMMXXX(opcode, BTST, 0b101111111110, Byte, BitImEa);


    // CHK
    //
    //       Syntax: CHK <ea>,Dy
    //         Size: Word

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X?

    opcode = parse("0100 ---1 10-- ----");
    ____XXX___MMMXXX(opcode, CHK, 0b101111111111, Word, Chk);


    // CLR
    //
    //       Syntax: CLR <ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 0010 ---- ----");
    ________SSMMMXXX(opcode, CLR, 0b101111111000, Byte | Word | Long, Clr);


    // CMP
    //
    //       Syntax: CMP <ea>,Dy
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X   X   X   X

    opcode = parse("1011 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, CMP, 0b101111111111, Byte,        Cmp);
    ____XXX_SSMMMXXX(opcode, CMP, 0b111111111111, Word | Long, Cmp);


    // CMPA
    //
    //       Syntax: CMPA <ea>,Ay
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("1011 ---- 11-- ----");
    ____XXXS__MMMXXX(opcode, CMPA, 0b111111111111, Word | Long, Cmpa);


    // CMPI
    //
    //       Syntax: CMPI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1100 ---- ----");
    ________SSMMMXXX(opcode, CMPI, 0b100000000000, Byte | Word | Long, CmpiRg);
    ________SSMMMXXX(opcode, CMPI, 0b001111111000, Byte | Word | Long, CmpiEa);


    // CMPM
    //
    //       Syntax: (1) CMPM (Ax)+,(Ay)+
    //         Size: Byte, Word, Longword

    // (Ax)+,(Ay)+
    opcode = parse("1011 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, CMPM, MODE_PI, Byte | Word | Long, Cmpm);


    // DBcc
    //
    //       Syntax: DBcc Dn,<label>
    //         Size: Word

    // Dn,<label>
    opcode = parse("0101 ---- 1100 1---");
    _____________XXX(opcode | 0x000, DBT,  MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x100, DBF,  MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x200, DBHI, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x300, DBLS, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x400, DBCC, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x500, DBCS, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x600, DBNE, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x700, DBEQ, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x800, DBVC, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0x900, DBVS, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0xA00, DBPL, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0xB00, DBMI, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0xC00, DBGE, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0xD00, DBLT, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0xE00, DBGT, MODE_IP, Word, Dbcc);
    _____________XXX(opcode | 0xF00, DBLE, MODE_IP, Word, Dbcc);


    // DIVS, DIVU
    //
    //       Syntax: DIVx <ea>,Dy
    //        Sizes: Longword, Word -> Longword

    //               -------------------------------------------------
    // <ea>,Dn       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1000 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, DIVS, 0b101111111111, Word, Div);

    opcode = parse("1000 ---0 11-- ----");
    ____XXX___MMMXXX(opcode, DIVU, 0b101111111111, Word, Div);


    // EOR
    //
    //       Syntax: EOR Dx,<ea>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dn       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("1011 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, EOR, 0b101111111000, Byte | Word | Long, AndRgEa);


    // EORI
    //
    //       Syntax: EORI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 1010 ---- ----");
    ________SSMMMXXX(opcode, EORI, 0b100000000000, Byte | Word | Long, AndiRg);
    ________SSMMMXXX(opcode, EORI, 0b001111111000, Byte | Word | Long, AndiEa);


    // EORI to CCR
    //
    //       Syntax: EORI #<data>,CCR
    //         Size: Byte
    //

    bind(parse("0000 1010 0011 1100"), Andiccr, EORICCR, MODE_IM, Byte);


    // EORI to SR
    //
    //       Syntax: EORI #<data>,SR
    //         Size: Byte
    //

    bind(parse("0000 1010 0111 1100"), Andisr, EORISR, MODE_IM, Word);


    // EXG
    //
    //       Syntax: EXG Dx,Dy
    //               EXG Ax,Dy
    //               EXG Ax,Ay
    //         Size: Longword

    opcode = parse("1100 ---1 0100 0---");
    ____XXX______XXX(opcode, EXG, MODE_IP, Long, ExgDxDy);

    opcode = parse("1100 ---1 1000 1---");
    ____XXX______XXX(opcode, EXG, MODE_IP, Long, ExgAxDy);

    opcode = parse("1100 ---1 0100 1---");
    ____XXX______XXX(opcode, EXG, MODE_IP, Long, ExgAxAy);


    // EXT
    //
    //       Syntax: EXT Dx
    //        Sizes: Word, Longword

    opcode = parse("0100 1000 --00 0---");
    _____________XXX(opcode | 2 << 6, EXT, MODE_DN, Word, Ext);
    _____________XXX(opcode | 3 << 6, EXT, MODE_DN, Long, Ext);


    // LINK
    //
    //       Syntax: LINK An,#<displacement>
    //        Sizes: Word

    opcode = parse("0100 1110 0101 0---");
    _____________XXX(opcode, LINK, MODE_IP, Word, Link);


    // JMP
    //
    //       Syntax: JMP <ea>
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

     opcode = parse("0100 1110 11-- ----");
     __________MMMXXX(opcode, JMP, 0b001001111110, Long, Jmp);


    // JSR
    //
    //       Syntax: JSR <ea>
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

     opcode = parse("0100 1110 10-- ----");
     __________MMMXXX(opcode, JSR, 0b001001111110, Long, Jsr);


    // LEA
    //
    //       Syntax: LEA <ea>,Ay
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    opcode = parse("0100 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, LEA, 0b001001111110, Long, Lea);


    // LSL, LSR
    //
    //       Syntax: (1) LSx Dx,Dy
    //               (2) LSx #<data>,Dy
    //               (3) LSx <ea>
    //        Sizes: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --10 1---");
     ____XXX_SS___XXX(opcode, LSL, MODE_DN, Byte | Word | Long, ShiftRg);

    opcode = parse("1110 ---0 --10 1---");
     ____XXX_SS___XXX(opcode, LSR, MODE_DN, Byte | Word | Long, ShiftRg);

    // #<data>,Dy
    opcode = parse("1110 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, LSL, MODE_IM, Byte | Word | Long, ShiftIm);

    opcode = parse("1110 ---0 --00 1---");
    ____XXX_SS___XXX(opcode, LSR, MODE_IM, Byte | Word | Long, ShiftIm);

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0011 11-- ----");
    __________MMMXXX(opcode, LSL, 0b001111111000, Word, ShiftEa);

    opcode = parse("1110 0010 11-- ----");
    __________MMMXXX(opcode, LSR, 0b001111111000, Word, ShiftEa);


    // MOVE
    //
    //       Syntax: MOVE <ea>,<e>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)   X   X   X   X   X   X   X   X   X   X

    //               -------------------------------------------------
    // <e>           | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    // <ea>,Dy
    opcode = parse("00-- ---0 00-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move0);
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move0);

    // <ea>,(Ay)
    opcode = parse("00-- ---0 10-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move2);
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move2);

    // <ea>,(Ay)+
    opcode = parse("00-- ---0 11-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move3);
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move3);

    // <ea>,-(Ay)
    opcode = parse("00-- ---1 00-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move4);
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move4);

    // <ea>,(d,Ay)
    opcode = parse("00-- ---1 01-- ----");
    __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move5);
    __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move5);

    // <ea>,(d,Ay,Xi)
     opcode = parse("00-- ---1 10-- ----");
     __SSXXX___MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move6);
     __SSXXX___MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move6);

    // <ea>,ABS.w
    opcode = parse("00-- 0001 11-- ----");
    __SS______MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move7);
    __SS______MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move7);

    // <ea>,ABS.l
    opcode = parse("00-- 0011 11-- ----");
    __SS______MMMXXX(opcode, MOVE, 0b101111111111, Byte,        Move8);
    __SS______MMMXXX(opcode, MOVE, 0b111111111111, Word | Long, Move8);


    // MOVEA
    //
    //       Syntax: MOVEA <ea>,Ay
    //        Sizes: Word, Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("001- ---0 01-- ----");

    ____XXX___MMMXXX(opcode | 0 << 12, MOVEA, 0b111111111111, Long, Movea)
    ____XXX___MMMXXX(opcode | 1 << 12, MOVEA, 0b111111111111, Word, Movea)


    // MOVEM
    //
    //       Syntax: MOVEM <ea>,<register list>
    //               MOVEM <register list>,<ea>
    //        Sizes: Word, Longword

    //               -------------------------------------------------
    // <ea>,<list>   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X       X   X   X   X   X   X

    //               -------------------------------------------------
    // <list>,<ea>   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X       X   X   X   X   X

    opcode = parse("0100 1100 1--- ----");
    __________MMMXXX(opcode | 0 << 6, MOVEM, 0b001101111110, Word, MovemEaRg);
    __________MMMXXX(opcode | 1 << 6, MOVEM, 0b001101111110, Long, MovemEaRg);

    opcode = parse("0100 1000 1--- ----");
    __________MMMXXX(opcode | 0 << 6, MOVEM, 0b001011111000, Word, MovemRgEa);
    __________MMMXXX(opcode | 1 << 6, MOVEM, 0b001011111000, Long, MovemRgEa);


    // MOVEP
    //
    //       Syntax: MOVEP Dx,(d,Ay)
    //               MOVEP (d,Ay),Dx
    //         Size: Word, Longword

    // MOVEP Dx,(d,Ay)
    opcode = parse("0000 ---1 1-00 1---");
    ____XXX______XXX(opcode | 0 << 6, MOVEP, MODE_DI, Word, MovepDxEa);
    ____XXX______XXX(opcode | 1 << 6, MOVEP, MODE_DI, Long, MovepDxEa);

    // MOVEP (d,Ay),Dx
    opcode = parse("0000 ---1 0-00 1---");
    ____XXX______XXX(opcode | 0 << 6, MOVEP, MODE_DI, Word, MovepEaDx);
    ____XXX______XXX(opcode | 1 << 6, MOVEP, MODE_DI, Long, MovepEaDx);


    // MOVEQ
    //
    //       Syntax: MOVEQ #<data>,Dn
    //        Sizes: Longword

    // #<data>,Dn
    opcode = parse("0111 ---0 ---- ----");
    ____XXX_XXXXXXXX(opcode, MOVEQ, MODE_IM, Long, Moveq);


    // MOVE to CCR
    //
    //       Syntax: MOVE <ea>,CCR
    //         Size: Word
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("0100 0100 11-- ----");
    __________MMMXXX(opcode, MOVETSR, 0b101111111111, Word, MoveToCcr);


    // MOVE from SR
    //
    //       Syntax: MOVE SR,<ea>
    //         Size: Word
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 0000 11-- ----");
    __________MMMXXX(opcode, MOVEFSR, 0b100000000000, Word, MoveFromSrRg);
    __________MMMXXX(opcode, MOVEFSR, 0b001111111000, Word, MoveFromSrEa);


    // MOVE to SR
    //
    //       Syntax: MOVE <ea>,SR
    //         Size: Word
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("0100 0110 11-- ----");
    __________MMMXXX(opcode, MOVETSR, 0b101111111111, Word, MoveToSr);


    // MOVEUSP
    //
    //       Syntax: MOVE USP,An
    //               MOVE An,USP
    //        Sizes: Longword

    opcode = parse("0100 1110 0110 ----");
    _____________XXX(opcode | 1 << 3, MOVEUSP, MODE_IP, Long, MoveUspAn);
    _____________XXX(opcode | 0 << 3, MOVEUSP, MODE_IP, Long, MoveAnUsp);


    // MULS, MULU
    //
    //       Syntax: MULx <ea>,Dy
    //        Sizes: Word x Word -> Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1100 ---1 11-- ----");
    ____XXX___MMMXXX(opcode, MULS, 0b101111111111, Word, Mul);

    opcode = parse("1100 ---0 11-- ----");
    ____XXX___MMMXXX(opcode, MULU, 0b101111111111, Word, Mul);


    // NBCD
    //
    //       Syntax: NBCD <ea>
    //        Sizes: Byte

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 1000 00-- ----");
    __________MMMXXX(opcode, NBCD, 0b101111111000, Byte, Nbcd);


    // NEG, NEGX, NOT
    //
    //       Syntax: Nxx <ea>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 0100 ---- ----");
    ________SSMMMXXX(opcode, NEG, 0b100000000000, Byte | Word | Long, NegRg);
    ________SSMMMXXX(opcode, NEG, 0b001111111000, Byte | Word | Long, NegEa);

    opcode = parse("0100 0000 ---- ----");
    ________SSMMMXXX(opcode, NEGX, 0b100000000000, Byte | Word | Long, NegRg);
    ________SSMMMXXX(opcode, NEGX, 0b001111111000, Byte | Word | Long, NegEa);

    opcode = parse("0100 0110 ---- ----");
    ________SSMMMXXX(opcode, NOT, 0b100000000000, Byte | Word | Long, NegRg);
    ________SSMMMXXX(opcode, NOT, 0b001111111000, Byte | Word | Long, NegEa);


    // NOP
    //
    //       Syntax: NOP
    //        Sizes: Unsized

    opcode = parse("0100 1110 0111 0001");
    bind(opcode, Nop, NOP, MODE_IP, Long);


    // OR
    //
    //       Syntax: OR <ea>,Dy
    //               OR Dx,<ea>
    //        Sizes: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

     opcode = parse("1000 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, OR, 0b101111111111, Byte | Word | Long, AndEaRg);

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X   X   X   X

    opcode = parse("1000 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, OR, 0b001111111000, Byte | Word | Long, AndRgEa);


    // ORI
    //
    //       Syntax: ORI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 0000 ---- ----");
    ________SSMMMXXX(opcode, ORI, 0b100000000000, Byte | Word | Long, AndiRg);
    ________SSMMMXXX(opcode, ORI, 0b001111111000, Byte | Word | Long, AndiEa);


    // ORI to CCR
    //
    //       Syntax: ORI #<data>,CCR
    //         Size: Byte
    //

    bind(parse("0000 0000 0011 1100"), Andiccr, ORICCR, MODE_IM, Byte);


    // ORI to SR
    //
    //       Syntax: ORI #<data>,SR
    //         Size: Byte
    //

    bind(parse("0000 0000 0111 1100"), Andisr, ORISR, MODE_IM, Word);


    // ROL, ROR, ROXL, ROXR
    //
    //       Syntax: (1) ROxx Dx,Dy
    //               (2) ROxx #<data>,Dy
    //               (3) ROxx <ea>
    //        Sizes: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1110 ---1 --11 1---");
    ____XXX_SS___XXX(opcode, ROL, MODE_DN, Byte | Word | Long, ShiftRg);

    opcode = parse("1110 ---0 --11 1---");
    ____XXX_SS___XXX(opcode, ROR, MODE_DN, Byte | Word | Long, ShiftRg);

    opcode = parse("1110 ---1 --11 0---");
    ____XXX_SS___XXX(opcode, ROXL, MODE_DN, Byte | Word | Long, ShiftRg);

    opcode = parse("1110 ---0 --11 0---");
    ____XXX_SS___XXX(opcode, ROXR, MODE_DN, Byte | Word | Long, ShiftRg);

    // #<data>,Dy
    opcode = parse("1110 ---1 --01 1---");
    ____XXX_SS___XXX(opcode, ROL, MODE_IM, Byte | Word | Long, ShiftIm);

    opcode = parse("1110 ---0 --01 1---");
    ____XXX_SS___XXX(opcode, ROR, MODE_IM, Byte | Word | Long, ShiftIm);

    opcode = parse("1110 ---1 --01 0---");
    ____XXX_SS___XXX(opcode, ROXL, MODE_IM, Byte | Word | Long, ShiftIm);

    opcode = parse("1110 ---0 --01 0---");
    ____XXX_SS___XXX(opcode, ROXR, MODE_IM, Byte | Word | Long, ShiftIm);

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1110 0111 11-- ----");
    __________MMMXXX(opcode, ROL, 0b001111111000, Word, ShiftEa);

    opcode = parse("1110 0110 11-- ----");
    __________MMMXXX(opcode, ROR, 0b001111111000, Word, ShiftEa);

    opcode = parse("1110 0101 11-- ----");
    __________MMMXXX(opcode, ROXL, 0b001111111000, Word, ShiftEa);

    opcode = parse("1110 0100 11-- ----");
    __________MMMXXX(opcode, ROXR, 0b001111111000, Word, ShiftEa);


    // PEA
    //
    //       Syntax: PEA <ea>,Ay
    //        Sizes: Longword

    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X           X   X   X   X   X   X

    opcode = parse("0100 1000 01-- ----");
    __________MMMXXX(opcode, PEA, 0b001001111110, Long, Pea);


    // RESET
    //
    //       Syntax: RESET
    //        Sizes: Unsized

    bind(parse("0100 1110 0111 0000"), Reset, RESET, MODE_IP, Long);


    // RTE
    //
    //       Syntax: RTE
    //        Sizes: Unsized

    bind(parse("0100 1110 0111 0011"), Rte, RTE, MODE_IP, Long);


    // RTR
    //
    //       Syntax: RTR
    //        Sizes: Unsized

    bind(parse("0100 1110 0111 0111"), Rtr, RTR, MODE_IP, Long);


    // RTS
    //
    //       Syntax: RTS
    //        Sizes: Unsized

    bind(parse("0100 1110 0111 0101"), Rts, RTS, MODE_IP, Long);


    // SBCD
    //
    //       Syntax: (1) SBCD Dx,Dy
    //               (2) SBCD -(Ax),-(Ay)
    //         Size: Byte

    // Dx,Dy
    opcode = parse("1000 ---1 0000 0---");
    ____XXX______XXX(opcode, SBCD, MODE_DN, Byte, Abcd);

    // -(Ax),-(Ay)
    opcode = parse("1000 ---1 0000 1---");
    ____XXX______XXX(opcode, SBCD, MODE_PD, Byte, Abcd);


    // Scc
    //
    //       Syntax: Scc <ea>
    //         Size: Word

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0101 ---- 11-- ----");
    __________MMMXXX(opcode | 0x000, ST,  0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x100, SF,  0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x200, SHI, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x300, SLS, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x400, SCC, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x500, SCS, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x600, SNE, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x700, SEQ, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x800, SVC, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0x900, SVS, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0xA00, SPL, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0xB00, SMI, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0xC00, SGE, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0xD00, SLT, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0xE00, SGT, 0b100000000000, Word, SccRg);
    __________MMMXXX(opcode | 0xF00, SLE, 0b100000000000, Word, SccRg);

    __________MMMXXX(opcode | 0x000, ST,  0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x100, SF,  0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x200, SHI, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x300, SLS, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x400, SCC, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x500, SCS, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x600, SNE, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x700, SEQ, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x800, SVC, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0x900, SVS, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0xA00, SPL, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0xB00, SMI, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0xC00, SGE, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0xD00, SLT, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0xE00, SGT, 0b001111111000, Word, SccEa);
    __________MMMXXX(opcode | 0xF00, SLE, 0b001111111000, Word, SccEa);

    // STOP
    //
    //       Syntax: STOP
    //        Sizes: Unsized

    bind(parse("0100 1110 0111 0010"), Stop, STOP, MODE_IP, Word);


    // SUB
    //
    //       Syntax: (1) SUB <ea>,Dy
    //               (2) SUB Dx,<ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>,Dy       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X   X   X   X

    opcode = parse("1001 ---0 ---- ----");
    ____XXX_SSMMMXXX(opcode, SUB, 0b101111111111, Byte,        AddEaRg);
    ____XXX_SSMMMXXX(opcode, SUB, 0b111111111111, Word | Long, AddEaRg);

    //               -------------------------------------------------
    // Dx,<ea>       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                         X   X   X   X   X   X   X

    opcode = parse("1001 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, SUB, 0b001111111000, Byte | Word | Long, AddRgEa);


    // SUBA
    //
    //       Syntax: SUBA <ea>,Ay
    //         Size: Word, Longword
    //
    //               -------------------------------------------------
    // <ea>,Ay       | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X   X   X   X   X   X   X   X   X   X   X   X

    opcode = parse("1001 ---- 11-- ----");
    ____XXXS__MMMXXX(opcode, SUBA, 0b111111111111, Word | Long, Adda)


    // SUBI
    //
    //       Syntax: SUBI #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0000 0100 ---- ----");
    ________SSMMMXXX(opcode, SUBI, 0b100000000000, Byte | Word | Long, AddiRg);
    ________SSMMMXXX(opcode, SUBI, 0b001111111000, Byte | Word | Long, AddiEa);


    // SUBQ
    //
    //       Syntax: SUBQ #<data>,<ea>
    //         Size: Byte, Word, Longword
    //
    //               -------------------------------------------------
    // #<data>,<ea>  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X  (X)  X   X   X   X   X   X   X

    opcode = parse("0101 ---1 ---- ----");
    ____XXX_SSMMMXXX(opcode, SUBQ, 0b100000000000, Byte | Word | Long, AddqDn);
    ____XXX_SSMMMXXX(opcode, SUBQ, 0b010000000000, Word | Long,        AddqAn);
    ____XXX_SSMMMXXX(opcode, SUBQ, 0b001111111000, Byte | Word | Long, AddqEa);


    // SUBX
    //
    //       Syntax: (1) SUBX Dx,Dy
    //               (2) SUBX -(Ax),-(Ay)
    //         Size: Byte, Word, Longword

    // Dx,Dy
    opcode = parse("1001 ---1 --00 0---");
    ____XXX_SS___XXX(opcode, SUBX, MODE_DN, Byte | Word | Long, AddxRg);

    // -(Ax),-(Ay)
    opcode = parse("1001 ---1 --00 1---");
    ____XXX_SS___XXX(opcode, SUBX, MODE_PD, Byte | Word | Long, AddxEa);


    // SWAP
    //
    //       Syntax: SWAP Dn
    //         Size: Word

    opcode = parse("0100 1000 0100 0---");
    _____________XXX(opcode, SWAP, MODE_DN, Word, Swap);


    // TAS
    //
    //       Syntax: TAS <ea>
    //         Size: Byte

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 1010 11-- ----");
    __________MMMXXX(opcode, TAS, 0b100000000000, Byte, TasRg);
    __________MMMXXX(opcode, TAS, 0b001111111000, Byte, TasEa);


    // TRAP
    //
    //       Syntax: TRAP #<vector>
    //        Sizes: Unsized

    opcode = parse("0100 1110 0100 ----");
    ____________XXXX(opcode, TRAP, MODE_IP, Long, Trap);


    // TRAPV
    //
    //       Syntax: TRAPV
    //        Sizes: Unsized

    bind(parse("0100 1110 0111 0110"), Trapv, TRAPV, MODE_IP, Long);


    // TST
    //
    //       Syntax: TST <ea>
    //         Size: Byte, Word, Longword

    //               -------------------------------------------------
    // <ea>          | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | A | B |
    //               -------------------------------------------------
    //                 X       X   X   X   X   X   X   X

    opcode = parse("0100 1010 ---- ----");
    ________SSMMMXXX(opcode, TST, 0b101111111000, Byte | Word | Long, Tst);


    // UNLK
    //
    //       Syntax: UNLK An
    //        Sizes: Word

    opcode = parse("0100 1110 0101 1---");
    _____________XXX(opcode, UNLK, MODE_IP, Word, Unlk);
}
