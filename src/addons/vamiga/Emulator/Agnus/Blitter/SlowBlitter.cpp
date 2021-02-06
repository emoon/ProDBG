// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

/* Micro-instructions:
 *
 * To keep the implementation flexible, the SlowBlitter is emulated as a
 * micro-programmable device. When a blit is processed, a micro-program is
 * executed that decide on the actions that are performed in a certain Blitter
 * cycle.
 *
 * A micro-program consists of the following micro-instructions:
 *
 *     NOTHING : No action is taken
 *     BUSIDLE : Waits for the bus to be free
 *         BUS : Waits for the bus to be free and allocates it
 *     WRITE_D : Writes back register D hold
 *     FETCH_A : Loads register A new
 *     FETCH_B : Loads register B new
 *     FETCH_C : Loads register C hold
 *      HOLD_A : Loads register A hold
 *      HOLD_B : Loads register B hold
 *      HOLD_D : Loads register D hold
 *        FILL : Run the fill circuitry
 *     BLTDONE : Marks the last instruction and terminates the Blitter
 *      REPEAT : Performs a conditional jump back to instruction 0
 */

static const u16 NOTHING   = 0b0000'0000'0000'0000;
static const u16 BUSIDLE   = 0b0000'0000'0000'0001;
static const u16 BUS       = 0b0000'0000'0000'0010;
static const u16 WRITE_D   = 0b0000'0000'0000'0100;
static const u16 FETCH_A   = 0b0000'0000'0000'1000;
static const u16 FETCH_B   = 0b0000'0000'0001'0000;
static const u16 FETCH_C   = 0b0000'0000'0010'0000;
static const u16 HOLD_A    = 0b0000'0000'0100'0000;
static const u16 HOLD_B    = 0b0000'0000'1000'0000;
static const u16 HOLD_D    = 0b0000'0001'0000'0000;
static const u16 FILL      = 0b0000'0010'0000'0000;
static const u16 BLTDONE   = 0b0000'0100'0000'0000;
static const u16 REPEAT    = 0b0000'1000'0000'0000;
static const u16 FETCH     = FETCH_A | FETCH_B | FETCH_C;

void
Blitter::initSlowBlitter()
{
    /* Micro programs
     *
     * The Copy Blitter micro programs are stored in array
     *
     *   copyBlitInstr[16][2][2][6]
     *
     * For each program, four different versions are stored:
     *
     *   [][0][0][] : Performs a Copy Blit in accuracy level 2
     *   [][0][1][] : Performs a Fill Copy Blit in accuracy level 2
     *   [][1][0][] : Performs a Copy Blit in accuracy level 1
     *   [][1][1][] : Performs a Fill Copy Blit in accuracy level 1
     *
     * Level 2 microprograms operate the bus and all Blitter components.
     * Level 1 microprograms are a stripped down version that operates
     * the bus only. This is what we call "fake execution", because the
     * blit itself has already been carried out by the Fast Blitter.
     *
     * The programs below have been derived from Table 6.2 of the HRM.
     * The published table doesn't seem to be 100% accurate. See the
     * microprograms below for applied modifications.
     *
     *           Active
     * BLTCON0  Channels            Cycle sequence
     *    F     A B C D    A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 D2
     *    E     A B C      A0 B0 C0 A1 B1 C1 A2 B2 C2
     *    D     A B   D    A0 B0 -- A1 B1 D0 A2 B2 D1 -- D2
     *    C     A B        A0 B0 -- A1 B1 -- A2 B2
     *    B     A   C D    A0 C0 -- A1 C1 D0 A2 C2 D1 -- D2
     *    A     A   C      A0 C0 A1 C1 A2 C2
     *    9     A     D    A0 -- A1 D0 A2 D1 -- D2
     *    8     A          A0 -- A1 -- A2
     *    7       B C D    B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- D2
     *    6       B C      B0 C0 -- B1 C1 -- B2 C2
     *    5       B   D    B0 -- -- B1 D0 -- B2 D1 -- D2
     *    4       B        B0 -- -- B1 -- -- B2
     *    3         C D    C0 -- -- C1 D0 -- C2 D1 -- D2
     *    2         C      C0 -- C1 -- C2
     *    1           D    D0 -- D1 -- D2
     *    0                -- -- -- --
     *
     * The programs below apply of the fill bit is set. They have been derived
     * from the "Errata for the Amiga Hardware Manual" (October 17, 1985).
     * The published table doesn't seem to be 100% accurate. See the
     * microprograms below for applied modifications.
     *
     *           Active
     * BLTCON0  Channels            Cycle sequence
     *    D     A B   D    A0 B0 -- -- A1 B1 D0 -- A2 B2 D1 -- D2
     *    9     A     D    A0 -- -- A1 D0 A2 D1 -- D2
     *    5       B   D    B0 -- -- -- B1 D0 -- -- B2 D1 -- D2
     *    1           D    -- -- -- D0 -- -- D1 -- -- D2
     *
     * For all other BLTCON0 combinations, the fill bit has no effect on timing.
     */
    void (Blitter::*copyBlitInstr[16][2][2][6])(void) = {

        // 0: -- -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <BUSIDLE>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <NOTHING>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <BUSIDLE>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <NOTHING>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <BUSIDLE>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <NOTHING>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 1:  -- D0 -- D1 | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <HOLD_D | BUSIDLE>,
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <WRITE_D>,
                    &Blitter::exec <BUSIDLE | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <WRITE_D | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <WRITE_D>,
                    &Blitter::fakeExec <BUSIDLE | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 2: C0 -- C1 -- | -- C2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                },
                {   // Full execution, fill
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 3: C0 -- -- C1 D0 -- C2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 4: B0 -- -- B1 -- -- | -- B2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FILL | HOLD_D | BUSIDLE>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 5: B0 -- -- B1 D0 -- B2 D1 -- | -- D2
        // 5: B0 -- -- -- B1 D0 -- -- B2 D1 -- -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <BUSIDLE | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <WRITE_D | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <WRITE_D | HOLD_B>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <BUSIDLE | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | HOLD_B>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                }
            }
        },

        // 6: B0 C0 -- B1 C1 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <BUSIDLE | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <BUSIDLE | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 7: B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <BUSIDLE | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                },
                {
                    // Full execution, fill
                    &Blitter::exec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <BUSIDLE | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <BUSIDLE | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                }
            }
        },

        // 8: A0 -- A1 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <HOLD_A | HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <HOLD_A | HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <HOLD_A | HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <HOLD_A | HOLD_B | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // 9: A0 -- A1 D0 A2 D1 | -- D2
        // 9: A0 -- -- A1 D0 -- A2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | HOLD_A | HOLD_B>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | HOLD_A | HOLD_B>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // A: A0 C0 A1 C1 A2 C2 | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // B: A0 C0 -- A1 C1 D0 A2 C2 D1 | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_C | HOLD_A | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_C | HOLD_A | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // C: A0 B0 -- A1 B1 -- A2 B2 -- | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <HOLD_B  | BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // D: A0 B0 -- A1 B1 D0 A2 B2 D1 | -- D2
        // D: A0 B0 -- -- A1 B1 D0 -- A2 B2 D1 -- | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <WRITE_D | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <WRITE_D | HOLD_B>,
                    &Blitter::exec <BUSIDLE | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <WRITE_D | HOLD_B>,
                    &Blitter::fakeExec <BUSIDLE | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                }
            }
        },

        // E: A0 B0 C0 A1 B1 C1 A2 B2 C2 | -- --
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::exec <FILL | HOLD_D>,
                    &Blitter::exec <BLTDONE>,
                    &Blitter::exec <BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B | REPEAT>,

                    &Blitter::fakeExec <FILL | HOLD_D>,
                    &Blitter::fakeExec <BLTDONE>,
                    &Blitter::fakeExec <BLTDONE>
                }
            }
        },

        // F: A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 | -- D2
        {
            {
                {   // Full execution, no fill
                    &Blitter::exec <FETCH_A | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                },
                {   // Full execution, fill
                    &Blitter::exec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::exec <FETCH_B | HOLD_A>,
                    &Blitter::exec <FETCH_C | HOLD_B>,
                    &Blitter::exec <WRITE_D | REPEAT>,

                    &Blitter::exec <HOLD_D>,
                    &Blitter::exec <WRITE_D | BLTDONE>
                }
            },
            {
                {   // Fake execution, no fill
                    &Blitter::fakeExec <FETCH_A | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                },
                {   // Fake execution, fill
                    &Blitter::fakeExec <FETCH_A | FILL | HOLD_D>,
                    &Blitter::fakeExec <FETCH_B | HOLD_A>,
                    &Blitter::fakeExec <FETCH_C | HOLD_B>,
                    &Blitter::fakeExec <WRITE_D | REPEAT>,

                    &Blitter::fakeExec <HOLD_D>,
                    &Blitter::fakeExec <WRITE_D | BLTDONE>
                }
            }
        }
    };

    /* The Line Blitter uses the same micro program in all situations.
     *
     * -- C0 -- -- -- C1 -- D0 -- C2 -- D1 | -- D2   (???)
    */
    void (Blitter::*lineBlitInstr[6])(void) = {

        // Fake execution
        &Blitter::fakeExec <BUSIDLE>,
        &Blitter::fakeExec <FETCH_C>,
        &Blitter::fakeExec <BUSIDLE>,
        &Blitter::fakeExec <WRITE_D | REPEAT>,

        &Blitter::fakeExec <NOTHING>,
        &Blitter::fakeExec <WRITE_D | BLTDONE>
    };

    // Copy all programs over
    assert(sizeof(this->copyBlitInstr) == sizeof(copyBlitInstr));
    memcpy(this->copyBlitInstr, copyBlitInstr, sizeof(copyBlitInstr));

    assert(sizeof(this->lineBlitInstr) == sizeof(lineBlitInstr));
    memcpy(this->lineBlitInstr, lineBlitInstr, sizeof(lineBlitInstr));
}

void
Blitter::beginFakeLineBlit()
{
    // Only call this function in line blit mode
    assert(bltconLINE());

    // Do the blit
    doFastLineBlit();

    // Prepare the slow Blitter
    bltsizeH = 1;
    resetXCounter();
    resetYCounter();

    // Schedule the first slow Blitter execution event
    agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_LINE_FAKE);
}

void
Blitter::beginSlowLineBlit()
{
    // Only call this function is line blit mode
    assert(bltconLINE());

    /* Note: There is no such thing as a slow line Blitter yet. Until such a
     * thing has been implemented, we call the fast Blitter instead.
     */

    beginFakeLineBlit();
}

void
Blitter::beginFakeCopyBlit()
{
    // Only call this function in copy blit mode
    assert(!bltconLINE());

    // Run the fast Blitter
    int nr = ((bltcon0 >> 7) & 0b11110) | !!bltconDESC();
    (this->*blitfunc[nr])();

    // Prepare the slow Blitter
    resetXCounter();
    resetYCounter();
    lockD = true;

    // Schedule the first slow Blitter execution event
    agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_COPY_FAKE);
}

void
Blitter::beginSlowCopyBlit()
{
    // Only call this function in copy blit mode
    assert(!bltconLINE());

    static bool verbose = true;
    if (BLT_CHECKSUM && verbose) {
        verbose = false;
        msg("Using the slow copy Blitter\n");
    }

    // Set width and height counters
    resetXCounter();
    resetYCounter();

    // Reset registers
    aold = 0;
    bold = 0;

    // Reset the fill carry bit
    fillCarry = bltconFCI();

    // Lock pipeline stage D
    lockD = true;

    // Schedule the first slow Blitter execution event
    agnus.scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_COPY_SLOW);

    // In debug mode, we execute the whole micro program immediately.
    // This let's us compare checksums with the fast Blitter.
    if (SLOW_BLT_DEBUG) {
        
        BusOwner owner = agnus.busOwner[agnus.pos.h];
        agnus.setBLS(false);
        
        while (agnus.hasEvent<BLT_SLOT>()) {
            agnus.busOwner[agnus.pos.h] = BUS_NONE;
            serviceEvent(agnus.slot[BLT_SLOT].id);
        }
        
        agnus.busOwner[agnus.pos.h] = owner;
    }
}

template <u16 instr> void
Blitter::exec()
{
    bool bus, busidle;
    bool desc = bltconDESC();
    
    // Determine if we need the bus
    if (instr & WRITE_D) {
        bus     = !lockD;
        busidle = lockD;
    } else {
        bus     = instr & (FETCH | BUS);
        busidle = instr & BUSIDLE;
    }

    // Trigger Blitter interrupt if this is the termination cycle
    if ((instr & BLTDONE) && !birq) {
        signalEnd();
        paula.raiseIrq(INT_BLIT);
        birq = true;
    }
    
    // Allocate the bus if needed
    if (bus && !agnus.allocateBus<BUS_BLITTER>()) return;

    // Check if the Blitter needs a free bus to continue
    if (busidle && !agnus.busIsFree<BUS_BLITTER>()) return;

    bltpc++;

    if (instr & WRITE_D) {

        // Only proceed if channel D is unlocked
        if (!lockD) {

            agnus.doBlitterDMA(bltdpt, dhold);

            if (BLT_GUARD) {
                memguard[bltdpt & agnus.ptrMask & mem.chipMask] = 1;
            }

            if (BLT_CHECKSUM) {
                check1 = fnv_1a_it32(check1, dhold);
                check2 = fnv_1a_it32(check2, bltdpt);
            }
            trace(BLT_DEBUG, "D: poke(%X), %X (check: %X %X)\n", bltdpt, dhold, check1, check2);
                
            bltdpt += desc ? -2 : 2;
            if (--cntD == 0) {
                bltdpt += desc ? -bltdmod : bltdmod;
                cntD = bltsizeH;
                fillCarry = !!bltconFCI();
            }
        }
    }

    if (instr & FETCH_A) {

        trace(BLT_DEBUG, "FETCH_A\n");

        anew = agnus.doBlitterDMA(bltapt);
        trace(BLT_DEBUG, "    A = peek(%X) = %X\n", bltapt, anew);
        trace(BLT_DEBUG, "    After fetch: A = %X\n", anew);
        bltapt += desc ? -2 : 2;
        if (--cntA == 0) {
            bltapt += desc ? -bltamod : bltamod;
            cntA = bltsizeH;
        }
    }

    if (instr & FETCH_B) {

        trace(BLT_DEBUG, "FETCH_B\n");

        bnew = agnus.doBlitterDMA(bltbpt);
        trace(BLT_DEBUG, "    B = peek(%X) = %X\n", bltbpt, bnew);
        trace(BLT_DEBUG, "    After fetch: B = %X\n", bnew);
        bltbpt += desc ? -2 : 2;
        if (--cntB == 0) {
            bltbpt += desc ? -bltbmod : bltbmod;
            cntB = bltsizeH;
        }
    }

    if (instr & FETCH_C) {

        trace(BLT_DEBUG, "FETCH_C\n");

        chold = agnus.doBlitterDMA(bltcpt);
        trace(BLT_DEBUG, "    C = peek(%X) = %X\n", bltcpt, chold);
        trace(BLT_DEBUG, "    After fetch: C = %X\n", chold);
        bltcpt += desc ? -2 : 2;
        if (--cntC == 0) {
            bltcpt += desc ? -bltcmod : bltcmod;
            cntC = bltsizeH;
        }
    }

    if (instr & HOLD_A) {

        trace(BLT_DEBUG, "HOLD_A\n");

        trace(BLT_DEBUG, "    After masking with %x (%x,%x) %x\n", mask, bltafwm, bltalwm, anew & mask);

        // Run the barrel shifters on data path A
        trace(BLT_DEBUG, "    ash = %d mask = %X\n", bltconASH(), mask);
        if (desc) {
            ahold = HI_W_LO_W(anew & mask, aold) >> (16 - bltconASH());
        } else {
            ahold = HI_W_LO_W(aold, anew & mask) >> bltconASH();
        }
        aold = anew & mask;
        trace(BLT_DEBUG, "    After shifting A (%d) A = %x\n", bltconASH(), ahold);
    }

    if (instr & HOLD_B) {

        trace(BLT_DEBUG, "HOLD_B\n");

        // Run the barrel shifters on data path B
        trace(BLT_DEBUG, "    bsh = %d\n", bltconBSH());
        if (desc) {
            bhold = HI_W_LO_W(bnew, bold) >> (16 - bltconBSH());
        } else {
            bhold = HI_W_LO_W(bold, bnew) >> bltconBSH();
        }
        bold = bnew;
        trace(BLT_DEBUG, "    After shifting B (%d) B = %x\n", bltconBSH(), bhold);
    }

    if (instr & HOLD_D) {

        trace(BLT_DEBUG, "HOLD_D\n");

        // Run the minterm logic circuit
        trace(BLT_DEBUG, "    Minterms: ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
        dhold = doMintermLogicQuick(ahold, bhold, chold, bltcon0 & 0xFF);
        assert(releaseBuild() || dhold == doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF));

        if (!lockD) {

            // Run the fill logic circuitry
            if (instr & FILL) doFill(dhold, fillCarry);

            // Update the zero flag
            if (dhold) bzero = false;
        }
        
    }

    if (instr & REPEAT) {

        u16 newpc = 0;

        trace(BLT_DEBUG, "REPEAT\n");
        iteration++;
        lockD = false;

        if (xCounter > 1) {

            bltpc = newpc;
            decXCounter();

        } else if (yCounter > 1) {

            bltpc = newpc;
            resetXCounter();
            decYCounter();

        } else {

            // signalEnd();
        }
    }

    if (instr & BLTDONE) {

        trace(BLT_DEBUG, "BLTDONE\n");
        endBlit();
    }
}

template <u16 instr> void
Blitter::fakeExec()
{
    bool bus, busidle;

    // Determine if we need the bus
    if (instr & WRITE_D) {
        bus     = !lockD;
        busidle = lockD;
    } else {
        bus     = instr & (FETCH | BUS);
        busidle = instr & BUSIDLE;
    }

    // Trigger Blitter interrupt if this is the termination cycle
    if ((instr & BLTDONE) && !birq) {
        signalEnd();
        paula.raiseIrq(INT_BLIT);
        birq = true;
    }

    // Allocate the bus if needed
    if (bus && !agnus.allocateBus<BUS_BLITTER>()) return;

    // Check if the Blitter needs a free bus to continue
    if (busidle && !agnus.busIsFree<BUS_BLITTER>()) return;

    bltpc++;

    if (instr & (FETCH | WRITE_D)) {

        // Record some fake data to make the DMA debugger happy
        assert(agnus.pos.h < HPOS_CNT);
        agnus.busValue[agnus.pos.h] = 0x8888;
    }

    if (instr & REPEAT) {

        u16 newpc = 0;

        trace(BLT_DEBUG, "REPEAT\n");
        iteration++;
        lockD = false;

        if (xCounter > 1) {

            bltpc = newpc;
            decXCounter();

        } else if (yCounter > 1) {

            bltpc = newpc;
            resetXCounter();
            decYCounter();

        } else {

            // signalEnd();
        }
    }

    if (instr & BLTDONE) {

        trace(BLT_DEBUG, "BLTDONE\n");
        endBlit();
    }
}

void
Blitter::setXCounter(u16 value)
{
    xCounter = value;

    // Set the mask for this iteration
    mask = 0xFFFF;

    // Apply the "first word mask" in the first iteration
    if (isFirstWord()) mask &= bltafwm;

    // Apply the "last word mask" in the last iteration
    if (isLastWord()) mask &= bltalwm;
}

void
Blitter::setYCounter(u16 value)
{
    yCounter = value;
}

void
Blitter::doBarrelShifterA()
{
    u16 masked = anew;

    if (isFirstWord()) masked &= bltafwm;
    if (isLastWord())  masked &= bltalwm;

    if(bltconDESC()){
        ahold = (aold >> (16 - bltconASH())) | (masked << bltconASH());
    }else{
        ahold = (aold << (16 - bltconASH())) | (masked >> bltconASH());
    }
}

void
Blitter::doBarrelShifterB()
{
    if(bltconDESC()) {
        bhold = (bold >> (16 - bltconBSH())) | (bnew << bltconBSH());
    } else {
        bhold = (bold << (16 - bltconBSH())) | (bnew >> bltconBSH());
    }
}
