// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "config.h"

/* Set to true to enable address error checking.
 *
 * The Motorola 68k signals an address error violation if a odd memory location
 * is addressed in combination with word or long word addressing.
 *
 * Enable to improve emulation compatibility, disable to gain speed.
 */
#define EMULATE_ADDRESS_ERROR true

/* Set to true to emulate the function code pins FC0 - FC2.
 *
 * Whenever memory is accessed, the function code pins enable external hardware
 * to inspect the access type. If used, these pins are usually connected to an
 * external memory management unit (MMU).
 *
 * Enable to improve emulation compatibility, disable to gain speed.
 */
#define EMULATE_FC true

/* Set to true to enable the disassembler.
 *
 * The disassembler requires a jump table which consumes about 1MB of memory.
 * By disabling the disassembler, you can save this amount of memory.
 */
#define ENABLE_DASM true

/* Set to true to build the InstrInfo lookup table.
 *
 * The info table stores information about the instruction (Instr I), the
 * addressing mode (Mode M), and the size attribute (Size S) for all 65536
 * instruction words. The table is meant to provide data for, e.g., external
 * debuggers. It is not needed by Moira itself and therefore disabled by
 * default.
 */
#define BUILD_INSTR_INFO_TABLE false

/* Set to true to run Moira in a special Musashi compatibility mode.
 *
 * The compatibility mode is used by the test runner application to compare
 * the results computed by Moira and Musashi, respectively.
 *
 * Disable to improve emulation compatibility.
 */
#define MIMIC_MUSASHI false
