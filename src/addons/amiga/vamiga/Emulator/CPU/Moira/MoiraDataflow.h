// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

/* Moira utilizes a three layer design to model dataflow inside the processor.
 *
 * Layer 1: Main entry points. Most instruction execution handlers call a
 *          layer 1 function to read or write their operands.
 *
 * Layer 2: Data providers. This layer provides functions for moving data back
 *          and forth between various locations.
 *
 * Layer 3: Memory interface. The functions from this layer perform the actual
 *          memory access.
 *
 * The following picture depicts the interplay between the different layers:
 *
 *                                      |
 * - - - - - - - - - - - - - - - - - - -|- - - - - - - - - - - - - - - - - - - -
 * Layer 1:                             V
 *                                    readOp
 *                                   (writeOp)
 *                                      |
 *  Addressing Mode M = 0---1---2---3---4---5---6---7---8---9---A---B
 *                     /    |   |   |   |   |   |   |   |   |   |    \
 *                    /     |   ---------------------------------     \
 *                   /      |           |                              \
 *                  /       |           V                               \
 *                 /        |       computeEA -----------                \
 *                /         |           |               |                 \
 * - - - - - - - -|- - - - -|- - - - - -|- - - - - - - -|- - - - - - - - -|- - -
 * Layer 2:       |         |           |               V                 |
 *                |         |           |            readExt              |
 *                V         V           V                                 V
 *              readD     readA      readM -----> addressError          readI
 *             (writeD)  (writeA)   (writeM)
 *                                      |
 *                                  updateAn()
 *                                      |
 * - - - - - - - - - - - - - - - - - - -|- - - - - - - - - - - - - - - - - - - -
 * Layer 3:                             |
 *                 Size S = B-----------W-----------L
 *                          |           |           |
 *                          V           V           V
 *                        read8       read16    2 x read16
 *                       (write8)    (write8)  (2 x write16)
 *
 */

/* Reads an operand
 *
 * The source of the operand is determined by the addressing mode M. If the
 * operand is read from memory, the effective address is stored in variable ea.
 * If the source is a register or an immediate value, variable ea remains
 * untouched.
 */
template<Mode M, Size S, Flags F = 0> bool readOp(int n, u32 &ea, u32 &result);

/* Writes an operand
 *
 * If parameter ea is omitted, the destination of the operand is determined
 * by the addressing mode M. Parameter 'last' indicates if this function is
 * initiates the last memory bus cycle of an instruction.
 */
template<Mode M, Size S, Flags F = 0> bool writeOp(int n, u32 val);
template<Mode M, Size S, Flags F = 0> void writeOp(int n, u32 ea, u32 val);

// Computes an effective address
template<Mode M, Size S, Flags F = 0> u32 computeEA(u32 n);

// Emulates the address register modification for modes (An)+, (An)-
template<Mode M, Size S> void updateAn(int n);
template<Mode M, Size S> void updateAnPD(int n);
template<Mode M, Size S> void undoAnPD(int n);
template<Mode M, Size S> void updateAnPI(int n);

// Reads a value from a specific memory space
template<MemSpace M, Size S, Flags F = 0> u32 readM(u32 addr);
template<MemSpace M, Size S, Flags F = 0> u32 readM(u32 addr, bool &error);

// Reads a value from program or data space, depending on the addressing mode
template<Mode M, Size S, Flags F = 0> u32 readM(u32 addr);
template<Mode M, Size S, Flags F = 0> u32 readM(u32 addr, bool &error);

// Writes a value to a specific memory space
template<MemSpace M, Size S, Flags F = 0> void writeM(u32 addr, u32 val);
template<MemSpace M, Size S, Flags F = 0> void writeM(u32 addr, u32 val, bool &error);

// Writes an operand to memory (without or with address error checking)
template<Mode M, Size S, Flags F = 0> void writeM(u32 addr, u32 val);
template<Mode M, Size S, Flags F = 0> void writeM(u32 addr, u32 val, bool &error);

// Reads an immediate value from memory
template<Size S> u32 readI();

// Pushes a value onto the stack
template<Size S, Flags F = 0> void push(u32 value);
template<Size S, Flags F = 0> void push(u32 value, bool &error);

// Checks whether the provided address should trigger an address error
template<Size S = Word> bool misaligned(u32 addr);

// Creates an address error stack frame
template<Flags F = 0> AEStackFrame makeFrame(u32 addr, u32 pc, u16 sr, u16 ird);
template<Flags F = 0> AEStackFrame makeFrame(u32 addr, u32 pc);
template<Flags F = 0> AEStackFrame makeFrame(u32 addr);

// Prefetches the next instruction
template<Flags F = 0> void prefetch();

// Performs a full prefetch cycle
template<Flags F = 0, int delay = 0> void fullPrefetch();

// Reads an extension word from memory
void readExt();

// Jumps to an exception vector
template<Flags F = 0> void jumpToVector(int nr);
