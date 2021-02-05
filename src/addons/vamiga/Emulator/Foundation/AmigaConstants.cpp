// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Utils.h"

const char *regName(u32 addr)
{
    assert(IS_EVEN(addr));

    static const char *name[256] = {

        "BLTDDAT",        "DMACONR",        "VPOSR",
        "VHPOSR",         "DSKDATR",        "JOY0DAT",
        "JOY1DAT",        "CLXDAT",         "ADKCONR",
        "POT0DAT",        "POT1DAT",        "POTGOR",
        "SERDATR",        "DSKBYTR",        "INTENAR",
        "INTREQR",        "DSKPTH",         "DSKPTL",
        "DSKLEN",         "DSKDAT",         "REFPTR",
        "VPOSW",          "VHPOSW",         "COPCON",
        "SERDAT",         "SERPER",         "POTGO",
        "JOYTEST",        "STREQU",         "STRVBL",
        "STRHOR",         "STRLONG",        "BLTCON0",
        "BLTCON1",        "BLTAFWM",        "BLTALWM",
        "BLTCPTH",        "BLTCPTL",        "BLTBPTH",
        "BLTBPTL",        "BLTAPTH",        "BLTAPTL",
        "BLTDPTH",        "BLTDPTL",        "BLTSIZE",
        "BLTCON0L (ECS)", "BLTSIZV (ECS)",  "BLTSIZH (ECS)",
        "BLTCMOD",        "BLTBMOD",        "BLTAMOD",
        "BLTDMOD",        "unused",         "unused",
        "unused",         "unused",         "BLTCDAT",
        "BLTBDAT",        "BLTADAT",        "unused",
        "SPRHDAT (ECS)",  "BPLHDAT (AGA)",  "DENISEID (ECS)",
        "DSKSYNC",        "COP1LCH",        "COP1LCL",
        "COP2LCH",        "COP2LCL",        "COPJMP1",
        "COPJMP2",        "COPINS",         "DIWSTRT",
        "DIWSTOP",        "DDFSTRT",        "DDFSTOP",
        "DMACON",         "CLXCON",         "INTENA",
        "INTREQ",         "ADKCON",         "AUD0PTH",
        "AUD0PTL",        "AUD0LEN",        "AUD0PER",
        "AUD0VOL",        "AUD0DAT",        "unused",
        "UNUSED",         "AUD1PTH",        "AUD1PTL",
        "AUD1LEN",        "AUD1PER",        "AUD1VOL",
        "AUD1DAT",        "unused",         "unused",
        "AUD2PTH",        "AUD2PTL",        "AUD2LEN",
        "AUD2PER",        "AUD2VOL",        "AUD2DAT",
        "unused",         "unused",         "AUD3PTH",
        "AUD3PTL",        "AUD3LEN",        "AUD3PER",
        "AUD3VOL",        "AUD3DAT",        "unused",
        "UNUSED",         "BPL1PTH",        "BPL1PTL",
        "BPL2PTH",        "BPL2PTL",        "BPL3PTH",
        "BPL3PTL",        "BPL4PTH",        "BPL4PTL",
        "BPL5PTH",        "BPL5PTL",        "BPL6PTH",
        "BPL6PTL",        "BPL7PTH (AGA)",  "BPL7PTL (AGA)",
        "BPL8PTH (AGA)",  "BPL8PTL (AGA)",  "BPLCON0",
        "BPLCON1",        "BPLCON2",        "BPLCON3 (ECS)",
        "BPL1MOD",        "BPL2MOD",        "BPLCON4 (AGA)",
        "BPLCON4 (AGA)",  "BPL1DAT",        "BPL2DAT",
        "BPL3DAT",        "BPL4DAT",        "BPL5DAT",
        "BPL6DAT",        "BPL7DAT (AGA)",  "BPL8DAT (AGA)",
        "SPR0PTH",        "SPR0PTL",        "SPR1PTH",
        "SPR1PTL",        "SPR2PTH",        "SPR2PTL",
        "SPR3PTH",        "SPR3PTL",        "SPR4PTH",
        "SPR4PTL",        "SPR5PTH",        "SPR5PTL",
        "SPR6PTH",        "SPR6PTL",        "SPR7PTH",
        "SPR7PTL",        "SPR0POS",        "SPR0CTL",
        "SPR0DATA",       "SPR0DATB",       "SPR1POS",
        "SPR1CTL",        "SPR1DATA",       "SPR1DATB",
        "SPR2POS",        "SPR2CTL",        "SPR2DATA",
        "SPR2DATB",       "SPR3POS",        "SPR3CTL",
        "SPR3DATA",       "SPR3DATB",       "SPR4POS",
        "SPR4CTL",        "SPR4DATA",       "SPR4DATB",
        "SPR5POS",        "SPR5CTL",        "SPR5DATA",
        "SPR5DATB",       "SPR6POS",        "SPR6CTL",
        "SPR6DATA",       "SPR6DATB",       "SPR7POS",
        "SPR7CTL",        "SPR7DATA",       "SPR7DATB",
        "COLOR00",        "COLOR01",        "COLOR02",
        "COLOR03",        "COLOR04",        "COLOR05",
        "COLOR06",        "COLOR07",        "COLOR08",
        "COLOR09",        "COLOR10",        "COLOR11",
        "COLOR12",        "COLOR13",        "COLOR14",
        "COLOR15",        "COLOR16",        "COLOR17",
        "COLOR18",        "COLOR19",        "COLOR20",
        "COLOR21",        "COLOR22",        "COLOR23",
        "COLOR24",        "COLOR25",        "COLOR26",
        "COLOR27",        "COLOR28",        "COLOR29",
        "COLOR30",        "COLOR31",        "HTOTAL (ECS)",
        "HSSTOP (ECS)",   "HBSTRT (ECS)",   "HBSTOP (ECS)",
        "VTOTAL (ECS)",   "VSSTOP (ECS)",   "VBSTRT (ECS)",
        "VBSTOP (ECS)",   "SPRHSTRT (AGA)", "SPRHSTOP (AGA)",
        "BPLHSTRT (AGA)", "BPLHSTOP (AGA)", "HHPOSW (AGA)",
        "HHPOSR (AGA)",   "BEAMCON0 (ECS)", "HSSTRT (ECS)",
        "VSSTRT (ECS)",   "HCENTER (ECS)",  "DIWHIGH (ECS)",
        "BPLHMOD (AGA)",  "SPRHPTH (AGA)",  "SPRHPTL (AGA)",
        "BPLHPTH (AGA)",  "BPLHPTL (AGA)",  "unused",
        "unused",         "unused",         "unused",
        "unused",         "unused",         "FMODE (AGA)",
        "NO-OP"
    };

    return name[(addr >> 1) & 0xFF];
}

const char *ciaRegName(u32 addr)
{
    assert(addr < 16);

    static const char *name[16] = {
        
        "PRA",      "PRB",      "DDRA",     "DDRB",
        "TALO",     "TAHI",     "TBLO",     "TBHI",
        "TODLO",    "TODMID",   "TODHI",    "UNUSED",
        "SDR",      "ICR",      "CRA",      "CRB"
    };

    return name[addr];
}
