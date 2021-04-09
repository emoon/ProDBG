// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include <stdint.h>

namespace moira {

typedef int8_t             i8;
typedef int16_t            i16;
typedef int32_t            i32;
typedef long long          i64;
typedef uint8_t            u8;
typedef uint16_t           u16;
typedef uint32_t           u32;
typedef unsigned long long u64;

typedef enum
{
    M68000     // We only support the 68000 yet
}
CPUModel;

typedef enum
{
    ILLEGAL,   // Illegal instruction
    LINE_A,    // Unused instruction (line A)
    LINE_F,    // Unused instruction (line F)

    ABCD,      // Add decimal with extend
    ADD,       // Add binary
    ADDA,      // Add address
    ADDI,      // Add immediate
    ADDQ,      // Add quick
    ADDX,      // Add extended
    AND,       // AND logical
    ANDI,      // AND immediate
    ANDICCR,   // AND immediate to condition code register
    ANDISR,    // AND immediate to status register
    ASL,       // Arithmetic shift left
    ASR,       // Arithmetic shift right
    BCC,       // Branch on carry clear
    BCS,       // Branch on carry set
    BEQ,       // Branch on equal
    BGE,       // Branch on greater than or equal
    BGT,       // Branch on greater than
    BHI,       // Branch on higher than
    BLE,       // Branch on less than or equal
    BLS,       // Branch on lower than or same
    BLT,       // Branch on less than
    BMI,       // Branch on minus
    BNE,       // Branch on not equal
    BPL,       // Branch on plus
    BVC,       // Branch on overflow clear
    BVS,       // Branch on overflow set
    BCHG,      // Test a bit and change
    BCLR,      // Test a bit and clear
    BRA,       // Branch always
    BSET,      // Test a bit and set
    BSR,       // Branch to subroutine
    BTST,      // Test a bit
    CHK,       // Check register against bounds
    CLR,       // Clear an operand
    CMP,       // Compare
    CMPA,      // Compare address
    CMPI,      // Compare immediate
    CMPM,      // Compare memory with memory
    DBCC,      // Test, decrement, and branch on carry clear
    DBCS,      // Test, decrement, and branch on carry set
    DBEQ,      // Test, decrement, and branch on equal
    DBGE,      // Test, decrement, and branch on greater than or equal
    DBGT,      // Test, decrement, and branch on greater than
    DBHI,      // Test, decrement, and branch on higher than
    DBLE,      // Test, decrement, and branch on less than or equal
    DBLS,      // Test, decrement, and branch on lower than or same
    DBLT,      // Test, decrement, and branch on less than
    DBMI,      // Test, decrement, and branch on minus
    DBNE,      // Test, decrement, and branch on not equal
    DBPL,      // Test, decrement, and branch on on plus
    DBVC,      // Test, decrement, and branch on overflow clear
    DBVS,      // Test, decrement, and branch on overflow set
    DBF,       // Test, decrement, and branch on false (never)
    DBT,       // Test, decrement, and branch on true (always)
    DIVS,      // Signed divide
    DIVU,      // Unsigned divide
    EOR,       // Exclusive OR logical
    EORI,      // Exclusive OR immediate
    EORICCR,   // Exclusive OR immediate to condition code register
    EORISR,    // Exclusive OR immediate to status register
    EXG,       // Exchange registers
    EXT,       // Sign-extend a data register
    JMP,       // Jump
    JSR,       // Jump to subroutine
    LEA,       // Load effective address
    LINK,      // Link and allocate
    LSL,       // Logical shift left
    LSR,       // Logical shift right
    MOVE,      // Copy data from source to destination
    MOVEA,     // Move address
    MOVECCR,   // Copy data to condition code register from source
    MOVEFSR,   // Copy data from status register to destination
    MOVETSR,   // Copy data to status register from source
    MOVEUSP,   // Copy data to or from USP
    MOVEM,     // Move multiple registers
    MOVEP,     // Move peripheral data
    MOVEQ,     // Move quick
    MULS,      // Signed multiply
    MULU,      // Unsigned multiply
    NBCD,      // Negate decimal with sign extend
    NEG,       // Negate
    NEGX,      // Negate with extend
    NOP,       // No operation
    NOT,       // Logical complement
    OR,        // OR logical
    ORI,       // OR immediate
    ORICCR,    // OR immediate to condition code register
    ORISR,     // OR immediate to status register
    PEA,       // Push effective address
    RESET,     // Reset external devices
    ROL,       // Rotate left
    ROR,       // Rotate right
    ROXL,      // Rotate left with extend
    ROXR,      // Rotate righ with extend
    RTE,       // Return from exception
    RTR,       // Return and restore condition codes
    RTS,       // Return from subroutine
    SBCD,      // Subtract decimal with extend
    SCC,       // Set on carry clear
    SCS,       // Set on carry set
    SEQ,       // Set on equal
    SGE,       // Set on greater than or equal
    SGT,       // Set on greater than
    SHI,       // Set on higher than
    SLE,       // Set on less than or equal
    SLS,       // Set on lower than or same
    SLT,       // Set on less than
    SMI,       // Set on minus
    SNE,       // Set on not equal
    SPL,       // Set on plus
    SVC,       // Set on overflow clear
    SVS,       // Set on overflow set
    SF,        // Set on false (never set)
    ST,        // Set on true (always set)
    STOP,      // Load status register and stop
    SUB,       // Subtract binary
    SUBA,      // Subtract address
    SUBI,      // Subtract immediate
    SUBQ,      // Subtract quick
    SUBX,      // Subtract extended
    SWAP,      // Swap register halves
    TAS,       // Test and set an operand
    TRAP,      // Trap
    TRAPV,     // Trap on overflow
    TST,       // Test an operand
    UNLK       // Unlink
}
Instr;

typedef enum
{
    MODE_DN,   //  0         Dn : Data register direct
    MODE_AN,   //  1         An : Address register direct
    MODE_AI,   //  2       (An) : Register indirect
    MODE_PI,   //  3      (An)+ : Postincrement register indirect
    MODE_PD,   //  4      -(An) : Predecrement register indirect
    MODE_DI,   //  5     (d,An) : Register indirect with displacement
    MODE_IX,   //  6  (d,An,Xi) : Indexed register indirect with displacement
    MODE_AW,   //  7   (####).w : Absolute addressing short
    MODE_AL,   //  8   (####).l : Absolute addressing long
    MODE_DIPC, //  9     (d,PC) : PC relative with displacement
    MODE_IXPC, // 10  (d,PC,Xi) : Indexed PC relative with displacement
    MODE_IM,   // 11       #### : Immediate data addressing
    MODE_IP    // 12       ---- : Implied addressing
}
Mode;

inline bool isRegMode(Mode M) { return M == 0 || M == 1;  }
inline bool isAbsMode(Mode M) { return M == 7 || M == 8;  }
inline bool isIdxMode(Mode M) { return M == 6 || M == 10; }
inline bool isMemMode(Mode M) { return M >= 2 && M <= 10; }
inline bool isPrgMode(Mode M) { return M == 9 || M == 10; }
inline bool isDspMode(Mode M) { return M == 5 || M == 6 || M == 9 || M == 10; }
inline bool isImmMode(Mode M) { return M == 11; }

typedef enum
{
    Byte = 1,  // .b : Byte addressing
    Word = 2,  // .w : Word addressing
    Long = 4   // .l : Long word addressing
}
Size;

typedef struct
{
    Instr I;
    Mode  M;
    Size  S;
}
InstrInfo;

typedef enum
{
    IRQ_AUTO,
    IRQ_USER,
    IRQ_SPURIOUS,
    IRQ_UNINITIALIZED
}
IrqMode;

typedef enum
{
    FC_USER_DATA       = 1,
    FC_USER_PROG       = 2,
    FC_SUPERVISOR_DATA = 5,
    FC_SUPERVISOR_PROG = 6
}
FunctionCode;

typedef enum
{
    MEM_DATA = 1,
    MEM_PROG = 2
}
MemSpace;

typedef struct
{
    u16 code;
    u32 addr;
    u16 ird;
    u16 sr;
    u32 pc;
}
AEStackFrame;

struct StatusRegister {

    bool t;               // Trace flag
    bool s;               // Supervisor flag
    bool x;               // Extend flag
    bool n;               // Negative flag
    bool z;               // Zero flag
    bool v;               // Overflow flag
    bool c;               // Carry flag

    u8 ipl;               // Required Interrupt Priority Level
};

struct Registers {

    u32 pc;               // Program counter
    u32 pc0;              // Beginning of the currently executed instruction
    StatusRegister sr;    // Status register

    union {
        struct {
            u32 d[8];     // D0, D1 ... D7
            u32 a[8];     // A0, A1 ... A7
        };
        struct {
            u32 r[16];    // D0, D1 ... D7, A0, A1 ... A7
        };
        struct {
            u32 _pad[15];
            u32 sp;       // Visible stack pointer (overlays a[7])
        };
    };

    u32 usp;              // User Stack Pointer
    u32 ssp;              // Supervisor Stack Pointer

    u8 ipl;               // Polled Interrupt Priority Level
};

struct PrefetchQueue {    // http://pasti.fxatari.com/68kdocs/68kPrefetch.html

    u16 irc;              // The most recent word prefetched from memory
    u16 ird;              // The instruction currently being executed
};

/* Execution flags
 *
 * The Motorola 68000 is a well organized processor that utilizes the same
 * general execution scheme for many instructions. However, the schemes
 * slighty differ between instruction. To take care of the subtle differences,
 * some execution functions take an dditional 'flags' argument which allows to
 * alter their behavior. All flags are passed as a template parameter for
 * efficiency.
 */
 
typedef u64 Flags;

// Memory access flags
static const u64 REVERSE        (1 << 0);  // Reverse the long word access order
static const u64 SKIP_LAST_READ (1 << 1);  // Reverse the long word access order

// Interrupt flags
static const u64 POLLIPL        (1 << 2);  // Poll the interrupt lines
                           
// Address error stack frame flags
static const u64 AE_WRITE       (1 << 3);  // Clear read flag in code word
static const u64 AE_PROG        (1 << 4);  // Set FC pins to program space
static const u64 AE_DATA        (1 << 5);  // Set FC pins to user space
static const u64 AE_INC_PC      (1 << 6);  // Increment PC by 2 in stack frame
static const u64 AE_DEC_PC      (1 << 7);  // Decrement PC by 2 in stack frame
static const u64 AE_INC_ADDR    (1 << 8);  // Increment ADDR by 2 in stack frame
static const u64 AE_DEC_ADDR    (1 << 9);  // Decrement ADDR by 2 in stack frame
static const u64 AE_SET_CB3     (1 << 10); // Set bit 3 in CODE segment

}
