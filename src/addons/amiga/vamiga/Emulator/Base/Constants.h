// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#pragma once

#include "Aliases.h"

//
// Custom registers
//

#define BLTDDAT  0x000L
#define DMACONR  0x002L
#define VPOSR    0x004L
#define VHPOSR   0x006L
#define DSKDATR  0x008L
#define JOY0DAT  0x00AL
#define JOY1DAT  0x00CL
#define CLXDAT   0x00EL
#define ADKCONR  0x010L
#define POT0DAT  0x012L
#define POT1DAT  0x014L
#define POTGOR   0x016L
#define SERDATR  0x018L
#define DSKBYTR  0x01AL
#define INTENAR  0x01CL
#define INTREQR  0x01EL
#define DSKPTH   0x020L
#define DSKPTL   0x022L
#define DSKLEN   0x024L
#define DSKDAT   0x026L
#define REFPTR   0x028L
#define VPOSW    0x02AL
#define VHPOSW   0x02CL
#define COPCON   0x02EL
#define SERDAT   0x030L
#define SERPER   0x032L
#define POTGO    0x034L
#define JOYTEST  0x036L
#define STREQU   0x038L
#define STRVBL   0x03AL
#define STRHOR   0x03CL
#define STRLONG  0x03EL
#define BLTCON0  0x040L
#define BLTCON1  0x042L
#define BLTAFWM  0x044L
#define BLTALWM  0x046L
#define BLTCPTH  0x048L
#define BLTCPTL  0x04AL
#define BLTBPTH  0x04CL
#define BLTBPTL  0x04EL
#define BLTAPTH  0x050L
#define BLTAPTL  0x052L
#define BLTDPTH  0x054L
#define BLTDPTL  0x056L
#define BLTSIZE  0x058L
#define BLTCON0L 0x05AL // ECS
#define BLTSIZV  0x05CL // ECS
#define BLTSIZH  0x05EL // ECS
#define BLTCMOD  0x060L
#define BLTBMOD  0x062L
#define BLTAMOD  0x064L
#define BLTDMOD  0x066L
#define BLTCDAT  0x070L
#define BLTBDAT  0x072L
#define BLTADAT  0x074L
#define SPRHDAT  0x078L // ECS
#define BPLHDAT  0x07AL // AGA
#define DENISEID 0x07CL // ECS
#define DSKSYNC  0x07EL
#define COP1LCH  0x080L
#define COP1LCL  0x082L
#define COP2LCH  0x084L
#define COP2LCL  0x086L
#define COPJMP1  0x088L
#define COPJMP2  0x08AL
#define COPINS   0x08CL
#define DIWSTRT  0x08EL
#define DIWSTOP  0x090L
#define DDFSTRT  0x092L
#define DDFSTOP  0x094L
#define DMACON   0x096L
#define CLXCON   0x098L
#define INTENA   0x09AL
#define INTREQ   0x09CL
#define ADKCON   0x09EL
#define AUD0LCH  0x0A0L
#define AUD0LCL  0x0A2L
#define AUD0LEN  0x0A4L
#define AUD0PER  0x0A6L
#define AUD0VOL  0x0A8L
#define AUD0DAT  0x0AAL
#define AUD1LCH  0x0B0L
#define AUD1LCL  0x0B2L
#define AUD1LEN  0x0B4L
#define AUD1PER  0x0B6L
#define AUD1VOL  0x0B8L
#define AUD1DAT  0x0BAL
#define AUD2LCH  0x0C0L
#define AUD2LCL  0x0C2L
#define AUD2LEN  0x0C4L
#define AUD2PER  0x0C6L
#define AUD2VOL  0x0C8L
#define AUD2DAT  0x0CAL
#define AUD3LCH  0x0D0L
#define AUD3LCL  0x0D2L
#define AUD3LEN  0x0D4L
#define AUD3PER  0x0D6L
#define AUD3VOL  0x0D8L
#define AUD3DAT  0x0DAL
#define BPL1PTH  0x0E0L
#define BPL1PTL  0x0E2L
#define BPL2PTH  0x0E4L
#define BPL2PTL  0x0E6L
#define BPL3PTH  0x0E8L
#define BPL3PTL  0x0EAL
#define BPL4PTH  0x0ECL
#define BPL4PTL  0x0EEL
#define BPL5PTH  0x0F0L
#define BPL5PTL  0x0F2L
#define BPL6PTH  0x0F4L
#define BPL6PTL  0x0F6L
#define BPL7PTH  0x0F8L // AGA
#define BPL7PTL  0x0FAL // AGA
#define BPL8PTH  0x0FCL // AGA
#define BPL8PTL  0x0FEL // AGA
#define BPLCON0  0x100L
#define BPLCON1  0x102L
#define BPLCON2  0x104L
#define BPLCON3  0x106L // ECS
#define BPL1MOD  0x108L
#define BPL2MOD  0x10AL
#define BPLCON4  0x10CL // AGA
#define CLXCON2  0x10EL // AGA
#define BPL1DAT  0x110L
#define BPL2DAT  0x112L
#define BPL3DAT  0x114L
#define BPL4DAT  0x116L
#define BPL5DAT  0x118L
#define BPL6DAT  0x11AL
#define BPL7DAT  0x11CL // AGA
#define BPL8DAT  0x11EL // AGA
#define SPR0PTH  0x120L
#define SPR0PTL  0x122L
#define SPR1PTH  0x124L
#define SPR1PTL  0x126L
#define SPR2PTH  0x128L
#define SPR2PTL  0x12AL
#define SPR3PTH  0x12CL
#define SPR3PTL  0x12EL
#define SPR4PTH  0x130L
#define SPR4PTL  0x132L
#define SPR5PTH  0x134L
#define SPR5PTL  0x136L
#define SPR6PTH  0x138L
#define SPR6PTL  0x13AL
#define SPR7PTH  0x13CL
#define SPR7PTL  0x13EL
#define SPR0POS  0x140L
#define SPR0CTL  0x142L
#define SPR0DATA 0x144L
#define SPR0DATB 0x146L
#define SPR1POS  0x148L
#define SPR1CTL  0x14AL
#define SPR1DATA 0x14CL
#define SPR1DATB 0x14EL
#define SPR2POS  0x150L
#define SPR2CTL  0x152L
#define SPR2DATA 0x154L
#define SPR2DATB 0x156L
#define SPR3POS  0x158L
#define SPR3CTL  0x15AL
#define SPR3DATA 0x15CL
#define SPR3DATB 0x15EL
#define SPR4POS  0x160L
#define SPR4CTL  0x162L
#define SPR4DATA 0x164L
#define SPR4DATB 0x166L
#define SPR5POS  0x168L
#define SPR5CTL  0x16AL
#define SPR5DATA 0x16CL
#define SPR5DATB 0x16EL
#define SPR6POS  0x170L
#define SPR6CTL  0x172L
#define SPR6DATA 0x174L
#define SPR6DATB 0x176L
#define SPR7POS  0x178L
#define SPR7CTL  0x17AL
#define SPR7DATA 0x17CL
#define SPR7DATB 0x17EL
#define COLOR00  0x180L
#define COLOR01  0x182L
#define COLOR02  0x184L
#define COLOR03  0x186L
#define COLOR04  0x188L
#define COLOR05  0x18AL
#define COLOR06  0x18CL
#define COLOR07  0x18EL
#define COLOR08  0x190L
#define COLOR09  0x192L
#define COLOR10  0x194L
#define COLOR11  0x196L
#define COLOR12  0x198L
#define COLOR13  0x19AL
#define COLOR14  0x19CL
#define COLOR15  0x19EL
#define COLOR16  0x1A0L
#define COLOR17  0x1A2L
#define COLOR18  0x1A4L
#define COLOR19  0x1A6L
#define COLOR20  0x1A8L
#define COLOR21  0x1AAL
#define COLOR22  0x1ACL
#define COLOR23  0x1AEL
#define COLOR24  0x1B0L
#define COLOR25  0x1B2L
#define COLOR26  0x1B4L
#define COLOR27  0x1B6L
#define COLOR28  0x1B8L
#define COLOR29  0x1BAL
#define COLOR30  0x1BCL
#define COLOR31  0x1BEL
#define HTOTAL   0x1C0L // ECS
#define HSSTOP   0x1C2L // ECS
#define HBSTRT   0x1C4L // ECS
#define HBSTOP   0x1C6L // ECS
#define VTOTAL   0x1C8L // ECS
#define VSSTOP   0x1CAL // ECS
#define VBSTRT   0x1CCL // ECS
#define VBSTOP   0x1CEL // ECS
#define SPRHSTRT 0x1D0L // AGA
#define SPRHSTOP 0x1D2L // AGA
#define BPLHSTRT 0x1D4L // AGA
#define BPLHSTOP 0x1D6L // AGA
#define HHPOSW   0x1D8L // AGA
#define HHPOSR   0x1DAL // AGA
#define BEAMCON0 0x1DCL // ECS
#define HSSTRT   0x1DEL // ECS
#define VSSTRT   0x1E0L // ECS
#define HCENTER  0x1E2L // ECS
#define DIWHIGH  0x1E4L // ECS
#define BPLHMOD  0x1E6L // AGA
#define SPRHPTH  0x1E8L // AGA
#define SPRHPTL  0x1EAL // AGA
#define BPLHPTH  0x1ECL // AGA
#define BPLHPTL  0x1EEL // AGA
#define FMODE    0x1FCL // AGA
#define NO_OP    0x1FEL

// DMACON register bits
#define BBUSY  0x4000
#define BZERO  0x2000
#define BLTPRI 0x0400
#define DMAEN  0x0200
#define BPLEN  0x0100
#define COPEN  0x0080
#define BLTEN  0x0040
#define SPREN  0x0020
#define DSKEN  0x0010
#define AUD3EN 0x0008
#define AUD2EN 0x0004
#define AUD1EN 0x0002
#define AUD0EN 0x0001

#define AUDEN  0x000F


//
// Screen parameters
//

/* Beam positions
 *
 * Vertical coordinates are measured in scanlines.
 * Horizontal coordinates are measured in DMA cycles.
 */

#define VPOS_MAX      312
#define VPOS_CNT      313

#define HPOS_MAX      226
#define HPOS_CNT      227


/* Screen buffer dimensions
 *
 * All values are measured in pixels. One DMA cycle corresponds to 4 pixels.
 * Hence, HPIXELS equals 4 * HPOS_CNT. VPIXELS is one greater than VPOS_CNT,
 * because of the misalignment offset applied to the screen buffer start
 * address (see below).
 */

#define VPIXELS       314                    // VPOS_CNT + 1 line
#define HPIXELS       908                    // 4 * HPOS_CNT
#define PIXELS        (VPIXELS * HPIXELS)
#define LAST_PIXEL    907

/* Blanking area
 *
 * To understand the horizontal position of the Amiga screen, it is important
 * to note that the HBLANK area does *not* start at DMA cycle 0. According to
 * "Amiga Intern", DMA cycle $0F (15) is the first and $35 (53) the last cycles
 * inside the HBLANK area. However, these values seem to be wrong and I am
 * using different values instead.
 *
 * As a result, the early DMA cycles do not appear on the left side of the
 * screen, but on the right side in the previous scanline. To mimic this
 * behaviour, a misalignment offset is added to the start address of the screen
 * buffer before it is written into the GPU texture. The offset is chosen such
 * that the HBLANK area starts at the first pixel of each line in the texture.
 * As a side effect of adding this offset, constant VPIXELS needs to be greater
 * than VPOS_CNT. Otherwise, we would access unallocated memory at the end of
 * the last scanline.
 */

#define HBLANK_MIN    0x0A
#define HBLANK_MAX    0x30
#define HBLANK_CNT    0x27 // equals HBLANK_MAX - HBLANK_MIN + 1

#define VBLANK_MIN    0x00
#define VBLANK_MAX    0x19
#define VBLANK_CNT    0x1A // equals VBLANK_MAX - VBLANK_MIN + 1

// Returns a printable name for a custom register
const char *regName(u32 addr);

// Returns a printable name for a CIA register
const char *ciaRegName(u32 addr);
