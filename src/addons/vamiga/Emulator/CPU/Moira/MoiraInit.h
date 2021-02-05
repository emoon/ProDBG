// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* This file declares all instruction handlers. All handlers follow a common
 * naming scheme:
 *
 *    execXXX : Handler for executing an instruction
 *    dasmXXX : Handler for disassembling an instruction
 */

#define MOIRA_DECLARE_SIMPLE(x) \
void dasm##x(StrWriter &str, u32 &addr, u16 op); \
void exec##x(u16 op);

#define MOIRA_DECLARE(x) \
template<Instr I, Mode M, Size S> void dasm##x(StrWriter &str, u32 &addr, u16 op); \
template<Instr I, Mode M, Size S> void exec##x(u16 op);

MOIRA_DECLARE_SIMPLE(LineA)
MOIRA_DECLARE_SIMPLE(LineF)
MOIRA_DECLARE_SIMPLE(Illegal)

MOIRA_DECLARE(ShiftRg)
MOIRA_DECLARE(ShiftIm)
MOIRA_DECLARE(ShiftEa)

MOIRA_DECLARE(Abcd)
MOIRA_DECLARE(AddEaRg)
MOIRA_DECLARE(AddRgEa)
MOIRA_DECLARE(Adda)
MOIRA_DECLARE(AddiRg)
MOIRA_DECLARE(AddiEa)
MOIRA_DECLARE(AddqDn)
MOIRA_DECLARE(AddqAn)
MOIRA_DECLARE(AddqEa)
MOIRA_DECLARE(AddxRg)
MOIRA_DECLARE(AddxEa)
MOIRA_DECLARE(AndEaRg)
MOIRA_DECLARE(AndRgEa)
MOIRA_DECLARE(AndiRg)
MOIRA_DECLARE(AndiEa)
MOIRA_DECLARE(Andiccr)
MOIRA_DECLARE(Andisr)

MOIRA_DECLARE(Bcc)
MOIRA_DECLARE(BitDxEa)
MOIRA_DECLARE(BitImEa)
MOIRA_DECLARE(Bsr)

MOIRA_DECLARE(Chk)
MOIRA_DECLARE(Clr)
MOIRA_DECLARE(Cmp)
MOIRA_DECLARE(Cmpa)
MOIRA_DECLARE(CmpiRg)
MOIRA_DECLARE(CmpiEa)
MOIRA_DECLARE(Cmpm)

MOIRA_DECLARE(Dbcc)

MOIRA_DECLARE(ExgDxDy)
MOIRA_DECLARE(ExgAxDy)
MOIRA_DECLARE(ExgAxAy)
MOIRA_DECLARE(Ext)

MOIRA_DECLARE(Jmp)
MOIRA_DECLARE(Jsr)

MOIRA_DECLARE(Lea)
MOIRA_DECLARE(Link)

MOIRA_DECLARE(Move0)
MOIRA_DECLARE(Move2)
MOIRA_DECLARE(Move3)
MOIRA_DECLARE(Move4)
MOIRA_DECLARE(Move5)
MOIRA_DECLARE(Move6)
MOIRA_DECLARE(Move7)
MOIRA_DECLARE(Move8)
MOIRA_DECLARE(Movea)
MOIRA_DECLARE(MovemEaRg)
MOIRA_DECLARE(MovemRgEa)
MOIRA_DECLARE(MovepDxEa)
MOIRA_DECLARE(MovepEaDx)
MOIRA_DECLARE(Moveq)
MOIRA_DECLARE(MoveToCcr)
MOIRA_DECLARE(MoveFromSrRg)
MOIRA_DECLARE(MoveFromSrEa)
MOIRA_DECLARE(MoveToSr)
MOIRA_DECLARE(MoveUspAn)
MOIRA_DECLARE(MoveAnUsp)
MOIRA_DECLARE(Mul)
MOIRA_DECLARE(Div)

MOIRA_DECLARE(Nbcd)
MOIRA_DECLARE(NegRg)
MOIRA_DECLARE(NegEa)
MOIRA_DECLARE(Nop)

MOIRA_DECLARE(Pea)

MOIRA_DECLARE(Reset)
MOIRA_DECLARE(Rte)
MOIRA_DECLARE(Rtr)
MOIRA_DECLARE(Rts)

MOIRA_DECLARE(SccRg)
MOIRA_DECLARE(SccEa)
MOIRA_DECLARE(Stop)
MOIRA_DECLARE(Swap)

MOIRA_DECLARE(TasRg)
MOIRA_DECLARE(TasEa)
MOIRA_DECLARE(Trap)
MOIRA_DECLARE(Trapv)
MOIRA_DECLARE(Tst)

MOIRA_DECLARE(Unlk)

// Musashi compatibility mode
template<Instr I, Mode M, Size S> void execMulMusashi(u16 op);
template<Instr I, Mode M, Size S> void execDivMusashi(u16 op);
